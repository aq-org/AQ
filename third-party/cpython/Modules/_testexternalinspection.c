#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef Py_BUILD_CORE_BUILTIN
#    define Py_BUILD_CORE_MODULE 1
#endif
#include "Python.h"
#include <internal/pycore_debug_offsets.h>  // _Py_DebugOffsets
#include <internal/pycore_frame.h>          // FRAME_SUSPENDED_YIELD_FROM
#include <internal/pycore_interpframe.h>    // FRAME_OWNED_BY_CSTACK
#include <internal/pycore_llist.h>          // struct llist_node
#include <internal/pycore_stackref.h>       // Py_TAG_BITS
#include "../Python/remote_debug.h"

#ifndef HAVE_PROCESS_VM_READV
#    define HAVE_PROCESS_VM_READV 0
#endif

struct _Py_AsyncioModuleDebugOffsets {
    struct _asyncio_task_object {
        uint64_t size;
        uint64_t task_name;
        uint64_t task_awaited_by;
        uint64_t task_is_task;
        uint64_t task_awaited_by_is_set;
        uint64_t task_coro;
        uint64_t task_node;
    } asyncio_task_object;
    struct _asyncio_interpreter_state {
        uint64_t size;
        uint64_t asyncio_tasks_head;
    } asyncio_interpreter_state;
    struct _asyncio_thread_state {
        uint64_t size;
        uint64_t asyncio_running_loop;
        uint64_t asyncio_running_task;
        uint64_t asyncio_tasks_head;
    } asyncio_thread_state;
};

// Get the PyAsyncioDebug section address for any platform
static uintptr_t
_Py_RemoteDebug_GetAsyncioDebugAddress(proc_handle_t* handle)
{
    uintptr_t address;

#ifdef MS_WINDOWS
    // On Windows, search for asyncio debug in executable or DLL
    address = search_windows_map_for_section(handle, "AsyncioD", L"_asyncio");
#elif defined(__linux__)
    // On Linux, search for asyncio debug in executable or DLL
    address = search_linux_map_for_section(handle, "AsyncioDebug", "_asyncio.cpython");
#elif defined(__APPLE__) && TARGET_OS_OSX
    // On macOS, try libpython first, then fall back to python
    address = search_map_for_section(handle, "AsyncioDebug", "_asyncio.cpython");
    if (address == 0) {
        PyErr_Clear();
        address = search_map_for_section(handle, "AsyncioDebug", "_asyncio.cpython");
    }
#else
    address = 0;
#endif

    return address;
}

static int
read_string(
    proc_handle_t *handle,
    _Py_DebugOffsets* debug_offsets,
    uintptr_t address,
    char* buffer,
    Py_ssize_t size
) {
    Py_ssize_t len;
    int result = _Py_RemoteDebug_ReadRemoteMemory(
        handle,
        address + debug_offsets->unicode_object.length,
        sizeof(Py_ssize_t),
        &len
    );
    if (result < 0) {
        return -1;
    }
    if (len >= size) {
        PyErr_SetString(PyExc_RuntimeError, "Buffer too small");
        return -1;
    }
    size_t offset = debug_offsets->unicode_object.asciiobject_size;
    result = _Py_RemoteDebug_ReadRemoteMemory(handle, address + offset, len, buffer);
    if (result < 0) {
        return -1;
    }
    buffer[len] = '\0';
    return 0;
}

static inline int
read_ptr(proc_handle_t *handle, uintptr_t address, uintptr_t *ptr_addr)
{
    int result = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(void*), ptr_addr);
    if (result < 0) {
        return -1;
    }
    return 0;
}

static inline int
read_Py_ssize_t(proc_handle_t *handle, uintptr_t address, Py_ssize_t *size)
{
    int result = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(Py_ssize_t), size);
    if (result < 0) {
        return -1;
    }
    return 0;
}

static int
read_py_ptr(proc_handle_t *handle, uintptr_t address, uintptr_t *ptr_addr)
{
    if (read_ptr(handle, address, ptr_addr)) {
        return -1;
    }
    *ptr_addr &= ~Py_TAG_BITS;
    return 0;
}

static int
read_char(proc_handle_t *handle, uintptr_t address, char *result)
{
    int res = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(char), result);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
read_int(proc_handle_t *handle, uintptr_t address, int *result)
{
    int res = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(int), result);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
read_unsigned_long(proc_handle_t *handle, uintptr_t address, unsigned long *result)
{
    int res = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(unsigned long), result);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static int
read_pyobj(proc_handle_t *handle, uintptr_t address, PyObject *ptr_addr)
{
    int res = _Py_RemoteDebug_ReadRemoteMemory(handle, address, sizeof(PyObject), ptr_addr);
    if (res < 0) {
        return -1;
    }
    return 0;
}

