/*void aqvm_invoke_python(size_t return_value, InternalObject args) {
    // TRACE_FUNCTION; // Uncomment if you have TRACE_FUNCTION defined

    // 1. Ensure Python Interpreter is initialized
    //    Consider doing this once globally at VM startup for performance.
    if (!Py_IsInitialized()) {
        Py_InitializeEx(0); // Initialize without signal handlers
        // Consider atexit(Py_Finalize); // Optional: Auto-cleanup on exit
    }

    // 2. Get Python code string from the first argument
    if (args.size < 1) {
        EXIT_VM("aqvm_invoke_python", "Python code string argument missing.");
    }
    size_t code_str_index = args.index[0];
    // Assuming type 0x05 is string - adjust if different
    if (object_table[code_str_index].type == NULL || object_table[code_str_index].type[0] != 0x05) {
         EXIT_VM("aqvm_invoke_python", "First argument must be a string (Python code).");
    }
    const char* python_code = GetStringData(code_str_index);
    if (python_code == NULL) {
        EXIT_VM("aqvm_invoke_python", "Failed to get Python code string.");
    }

    // 3. Argument Conversion (AQ -> Python) - Placeholder
    //    This is complex and needs mapping AQ types to Python types.
    //    Example: Convert args.index[1], args.index[2], ... to PyObject*s
    //    and potentially put them in the Python execution context (e.g., pLocalDict).
    //    For now, we assume the python_code is self-contained.

    // 4. Python Execution Context
    PyObject *pGlobalDict = PyDict_New(); // Use separate dicts for safety
    PyObject *pLocalDict = PyDict_New();
    PyObject *pMainModule = PyImport_AddModule("__main__");
    if (pGlobalDict == NULL || pLocalDict == NULL || pMainModule == NULL) {
        Py_XDECREF(pGlobalDict);
        Py_XDECREF(pLocalDict);
        EXIT_VM("aqvm_invoke_python", "Failed to create Python dictionaries or get __main__ module.");
    }
    // Copy builtins into global dict to make standard functions available
    PyDict_Update(pGlobalDict, PyModule_GetDict(PyImport_AddModule("builtins")));

    // 5. Execute Python Code
    PyObject* pResult = PyRun_String(python_code, Py_eval_input, pGlobalDict, pLocalDict);
    // Try Py_file_input if Py_eval_input fails (e.g., for multi-statement code)
    if (pResult == NULL && PyErr_Occurred()) {
        PyErr_Clear(); // Clear the eval error
        pResult = PyRun_String(python_code, Py_file_input, pGlobalDict, pLocalDict);
    }

    // 6. Error Handling
    if (pResult == NULL) {
        PyErr_Print(); // Print Python error to stderr
        fprintf(stderr, "[ERROR] aqvm_invoke_python: Python execution failed for code:\n%s\n", python_code);
        SetObjectToNull(return_value); // Set AQ return value to null/void on error
        Py_DECREF(pGlobalDict);
        Py_DECREF(pLocalDict);
        return; // Or EXIT_VM depending on desired behavior
    }

    // 7. Return Value Conversion (Python -> AQ) - Placeholder
    //    Convert pResult (PyObject*) back to an AQ Object.
    if (PyLong_Check(pResult)) {
        long result_val = PyLong_AsLong(pResult);
        if (PyErr_Occurred()) { // Check for overflow
            PyErr_Print();
            fprintf(stderr, "[WARN] aqvm_invoke_python: Python long conversion failed.\n");
            SetObjectToNull(return_value);
        } else {
            SetLongData(return_value, (int64_t)result_val); // Assuming AQ uses int64_t
        }
    } else if (PyUnicode_Check(pResult)) {
        const char* result_str = PyUnicode_AsUTF8(pResult);
        if (result_str == NULL) {
             PyErr_Print();
             fprintf(stderr, "[WARN] aqvm_invoke_python: Python string conversion failed.\n");
             SetObjectToNull(return_value);
        } else {
            SetStringData(return_value, result_str);
        }
    } else if (PyFloat_Check(pResult)) {
        double result_double = PyFloat_AsDouble(pResult);
         if (PyErr_Occurred()) {
            PyErr_Print();
            fprintf(stderr, "[WARN] aqvm_invoke_python: Python float conversion failed.\n");
            SetObjectToNull(return_value);
        } else {
            // SetDoubleData(return_value, result_double); // Assuming you have SetDoubleData
            fprintf(stderr, "[WARN] aqvm_invoke_python: AQ double return type not implemented yet.\n");
            SetObjectToNull(return_value);
        }
    } else if (pResult == Py_None) {
         SetObjectToNull(return_value);
    } else {
        // Handle other Python types (list, dict, etc.) or report unsupported type
        fprintf(stderr, "[WARN] aqvm_invoke_python: Unsupported Python return type.\n");
        SetObjectToNull(return_value);
    }

    // 8. Cleanup
    Py_DECREF(pResult);
    Py_DECREF(pGlobalDict);
    Py_DECREF(pLocalDict);
}
*/
/*
#include <Python.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // 1. 初始化 Python 解释器
    // Py_SetProgramName(argv[0]); // 可选：告知解释器程序路径
    Py_InitializeEx(0); // 使用 0 参数避免注册信号处理程序

    // 检查初始化是否成功
    if (!Py_IsInitialized()) {
        fprintf(stderr, "Error initializing Python interpreter\n");
        return 1;
    }

    // 2. 要执行的 Python 代码
    const char* python_code = "print('Hello from Python!')";

    // 3. 执行 Python 代码字符串
    // PyRun_SimpleStringFlags 是一个简单的接口，它在 __main__ 模块的上下文中执行代码
    // 它会打印任何输出到标准输出，并打印错误到标准错误
    int result = PyRun_SimpleStringFlags(python_code, NULL);

    // 检查执行结果
    if (result != 0) {
        fprintf(stderr, "Error executing Python code\n");
        // 如果 PyRun_SimpleStringFlags 失败，Python 错误信息通常已经打印
        // PyErr_Print(); // 可以取消注释以确保打印（但 SimpleString 通常会处理）
    }

    // 4. 关闭 Python 解释器并释放资源
    // 检查 Py_FinalizeEx 的返回值是个好习惯
    if (Py_FinalizeEx() < 0) {
        fprintf(stderr, "Error finalizing Python interpreter\n");
        // 通常这意味着有未处理的 Python 异常
        // PyErr_Print(); // 可以尝试打印最后的错误
        return 120; // 根据 Python 文档建议的退出码
    }

    printf("C program finished successfully.\n");
    return 0;
}*/

#include <Python.h>

int main(int argc, char *argv[]) {
    Py_InitializeEx(0); // 初始化 Python 解释器 (0 表示不初始化信号处理)

    const char* python_code = 
        "import math\n"
        "try:\n"
        "    result = math.sqrt(16)\n"
        "    print(f'The square root from C is: {result}')\n"
        "except Exception as e:\n"
        "    print(f'Python error: {e}')\n";

    // 执行 Python 代码字符串
    int ret = PyRun_SimpleStringFlags(python_code, NULL);

    if (ret != 0) {
        fprintf(stderr, "Error executing Python code.\n");
        if (PyErr_Occurred()) {
            PyErr_Print(); // 打印 Python 异常信息
        }
    }

    if (Py_FinalizeEx() < 0) { // 关闭 Python 解释器
        exit(120);
    }
    return ret;
}