static PyObject *
read_py_str(
    proc_handle_t *handle,
    _Py_DebugOffsets* debug_offsets,
    uintptr_t address,
    Py_ssize_t max_len
) {
    assert(max_len > 0);

    PyObject *result = NULL;

    char *buf = (char *)PyMem_RawMalloc(max_len);
    if (buf == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    if (read_string(handle, debug_offsets, address, buf, max_len)) {
        goto err;
    }

    result = PyUnicode_FromString(buf);
    if (result == NULL) {
        goto err;
    }

    PyMem_RawFree(buf);
    assert(result != NULL);
    return result;

err:
    PyMem_RawFree(buf);
    return NULL;
}

static long
read_py_long(proc_handle_t *handle, _Py_DebugOffsets* offsets, uintptr_t address)
{
    unsigned int shift = PYLONG_BITS_IN_DIGIT;

    Py_ssize_t size;
    uintptr_t lv_tag;

    int bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle, address + offsets->long_object.lv_tag,
        sizeof(uintptr_t),
        &lv_tag);
    if (bytes_read < 0) {
        return -1;
    }

    int negative = (lv_tag & 3) == 2;
    size = lv_tag >> 3;

    if (size == 0) {
        return 0;
    }

    digit *digits = (digit *)PyMem_RawMalloc(size * sizeof(digit));
    if (!digits) {
        PyErr_NoMemory();
        return -1;
    }

    bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle,
        address + offsets->long_object.ob_digit,
        sizeof(digit) * size,
        digits
    );
    if (bytes_read < 0) {
        goto error;
    }

    long long value = 0;

    // In theory this can overflow, but because of llvm/llvm-project#16778
    // we can't use __builtin_mul_overflow because it fails to link with
    // __muloti4 on aarch64. In practice this is fine because all we're
    // testing here are task numbers that would fit in a single byte.
    for (Py_ssize_t i = 0; i < size; ++i) {
        long long factor = digits[i] * (1UL << (Py_ssize_t)(shift * i));
        value += factor;
    }
    PyMem_RawFree(digits);
    if (negative) {
        value *= -1;
    }
    return (long)value;
error:
    PyMem_RawFree(digits);
    return -1;
}

static PyObject *
parse_task_name(
    proc_handle_t *handle,
    _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t task_address
) {
    uintptr_t task_name_addr;
    int err = read_py_ptr(
        handle,
        task_address + async_offsets->asyncio_task_object.task_name,
        &task_name_addr);
    if (err) {
        return NULL;
    }

    // The task name can be a long or a string so we need to check the type

    PyObject task_name_obj;
    err = read_pyobj(
        handle,
        task_name_addr,
        &task_name_obj);
    if (err) {
        return NULL;
    }

    unsigned long flags;
    err = read_unsigned_long(
        handle,
        (uintptr_t)task_name_obj.ob_type + offsets->type_object.tp_flags,
        &flags);
    if (err) {
        return NULL;
    }

    if ((flags & Py_TPFLAGS_LONG_SUBCLASS)) {
        long res = read_py_long(handle, offsets, task_name_addr);
        if (res == -1) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to get task name");
            return NULL;
        }
        return PyUnicode_FromFormat("Task-%d", res);
    }

    if(!(flags & Py_TPFLAGS_UNICODE_SUBCLASS)) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid task name object");
        return NULL;
    }

    return read_py_str(
        handle,
        offsets,
        task_name_addr,
        255
    );
}

static int
parse_coro_chain(
    proc_handle_t *handle,
    struct _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t coro_address,
    PyObject *render_to
) {
    assert((void*)coro_address != NULL);

    uintptr_t gen_type_addr;
    int err = read_ptr(
        handle,
        coro_address + sizeof(void*),
        &gen_type_addr);
    if (err) {
        return -1;
    }

    uintptr_t gen_name_addr;
    err = read_py_ptr(
        handle,
        coro_address + offsets->gen_object.gi_name,
        &gen_name_addr);
    if (err) {
        return -1;
    }

    PyObject *name = read_py_str(
        handle,
        offsets,
        gen_name_addr,
        255
    );
    if (name == NULL) {
        return -1;
    }

    if (PyList_Append(render_to, name)) {
        Py_DECREF(name);
        return -1;
    }
    Py_DECREF(name);

    int gi_frame_state;
    err = read_int(
        handle,
        coro_address + offsets->gen_object.gi_frame_state,
        &gi_frame_state);
    if (err) {
        return -1;
    }

    if (gi_frame_state == FRAME_SUSPENDED_YIELD_FROM) {
        char owner;
        err = read_char(
            handle,
            coro_address + offsets->gen_object.gi_iframe +
                offsets->interpreter_frame.owner,
            &owner
        );
        if (err) {
            return -1;
        }
        if (owner != FRAME_OWNED_BY_GENERATOR) {
            PyErr_SetString(
                PyExc_RuntimeError,
                "generator doesn't own its frame \\_o_/");
            return -1;
        }

        uintptr_t stackpointer_addr;
        err = read_py_ptr(
            handle,
            coro_address + offsets->gen_object.gi_iframe +
                offsets->interpreter_frame.stackpointer,
            &stackpointer_addr);
        if (err) {
            return -1;
        }

        if ((void*)stackpointer_addr != NULL) {
            uintptr_t gi_await_addr;
            err = read_py_ptr(
                handle,
                stackpointer_addr - sizeof(void*),
                &gi_await_addr);
            if (err) {
                return -1;
            }

            if ((void*)gi_await_addr != NULL) {
                uintptr_t gi_await_addr_type_addr;
                int err = read_ptr(
                    handle,
                    gi_await_addr + sizeof(void*),
                    &gi_await_addr_type_addr);
                if (err) {
                    return -1;
                }

                if (gen_type_addr == gi_await_addr_type_addr) {
                    /* This needs an explanation. We always start with parsing
                       native coroutine / generator frames. Ultimately they
                       are awaiting on something. That something can be
                       a native coroutine frame or... an iterator.
                       If it's the latter -- we can't continue building
                       our chain. So the condition to bail out of this is
                       to do that when the type of the current coroutine
                       doesn't match the type of whatever it points to
                       in its cr_await.
                    */
                    err = parse_coro_chain(
                        handle,
                        offsets,
                        async_offsets,
                        gi_await_addr,
                        render_to
                    );
                    if (err) {
                        return -1;
                    }
                }
            }
        }

    }

    return 0;
}


static int
parse_task_awaited_by(
    proc_handle_t *handle,
    struct _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t task_address,
    PyObject *awaited_by
);


static int
parse_task(
    proc_handle_t *handle,
    struct _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t task_address,
    PyObject *render_to
) {
    char is_task;
    int err = read_char(
        handle,
        task_address + async_offsets->asyncio_task_object.task_is_task,
        &is_task);
    if (err) {
        return -1;
    }

    uintptr_t refcnt;
    read_ptr(handle, task_address + sizeof(Py_ssize_t), &refcnt);

    PyObject* result = PyList_New(0);
    if (result == NULL) {
        return -1;
    }

    PyObject *call_stack = PyList_New(0);
    if (call_stack == NULL) {
        goto err;
    }
    if (PyList_Append(result, call_stack)) {
        Py_DECREF(call_stack);
        goto err;
    }
    /* we can operate on a borrowed one to simplify cleanup */
    Py_DECREF(call_stack);

    if (is_task) {
        PyObject *tn = parse_task_name(
            handle, offsets, async_offsets, task_address);
        if (tn == NULL) {
            goto err;
        }
        if (PyList_Append(result, tn)) {
            Py_DECREF(tn);
            goto err;
        }
        Py_DECREF(tn);

        uintptr_t coro_addr;
        err = read_py_ptr(
            handle,
            task_address + async_offsets->asyncio_task_object.task_coro,
            &coro_addr);
        if (err) {
            goto err;
        }

        if ((void*)coro_addr != NULL) {
            err = parse_coro_chain(
                handle,
                offsets,
                async_offsets,
                coro_addr,
                call_stack
            );
            if (err) {
                goto err;
            }

            if (PyList_Reverse(call_stack)) {
                goto err;
            }
        }
    }

    if (PyList_Append(render_to, result)) {
        goto err;
    }

    PyObject *awaited_by = PyList_New(0);
    if (awaited_by == NULL) {
        goto err;
    }
    if (PyList_Append(result, awaited_by)) {
        Py_DECREF(awaited_by);
        goto err;
    }
    /* we can operate on a borrowed one to simplify cleanup */
    Py_DECREF(awaited_by);

    if (parse_task_awaited_by(handle, offsets, async_offsets,
                              task_address, awaited_by)
    ) {
        goto err;
    }
    Py_DECREF(result);

    return 0;

err:
    Py_DECREF(result);
    return -1;
}

static int
parse_tasks_in_set(
    proc_handle_t *handle,
    struct _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t set_addr,
    PyObject *awaited_by
) {
    uintptr_t set_obj;
    if (read_py_ptr(
            handle,
            set_addr,
            &set_obj)
    ) {
        return -1;
    }

    Py_ssize_t num_els;
    if (read_Py_ssize_t(
            handle,
            set_obj + offsets->set_object.used,
            &num_els)
    ) {
        return -1;
    }

    Py_ssize_t set_len;
    if (read_Py_ssize_t(
            handle,
            set_obj + offsets->set_object.mask,
            &set_len)
    ) {
        return -1;
    }
    set_len++; // The set contains the `mask+1` element slots.

    uintptr_t table_ptr;
    if (read_ptr(
            handle,
            set_obj + offsets->set_object.table,
            &table_ptr)
    ) {
        return -1;
    }

    Py_ssize_t i = 0;
    Py_ssize_t els = 0;
    while (i < set_len) {
        uintptr_t key_addr;
        if (read_py_ptr(handle, table_ptr, &key_addr)) {
            return -1;
        }

        if ((void*)key_addr != NULL) {
            Py_ssize_t ref_cnt;
            if (read_Py_ssize_t(handle, table_ptr, &ref_cnt)) {
                return -1;
            }

            if (ref_cnt) {
                // if 'ref_cnt=0' it's a set dummy marker

                if (parse_task(
                    handle,
                    offsets,
                    async_offsets,
                    key_addr,
                    awaited_by)
                ) {
                    return -1;
                }

                if (++els == num_els) {
                    break;
                }
            }
        }

        table_ptr += sizeof(void*) * 2;
        i++;
    }
    return 0;
}


static int
parse_task_awaited_by(
    proc_handle_t *handle,
    struct _Py_DebugOffsets* offsets,
    struct _Py_AsyncioModuleDebugOffsets* async_offsets,
    uintptr_t task_address,
    PyObject *awaited_by
) {
    uintptr_t task_ab_addr;
    int err = read_py_ptr(
        handle,
        task_address + async_offsets->asyncio_task_object.task_awaited_by,
        &task_ab_addr);
    if (err) {
        return -1;
    }

    if ((void*)task_ab_addr == NULL) {
        return 0;
    }

    char awaited_by_is_a_set;
    err = read_char(
        handle,
        task_address + async_offsets->asyncio_task_object.task_awaited_by_is_set,
        &awaited_by_is_a_set);
    if (err) {
        return -1;
    }

    if (awaited_by_is_a_set) {
        if (parse_tasks_in_set(
            handle,
            offsets,
            async_offsets,
            task_address + async_offsets->asyncio_task_object.task_awaited_by,
            awaited_by)
         ) {
            return -1;
        }
    } else {
        uintptr_t sub_task;
        if (read_py_ptr(
                handle,
                task_address + async_offsets->asyncio_task_object.task_awaited_by,
                &sub_task)
        ) {
            return -1;
        }

        if (parse_task(
            handle,
            offsets,
            async_offsets,
            sub_task,
            awaited_by)
        ) {
            return -1;
        }
    }

    return 0;
}

static int
parse_code_object(
    proc_handle_t *handle,
    PyObject* result,
    struct _Py_DebugOffsets* offsets,
    uintptr_t address,
    uintptr_t* previous_frame
) {
    uintptr_t address_of_function_name;
    int bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle,
        address + offsets->code_object.name,
        sizeof(void*),
        &address_of_function_name
    );
    if (bytes_read < 0) {
        return -1;
    }

    if ((void*)address_of_function_name == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No function name found");
        return -1;
    }

    PyObject* py_function_name = read_py_str(
        handle, offsets, address_of_function_name, 256);
    if (py_function_name == NULL) {
        return -1;
    }

    if (PyList_Append(result, py_function_name) == -1) {
        Py_DECREF(py_function_name);
        return -1;
    }
    Py_DECREF(py_function_name);

    return 0;
}

static int
parse_frame_object(
    proc_handle_t *handle,
    PyObject* result,
    struct _Py_DebugOffsets* offsets,
    uintptr_t address,
    uintptr_t* previous_frame
) {
    int err;

    Py_ssize_t bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle,
        address + offsets->interpreter_frame.previous,
        sizeof(void*),
        previous_frame
    );
    if (bytes_read < 0) {
        return -1;
    }

    char owner;
    if (read_char(handle, address + offsets->interpreter_frame.owner, &owner)) {
        return -1;
    }

    if (owner >= FRAME_OWNED_BY_INTERPRETER) {
        return 0;
    }

    uintptr_t address_of_code_object;
    err = read_py_ptr(
        handle,
        address + offsets->interpreter_frame.executable,
        &address_of_code_object
    );
    if (err) {
        return -1;
    }

    if ((void*)address_of_code_object == NULL) {
        return 0;
    }

    return parse_code_object(
        handle, result, offsets, address_of_code_object, previous_frame);
}

static int
parse_async_frame_object(
    proc_handle_t *handle,
    PyObject* result,
    struct _Py_DebugOffsets* offsets,
    uintptr_t address,
    uintptr_t* previous_frame,
    uintptr_t* code_object
) {
    int err;

    Py_ssize_t bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle,
        address + offsets->interpreter_frame.previous,
        sizeof(void*),
        previous_frame
    );
    if (bytes_read < 0) {
        return -1;
    }

    char owner;
    bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
        handle, address + offsets->interpreter_frame.owner, sizeof(char), &owner);
    if (bytes_read < 0) {
        return -1;
    }

    if (owner == FRAME_OWNED_BY_CSTACK || owner == FRAME_OWNED_BY_INTERPRETER) {
        return 0;  // C frame
    }

    if (owner != FRAME_OWNED_BY_GENERATOR
        && owner != FRAME_OWNED_BY_THREAD) {
        PyErr_Format(PyExc_RuntimeError, "Unhandled frame owner %d.\n", owner);
        return -1;
    }

    err = read_py_ptr(
        handle,
        address + offsets->interpreter_frame.executable,
        code_object
    );
    if (err) {
        return -1;
    }

    assert(code_object != NULL);
    if ((void*)*code_object == NULL) {
        return 0;
    }

    if (parse_code_object(
        handle, result, offsets, *code_object, previous_frame)) {
        return -1;
    }

    return 1;
}

static int
read_async_debug(
    proc_handle_t *handle,
    struct _Py_AsyncioModuleDebugOffsets* async_debug
) {
    uintptr_t async_debug_addr = _Py_RemoteDebug_GetAsyncioDebugAddress(handle);
    if (!async_debug_addr) {
        return -1;
    }

    size_t size = sizeof(struct _Py_AsyncioModuleDebugOffsets);
    int result = _Py_RemoteDebug_ReadRemoteMemory(handle, async_debug_addr, size, async_debug);
    return result;
}

static int
find_running_frame(
    proc_handle_t *handle,
    uintptr_t runtime_start_address,
    _Py_DebugOffsets* local_debug_offsets,
    uintptr_t *frame
) {
    uint64_t interpreter_state_list_head =
        local_debug_offsets->runtime_state.interpreters_head;

    uintptr_t address_of_interpreter_state;
    int bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
            handle,
            runtime_start_address + interpreter_state_list_head,
            sizeof(void*),
            &address_of_interpreter_state);
    if (bytes_read < 0) {
        return -1;
    }

    if (address_of_interpreter_state == 0) {
        PyErr_SetString(PyExc_RuntimeError, "No interpreter state found");
        return -1;
    }

    uintptr_t address_of_thread;
    bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
            handle,
            address_of_interpreter_state +
                local_debug_offsets->interpreter_state.threads_main,
            sizeof(void*),
            &address_of_thread);
    if (bytes_read < 0) {
        return -1;
    }

    // No Python frames are available for us (can happen at tear-down).
    if ((void*)address_of_thread != NULL) {
        int err = read_ptr(
            handle,
            address_of_thread + local_debug_offsets->thread_state.current_frame,
            frame);
        if (err) {
            return -1;
        }
        return 0;
    }

    *frame = (uintptr_t)NULL;
    return 0;
}

static int
find_running_task(
    proc_handle_t *handle,
    uintptr_t runtime_start_address,
    _Py_DebugOffsets *local_debug_offsets,
    struct _Py_AsyncioModuleDebugOffsets *async_offsets,
    uintptr_t *running_task_addr
) {
    *running_task_addr = (uintptr_t)NULL;

    uint64_t interpreter_state_list_head =
        local_debug_offsets->runtime_state.interpreters_head;

    uintptr_t address_of_interpreter_state;
    int bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
            handle,
            runtime_start_address + interpreter_state_list_head,
            sizeof(void*),
            &address_of_interpreter_state);
    if (bytes_read < 0) {
        return -1;
    }

    if (address_of_interpreter_state == 0) {
        PyErr_SetString(PyExc_RuntimeError, "No interpreter state found");
        return -1;
    }

    uintptr_t address_of_thread;
    bytes_read = _Py_RemoteDebug_ReadRemoteMemory(
            handle,
            address_of_interpreter_state +
                local_debug_offsets->interpreter_state.threads_head,
            sizeof(void*),
            &address_of_thread);
    if (bytes_read < 0) {
        return -1;
    }

    uintptr_t address_of_running_loop;
    // No Python frames are available for us (can happen at tear-down).
    if ((void*)address_of_thread == NULL) {
        return 0;
    }

    bytes_read = read_py_ptr(
        handle,
        address_of_thread
        + async_offsets->asyncio_thread_state.asyncio_running_loop,
        &address_of_running_loop);
    if (bytes_read == -1) {
        return -1;
    }

    // no asyncio loop is now running
    if ((void*)address_of_running_loop == NULL) {
        return 0;
    }

    int err = read_ptr(
        handle,
        address_of_thread
        + async_offsets->asyncio_thread_state.asyncio_running_task,
        running_task_addr);
    if (err) {
        return -1;
    }

    return 0;
}

static int
append_awaited_by_for_thread(
    proc_handle_t *handle,
    uintptr_t head_addr,
    struct _Py_DebugOffsets *debug_offsets,
    struct _Py_AsyncioModuleDebugOffsets *async_offsets,
    PyObject *result
) {
    struct llist_node task_node;

    if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                handle,
                head_addr,
                sizeof(task_node),
                &task_node))
    {
        return -1;
    }

    size_t iteration_count = 0;
    const size_t MAX_ITERATIONS = 2 << 15;  // A reasonable upper bound
    while ((uintptr_t)task_node.next != head_addr) {
        if (++iteration_count > MAX_ITERATIONS) {
            PyErr_SetString(PyExc_RuntimeError, "Task list appears corrupted");
            return -1;
        }

        if (task_node.next == NULL) {
            PyErr_SetString(
                PyExc_RuntimeError,
                "Invalid linked list structure reading remote memory");
            return -1;
        }

        uintptr_t task_addr = (uintptr_t)task_node.next
            - async_offsets->asyncio_task_object.task_node;

        PyObject *tn = parse_task_name(
            handle,
            debug_offsets,
            async_offsets,
            task_addr);
        if (tn == NULL) {
            return -1;
        }

        PyObject *current_awaited_by = PyList_New(0);
        if (current_awaited_by == NULL) {
            Py_DECREF(tn);
            return -1;
        }

        PyObject *result_item = PyTuple_New(2);
        if (result_item == NULL) {
            Py_DECREF(tn);
            Py_DECREF(current_awaited_by);
            return -1;
        }

        PyTuple_SET_ITEM(result_item, 0, tn);  // steals ref
        PyTuple_SET_ITEM(result_item, 1, current_awaited_by);  // steals ref
        if (PyList_Append(result, result_item)) {
            Py_DECREF(result_item);
            return -1;
        }
        Py_DECREF(result_item);

        if (parse_task_awaited_by(handle, debug_offsets, async_offsets,
                                  task_addr, current_awaited_by))
        {
            return -1;
        }

        // onto the next one...
        if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                    handle,
                    (uintptr_t)task_node.next,
                    sizeof(task_node),
                    &task_node))
        {
            return -1;
        }
    }

    return 0;
}

static int
append_awaited_by(
    proc_handle_t *handle,
    unsigned long tid,
    uintptr_t head_addr,
    struct _Py_DebugOffsets *debug_offsets,
    struct _Py_AsyncioModuleDebugOffsets *async_offsets,
    PyObject *result)
{
    PyObject *tid_py = PyLong_FromUnsignedLong(tid);
    if (tid_py == NULL) {
        return -1;
    }

    PyObject *result_item = PyTuple_New(2);
    if (result_item == NULL) {
        Py_DECREF(tid_py);
        return -1;
    }

    PyObject* awaited_by_for_thread = PyList_New(0);
    if (awaited_by_for_thread == NULL) {
        Py_DECREF(tid_py);
        Py_DECREF(result_item);
        return -1;
    }

    PyTuple_SET_ITEM(result_item, 0, tid_py);  // steals ref
    PyTuple_SET_ITEM(result_item, 1, awaited_by_for_thread);  // steals ref
    if (PyList_Append(result, result_item)) {
        Py_DECREF(result_item);
        return -1;
    }
    Py_DECREF(result_item);

    if (append_awaited_by_for_thread(
            handle,
            head_addr,
            debug_offsets,
            async_offsets,
            awaited_by_for_thread))
    {
        return -1;
    }

    return 0;
}

static PyObject*
get_all_awaited_by(PyObject* self, PyObject* args)
{
#if (!defined(__linux__) && !defined(__APPLE__))  && !defined(MS_WINDOWS) || \
    (defined(__linux__) && !HAVE_PROCESS_VM_READV)
    PyErr_SetString(
        PyExc_RuntimeError,
        "get_all_awaited_by is not implemented on this platform");
    return NULL;
#endif

    int pid;
    if (!PyArg_ParseTuple(args, "i", &pid)) {
        return NULL;
    }

    proc_handle_t the_handle;
    proc_handle_t *handle = &the_handle;
    if (_Py_RemoteDebug_InitProcHandle(handle, pid) < 0) {
        return 0;
    }

    uintptr_t runtime_start_addr = _Py_RemoteDebug_GetPyRuntimeAddress(handle);
    if (runtime_start_addr == 0) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(
                PyExc_RuntimeError, "Failed to get .PyRuntime address");
        }
        return NULL;
    }
    struct _Py_DebugOffsets local_debug_offsets;

    if (_Py_RemoteDebug_ReadDebugOffsets(handle, &runtime_start_addr, &local_debug_offsets)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read debug offsets");
        return NULL;
    }

    struct _Py_AsyncioModuleDebugOffsets local_async_debug;
    if (read_async_debug(handle, &local_async_debug)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read asyncio debug offsets");
        return NULL;
    }

    PyObject *result = PyList_New(0);
    if (result == NULL) {
        return NULL;
    }

    uint64_t interpreter_state_list_head =
        local_debug_offsets.runtime_state.interpreters_head;

    uintptr_t interpreter_state_addr;
    if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                handle,
                runtime_start_addr + interpreter_state_list_head,
                sizeof(void*),
                &interpreter_state_addr))
    {
        goto result_err;
    }

    uintptr_t thread_state_addr;
    unsigned long tid = 0;
    if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                handle,
                interpreter_state_addr
                + local_debug_offsets.interpreter_state.threads_head,
                sizeof(void*),
                &thread_state_addr))
    {
        goto result_err;
    }

    uintptr_t head_addr;
    while (thread_state_addr != 0) {
        if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                    handle,
                    thread_state_addr
                    + local_debug_offsets.thread_state.native_thread_id,
                    sizeof(tid),
                    &tid))
        {
            goto result_err;
        }

        head_addr = thread_state_addr
            + local_async_debug.asyncio_thread_state.asyncio_tasks_head;

        if (append_awaited_by(handle, tid, head_addr, &local_debug_offsets,
                              &local_async_debug, result))
        {
            goto result_err;
        }

        if (0 > _Py_RemoteDebug_ReadRemoteMemory(
                    handle,
                    thread_state_addr + local_debug_offsets.thread_state.next,
                    sizeof(void*),
                    &thread_state_addr))
        {
            goto result_err;
        }
    }

    head_addr = interpreter_state_addr
        + local_async_debug.asyncio_interpreter_state.asyncio_tasks_head;

    // On top of a per-thread task lists used by default by asyncio to avoid
    // contention, there is also a fallback per-interpreter list of tasks;
    // any tasks still pending when a thread is destroyed will be moved to the
    // per-interpreter task list.  It's unlikely we'll find anything here, but
    // interesting for debugging.
    if (append_awaited_by(handle, 0, head_addr, &local_debug_offsets,
                        &local_async_debug, result))
    {
        goto result_err;
    }

    _Py_RemoteDebug_CleanupProcHandle(handle);
    return result;

result_err:
    Py_DECREF(result);
    _Py_RemoteDebug_CleanupProcHandle(handle);
    return NULL;
}

static PyObject*
get_stack_trace(PyObject* self, PyObject* args)
{
#if (!defined(__linux__) && !defined(__APPLE__))  && !defined(MS_WINDOWS) || \
    (defined(__linux__) && !HAVE_PROCESS_VM_READV)
    PyErr_SetString(
        PyExc_RuntimeError,
        "get_stack_trace is not supported on this platform");
    return NULL;
#endif

    int pid;
    if (!PyArg_ParseTuple(args, "i", &pid)) {
        return NULL;
    }

    proc_handle_t the_handle;
    proc_handle_t *handle = &the_handle;
    if (_Py_RemoteDebug_InitProcHandle(handle, pid) < 0) {
        return 0;
    }

    PyObject* result = NULL;

    uintptr_t runtime_start_address = _Py_RemoteDebug_GetPyRuntimeAddress(handle);
    if (runtime_start_address == 0) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(
                PyExc_RuntimeError, "Failed to get .PyRuntime address");
        }
        goto result_err;
    }
    struct _Py_DebugOffsets local_debug_offsets;

    if (_Py_RemoteDebug_ReadDebugOffsets(handle, &runtime_start_address, &local_debug_offsets)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read debug offsets");
        goto result_err;
    }

    uintptr_t address_of_current_frame;
    if (find_running_frame(
        handle, runtime_start_address, &local_debug_offsets,
        &address_of_current_frame)
    ) {
        goto result_err;
    }

    result = PyList_New(0);
    if (result == NULL) {
        goto result_err;
    }

    while ((void*)address_of_current_frame != NULL) {
        if (parse_frame_object(
                    handle,
                    result,
                    &local_debug_offsets,
                    address_of_current_frame,
                    &address_of_current_frame)
            < 0)
        {
            Py_DECREF(result);
            goto result_err;
        }
    }

result_err:
    _Py_RemoteDebug_CleanupProcHandle(handle);
    return result;
}

static PyObject*
get_async_stack_trace(PyObject* self, PyObject* args)
{
#if (!defined(__linux__) && !defined(__APPLE__))  && !defined(MS_WINDOWS) || \
    (defined(__linux__) && !HAVE_PROCESS_VM_READV)
    PyErr_SetString(
        PyExc_RuntimeError,
        "get_stack_trace is not supported on this platform");
    return NULL;
#endif
    int pid;

    if (!PyArg_ParseTuple(args, "i", &pid)) {
        return NULL;
    }

    proc_handle_t the_handle;
    proc_handle_t *handle = &the_handle;
    if (_Py_RemoteDebug_InitProcHandle(handle, pid) < 0) {
        return 0;
    }

    uintptr_t runtime_start_address = _Py_RemoteDebug_GetPyRuntimeAddress(handle);
    if (runtime_start_address == 0) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(
                PyExc_RuntimeError, "Failed to get .PyRuntime address");
        }
        return NULL;
    }
    struct _Py_DebugOffsets local_debug_offsets;

    if (_Py_RemoteDebug_ReadDebugOffsets(handle, &runtime_start_address, &local_debug_offsets)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read debug offsets");
        return NULL;
    }

    struct _Py_AsyncioModuleDebugOffsets local_async_debug;
    if (read_async_debug(handle, &local_async_debug)) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read asyncio debug offsets");
        return NULL;
    }

    PyObject* result = PyList_New(1);
    if (result == NULL) {
        return NULL;
    }
    PyObject* calls = PyList_New(0);
    if (calls == NULL) {
        Py_DECREF(result);
        return NULL;
    }
    if (PyList_SetItem(result, 0, calls)) { /* steals ref to 'calls' */
        Py_DECREF(result);
        Py_DECREF(calls);
        return NULL;
    }

    uintptr_t running_task_addr = (uintptr_t)NULL;
    if (find_running_task(
        handle, runtime_start_address, &local_debug_offsets, &local_async_debug,
        &running_task_addr)
    ) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to find running task");
        goto result_err;
    }

    if ((void*)running_task_addr == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "No running task found");
        goto result_err;
    }

    uintptr_t running_coro_addr;
    if (read_py_ptr(
        handle,
        running_task_addr + local_async_debug.asyncio_task_object.task_coro,
        &running_coro_addr
    )) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to read running task coro");
        goto result_err;
    }

    if ((void*)running_coro_addr == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Running task coro is NULL");
        goto result_err;
    }

    // note: genobject's gi_iframe is an embedded struct so the address to
    // the offset leads directly to its first field: f_executable
    uintptr_t address_of_running_task_code_obj;
    if (read_py_ptr(
        handle,
        running_coro_addr + local_debug_offsets.gen_object.gi_iframe,
        &address_of_running_task_code_obj
    )) {
        goto result_err;
    }

    if ((void*)address_of_running_task_code_obj == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Running task code object is NULL");
        goto result_err;
    }

    uintptr_t address_of_current_frame;
    if (find_running_frame(
        handle, runtime_start_address, &local_debug_offsets,
        &address_of_current_frame)
    ) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to find running frame");
        goto result_err;
    }

    uintptr_t address_of_code_object;
    while ((void*)address_of_current_frame != NULL) {
        int res = parse_async_frame_object(
            handle,
            calls,
            &local_debug_offsets,
            address_of_current_frame,
            &address_of_current_frame,
            &address_of_code_object
        );

        if (res < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to parse async frame object");
            goto result_err;
        }

        if (address_of_code_object == address_of_running_task_code_obj) {
            break;
        }
    }

    PyObject *tn = parse_task_name(
        handle, &local_debug_offsets, &local_async_debug, running_task_addr);
    if (tn == NULL) {
        goto result_err;
    }
    if (PyList_Append(result, tn)) {
        Py_DECREF(tn);
        goto result_err;
    }
    Py_DECREF(tn);

    PyObject* awaited_by = PyList_New(0);
    if (awaited_by == NULL) {
        goto result_err;
    }
    if (PyList_Append(result, awaited_by)) {
        Py_DECREF(awaited_by);
        goto result_err;
    }
    Py_DECREF(awaited_by);

    if (parse_task_awaited_by(
        handle, &local_debug_offsets, &local_async_debug,
        running_task_addr, awaited_by)
    ) {
        goto result_err;
    }

    _Py_RemoteDebug_CleanupProcHandle(handle);
    return result;

result_err:
    _Py_RemoteDebug_CleanupProcHandle(handle);
    Py_DECREF(result);
    return NULL;
}


static PyMethodDef methods[] = {
    {"get_stack_trace", get_stack_trace, METH_VARARGS,
        "Get the Python stack from a given pod"},
    {"get_async_stack_trace", get_async_stack_trace, METH_VARARGS,
        "Get the asyncio stack from a given pid"},
    {"get_all_awaited_by", get_all_awaited_by, METH_VARARGS,
        "Get all tasks and their awaited_by from a given pid"},
    {NULL, NULL, 0, NULL},
};

static struct PyModuleDef module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "_testexternalinspection",
    .m_size = -1,
    .m_methods = methods,
};

PyMODINIT_FUNC
PyInit__testexternalinspection(void)
{
    PyObject* mod = PyModule_Create(&module);
    if (mod == NULL) {
        return NULL;
    }
#ifdef Py_GIL_DISABLED
    PyUnstable_Module_SetGIL(mod, Py_MOD_GIL_NOT_USED);
#endif
    int rc = PyModule_AddIntConstant(
        mod, "PROCESS_VM_READV_SUPPORTED", HAVE_PROCESS_VM_READV);
    if (rc < 0) {
        Py_DECREF(mod);
        return NULL;
    }
    return mod;
}
