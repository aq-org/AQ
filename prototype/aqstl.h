// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif

/*typedef struct StackNode {
  char* function_name;
  struct StackNode* next;
} StackNode;

StackNode* call_stack = NULL;

void PushStack(const char* function_name) {
  StackNode* new_node = (StackNode*)malloc(sizeof(StackNode));
  new_node->function_name = strdup(function_name);
  new_node->next = call_stack;
  call_stack = new_node;
}

void PopStack() {
  if (call_stack != NULL) {
    StackNode* temp = call_stack;
    call_stack = call_stack->next;
    free(temp->function_name);
    free(temp);
  }
}

void PrintStackRecursive(StackNode* node) {
  if (node == NULL) {
    printf("[INFO] Run: ");
    return;
  }
  PrintStackRecursive(node->next);
  printf("%s -> ", node->function_name);
}

void PrintStack() {
  PrintStackRecursive(call_stack);
  printf("Success\n");
}

typedef struct Trace {
  const char* function_name;
} Trace;

Trace TraceCreate(const char* function_name) {
  Trace trace;
  trace.function_name = function_name;
  PushStack(function_name);
  PrintStack();
  return trace;
}

void TraceDestroy(Trace* trace) {
  if (trace) {
    PopStack();
    PrintStack();
  }
}

#define Trace(trace)                       \
  Trace trace = TraceCreate(__FUNCTION__); \
  __attribute__((cleanup(TraceDestroy))) Trace* trace_ptr = &trace;

#define TRACE_FUNCTION                                  \
  Trace _trace __attribute__((cleanup(TraceDestroy))) = \
      TraceCreate(__FUNCTION__)
*/

#define TRACE_FUNCTION

void EXIT_VM(const char* func_name, const char* message);

struct Object* object_table;

size_t object_table_size;

union Data {
  int8_t byte_data;
  int64_t long_data;
  double double_data;
  uint64_t uint64t_data;
  const char* string_data;
  struct Object* ptr_data;
  struct Object* reference_data;
  struct Object* const_data;
  struct Object* object_data;
};

struct Object {
  uint8_t* type;
  bool const_type;
  union Data data;
};

typedef struct {
  size_t size;
  size_t* index;
} InternalObject;

typedef void (*func_ptr)(InternalObject, size_t);

struct Pair {
  char* first;
  func_ptr second;
};

struct LinkedList {
  struct Pair pair;
  struct LinkedList* next;
};

struct LinkedList name_table[256];

void AddFreePtr(void* ptr);

struct Object* GetOriginData(struct Object* object);
struct Object* GetObjectData(size_t index);
struct Object* GetObjectObjectData(struct Object* data);
const char* GetStringData(size_t index);
const char* GetStringObjectData(struct Object* object);
uint64_t GetUint64tObjectData(struct Object* object);
uint64_t GetUint64tData(size_t index);
int64_t GetLongObjectData(struct Object* object);
double GetDoubleObjectData(struct Object* object);
double GetDoubleData(size_t index);
int64_t GetLongData(size_t index);
int8_t GetByteObjectData(struct Object* data);
int8_t GetByteData(size_t index);
struct Object* GetPtrObjectData(struct Object* object);
struct Object* GetPtrData(size_t index);
void SetObjectData(size_t index, struct Object* object);
void SetByteData(size_t, int8_t);
void SetLongData(size_t, int64_t);
void SetDoubleData(size_t, double);
void SetUint64tData(size_t, uint64_t);
void SetPtrData(size_t index, struct Object* ptr);
void SetStringData(size_t index, const char* string);
void SetReferenceData(size_t index, struct Object* object);
void SetConstData(size_t index, struct Object* object);
void SetObjectObjectData(struct Object* data, struct Object* object);
void SetByteObjectData(struct Object* data, int8_t);
void SetLongObjectData(struct Object* data, int64_t);
void SetDoubleObjectData(struct Object* data, double);
void SetUint64tObjectData(struct Object* data, uint64_t);
void SetPtrObjectData(struct Object* data, struct Object* ptr);
void SetStringObjectData(struct Object* data, const char* string);
void SetReferenceObjectData(struct Object* data, struct Object* object);
void SetConstObjectData(struct Object* data, struct Object* object);
void SetObjectObjectData(struct Object* data, struct Object* object);

void aqstl_print(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + *args.index;
  object = GetOriginData(object);
  if (object == NULL)
    EXIT_VM("aqstl_print(InternalObject,size_t)", "Invalid object.");

  switch (object->type[0]) {
    case 0x01:
      SetLongData(return_value, printf("%d", GetByteData(*args.index)));
      break;
    case 0x02:
      // printf("print long");
      SetLongData(return_value, printf("%lld", GetLongData(*args.index)));
      break;
    case 0x03:
      SetLongData(return_value, printf("%.15f", GetDoubleData(*args.index)));
      break;
    case 0x04:
      SetLongData(return_value, printf("%zu", GetUint64tData(*args.index)));
      break;
    case 0x05:
      SetLongData(return_value, printf("%s", GetStringData(*args.index)));
      break;
    case 0x06:
      SetLongData(return_value, printf("%p", GetPtrData(*args.index)));
      break;
    default:
      EXIT_VM("aqstl_print(InternalObject,size_t)", "Unsupported type.");
      break;
  }
}

void aqstl_vaprint(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + args.index[0];
  object = GetOriginData(object);

  if (object == NULL || object->type == NULL || object->type[0] != 0x06)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid object.");

  for (size_t i = 1; i < GetUint64tObjectData(object->data.ptr_data) + 1; i++) {
    switch (GetOriginData(object->data.ptr_data + i)->type[0]) {
      case 0x01:
        SetLongData(return_value,
                    printf("%d", GetByteObjectData(object->data.ptr_data + i)));
        break;
      case 0x02:
        SetLongData(
            return_value,
            printf("%lld", GetLongObjectData(object->data.ptr_data + i)));
        break;
      case 0x03:
        SetLongData(
            return_value,
            printf("%.15f", GetDoubleObjectData(object->data.ptr_data + i)));
        break;
      case 0x04:
        SetLongData(
            return_value,
            printf("%zu", GetUint64tObjectData(object->data.ptr_data + i)));
        break;
      case 0x05:
        SetLongData(
            return_value,
            printf("%s", GetStringObjectData(object->data.ptr_data + i)));
        break;
      case 0x06:
        SetLongData(return_value,
                    printf("%p", GetPtrObjectData(object->data.ptr_data + i)));
        break;
      default:
        printf("Type: %u\n", (object->data.ptr_data + i)->type[0]);
        EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Unsupported type.");
        break;
    }
  }
}

void aqstl_remove(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_remove(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_remove(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value, remove(GetStringData(*args.index)));
}
void aqstl_rename(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_rename(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_rename(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value, rename(GetStringData(*args.index),
                                   GetStringData(*(args.index + 1))));
}

void aqstl_pp(InternalObject args, size_t return_value);
// pprint module implementation

// 定义PrettyPrinter结构体保存格式化参数
struct PrettyPrinter {
    int indent;
    int width;
    int depth;
    bool compact;
    bool sort_dicts;
    bool underscore_numbers;
    struct Memo* memo;
};

// 递归对象检测备忘录（复用深拷贝备忘录结构扩展）
struct Memo {
    struct Object* original;
    struct Object* copy;
    struct Memo* next;
};

// 辅助函数：生成递归对象表示字符串
static const char* get_recursion_repr(struct Object* obj) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "<Recursion on %s with id=%p>", obj->type ? "object" : "unknown", obj);
    return buf;
}

// 辅助函数：处理数字千位分隔符
static void format_underscore_numbers(char* num_str) {
    if (num_str == NULL || *num_str == '\0') return;
    size_t len = strlen(num_str);
    if (len <= 3) return;  // 不足三位不需要分隔

    // 处理符号位（如果有）
    int sign = 0;
    if (num_str[0] == '-' || num_str[0] == '+') {
        sign = 1;
        len--;
        num_str++;
    }

    // 从右往左每三位插入下划线（忽略小数点后的部分）
    char* decimal_point = strchr(num_str, '.');
    size_t int_part_len = decimal_point ? (decimal_point - num_str) : len;
    int count = 0;
    for (int i = int_part_len - 1; i > 0; i--) {
        if (isdigit(num_str[i])) {
            count++;
            if (count == 3) {
                memmove(num_str + i + 1, num_str + i, len - i + 1);
                num_str[i] = '_';
                count = 0;
                len++;
                if (decimal_point) decimal_point++;  // 调整小数点位置
            }
        }
    }
    // 恢复符号位
    if (sign) num_str--;
}

// 键值对结构体定义
struct KeyValuePair {
    struct Object key;
    struct Object value;
};

// 对象比较函数（用于集合排序）
static int compare_objects(const void* a, const void* b) {
    struct Object* obj_a = (struct Object*)a;
    struct Object* obj_b = (struct Object*)b;
    // 先比较类型
    if (obj_a->type[0] != obj_b->type[0]) {
        return (int)(obj_a->type[0] - obj_b->type[0]);
    }
    // 同类型时比较值
    switch (obj_a->type[0]) {
        case 0x02: {  // 整数类型
            int64_t val_a = GetLongObjectData(obj_a);
            int64_t val_b = GetLongObjectData(obj_b);
            return (val_a > val_b) ? 1 : (val_a < val_b) ? -1 : 0;
        }
        case 0x03: {  // 浮点数类型
            double val_a = GetDoubleObjectData(obj_a);
            double val_b = GetDoubleObjectData(obj_b);
            return (val_a > val_b) ? 1 : (val_a < val_b) ? -1 : 0;
        }
        case 0x05: {  // 字符串类型
            const char* str_a = GetStringObjectData(obj_a);
            const char* str_b = GetStringObjectData(obj_b);
            return strcmp(str_a, str_b);
        }
        default:  // 其他类型视为相等
            return 0;
    }
}

// 键值对比较函数（用于字典排序）
static int compare_pairs(const void* a, const void* b) {
    struct KeyValuePair* pair_a = (struct KeyValuePair*)a;
    struct KeyValuePair* pair_b = (struct KeyValuePair*)b;
    return compare_objects(&pair_a->key, &pair_b->key);
}

// 辅助函数：格式化对象核心逻辑
static void pformat_object(struct Object* obj, struct PrettyPrinter* pp, char* buffer, size_t* buf_len) {
    // 检测递归对象
    struct Memo* current = pp->memo;
    while (current) {
        if (current->original == obj) {
            const char* repr = get_recursion_repr(obj);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "%s", repr);
            return;
        }
        current = current->next;
    }

    // 添加当前对象到备忘录
    struct Memo* new_memo = (struct Memo*)malloc(sizeof(struct Memo));
    new_memo->original = obj;
    new_memo->next = pp->memo;
    pp->memo = new_memo;

    // 根据类型格式化
    switch (obj->type[0]) {
        case 0x05: {  // 字符串类型
            const char* str = GetStringObjectData(obj);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "'%s'", str);
            break;
        }
        case 0x06: {  // 指针（动态数组）类型
            if (pp->depth == 0) {
                *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "[...]");
                break;
            }
            pp->depth--;
            size_t elem_count = GetUint64tObjectData(obj->data.ptr_data);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "[");
            for (size_t i = 0; i < elem_count; i++) {
                if (i > 0) {
                    if (pp->compact) {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ", ");
                    } else {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "\n%%*s", pp->indent, "");
                    }
                }
                pformat_object(obj->data.ptr_data + i + 1, pp, buffer, buf_len);
            }
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "]");
            pp->depth++;
            break;
        }
        case 0x07: {  // 引用类型
            if (pp->depth == 0) {
                *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "(...)");
                break;
            }
            pp->depth--;
            size_t elem_count = GetUint64tObjectData(obj->data.ptr_data);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "(");
            for (size_t i = 0; i < elem_count; i++) {
                if (i > 0) {
                    if (pp->compact) {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ", ");
                    } else {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "\n%%*s", pp->indent, "");
                    }
                }
                pformat_object(obj->data.ptr_data + i + 1, pp, buffer, buf_len);
            }
            if (elem_count == 1) *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ",");
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ")");
            pp->depth++;
            break;
        }
        case 0x08: {  // 常量引用类型
            if (pp->depth == 0) {
                *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "{...}");
                break;
            }
            pp->depth--;
            size_t elem_count = GetUint64tObjectData(obj->data.ptr_data);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "{");
            struct Object* elems = obj->data.ptr_data + 1;
            if (pp->sort_dicts) qsort(elems, elem_count, sizeof(struct Object), compare_objects);
            for (size_t i = 0; i < elem_count; i++) {
                if (i > 0) {
                    if (pp->compact) {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ", ");
                    } else {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "\n%%*s", pp->indent, "");
                    }
                }
                pformat_object(&elems[i], pp, buffer, buf_len);
            }
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "}");
            pp->depth++;
            break;
        }
        case 0x09: {  // 类类型
            if (pp->depth == 0) {
                *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "{...}");
                break;
            }
            pp->depth--;
            size_t pair_count = GetUint64tObjectData(obj->data.ptr_data);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "{");
            struct KeyValuePair* pairs = (struct KeyValuePair*)(obj->data.ptr_data + 1);
            if (pp->sort_dicts) qsort(pairs, pair_count, sizeof(struct KeyValuePair), compare_pairs);
            for (size_t i = 0; i < pair_count; i++) {
                if (i > 0) {
                    if (pp->compact) {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ", ");
                    } else {
                        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "\n%%*s", pp->indent, "");
                    }
                }
                pformat_object(&pairs[i].key, pp, buffer, buf_len);
                *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, ": ");
                pformat_object(&pairs[i].value, pp, buffer, buf_len);
            }
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "}");
            pp->depth++;
            break;
        }
        case 0x02: {  // 整数类型（长整型）
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%lld", GetLongObjectData(obj));
            if (pp->underscore_numbers) format_underscore_numbers(num_buf);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "%s", num_buf);
            break;
        }
        case 0x03: {  // 浮点数类型
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%.15g", GetDoubleObjectData(obj));
            if (pp->underscore_numbers) format_underscore_numbers(num_buf);
            *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "%s", num_buf);
            break;
        }
        default:
            EXIT_VM("pformat_object", "Unsupported object type for pretty printing");
            break;
    }

    // 移除当前备忘录条目
    pp->memo = new_memo->next;
    free(new_memo);
}

// 新增：指针类型（复合对象）处理（原case 0x06逻辑迁移）
static void pformat_compound_object(struct Object* obj, struct PrettyPrinter* pp, char* buffer, size_t* buf_len) {
    if (pp->depth > 0) {
        pp->depth--;
        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "{");
        pformat_object(obj->data.ptr_data, pp, buffer, buf_len);
        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "}");
        pp->depth++;
    } else {
        *buf_len += snprintf(buffer + *buf_len, sizeof(buffer) - *buf_len, "...");
    }
}

// 实现pp函数
void aqstl_pp(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 1 || args.size > 8)
        EXIT_VM("aqstl_pp", "Invalid args, requires 1-7 arguments (object, stream, indent, width, depth, compact, sort_dicts, underscore_numbers)");
    if (return_value >= object_table_size)
        EXIT_VM("aqstl_pp", "Invalid return value slot");

    struct Object* obj = GetOriginData(object_table + args.index[0]);
    struct PrettyPrinter pp = {
        .indent = (int)(args.size >= 3 ? GetLongObjectData(object_table + args.index[2]) : 1),
        .width = (int)(args.size >= 4 ? GetLongObjectData(object_table + args.index[3]) : 80),
        .depth = (int)(args.size >= 5 ? GetLongObjectData(object_table + args.index[4]) : -1),
        .compact = (bool)(args.size >= 6 ? GetLongObjectData(object_table + args.index[5]) : false),
        .sort_dicts = (bool)(args.size >= 7 ? GetLongObjectData(object_table + args.index[6]) : true),
        .underscore_numbers = (bool)(args.size >= 8 ? GetLongObjectData(object_table + args.index[7]) : false),
        .memo = NULL
    };

    char buffer[1024] = {0};
    size_t buf_len = 0;
    pformat_object(obj, &pp, buffer, &buf_len);

    // 输出到stream（示例使用stdout）
    printf("%s\n", buffer);
    SetStringData(return_value, buffer);
}

// pprint是pp的别名（默认sort_dicts=true）
#define aqstl_pprint aqstl_pp

// 实现os.path.dirname函数
/*void aqstl_path_dirname(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_dirname", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_dirname", "参数必须是字符串或路径对象");
    }

    // 查找最后一个路径分隔符（跨平台）
    const char* last_sep = NULL;
#ifdef _WIN32
    last_sep = strpbrk(path, "\\");
    while (last_sep) {
        const char* next = strpbrk(last_sep + 1, "\\");
        if (!next) break;
        last_sep = next;
    }
#else
    last_sep = strrchr(path, '/');
#endif
    const char* dirname = (last_sep && last_sep > path) ? path : "";
    if (last_sep) {
        // 截断到最后一个分隔符前
        size_t dir_len = last_sep - path;
        char* dir_buf = (char*)malloc(dir_len + 1);
        strncpy(dir_buf, path, dir_len);
        dir_buf[dir_len] = '\0';
        dirname = dir_buf;
    }

    // 创建返回字符串对象
    SetStringData(return_value, dirname);
    if (last_sep) free((void*)dirname);
}*/

// 实现os.path.expanduser函数
void aqstl_path_expanduser(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_expanduser", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_expanduser", "参数必须是字符串或路径对象");
    }

    char expanded_path[1024];
#ifdef _WIN32
    // Windows家目录获取逻辑
    char* home = getenv("USERPROFILE");
    if (!home) {
        char drive[3] = {0};
        char path[256] = {0};
        GetEnvironmentVariableA("HOMEDRIVE", drive, sizeof(drive));
        GetEnvironmentVariableA("HOMEPATH", path, sizeof(path));
        snprintf(expanded_path, sizeof(expanded_path), "%s%s", drive, path);
    } else {
        strncpy(expanded_path, home, sizeof(expanded_path)-1);
    }
#else
    // Unix家目录获取逻辑
    char* home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw ? pw->pw_dir : "/";
    }
    strncpy(expanded_path, home, sizeof(expanded_path)-1);
#endif

    // 替换路径中的~符号
    if (strstr(path, "~/") == path) {
        strcat(expanded_path, path + 1);
    } else {
        strncpy(expanded_path, path, sizeof(expanded_path)-1);
    }
    expanded_path[sizeof(expanded_path)-1] = '\0';

    // 创建返回字符串对象
    SetStringData(return_value, expanded_path);
}


// 实现pformat函数
void aqstl_pformat(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 1 || args.size > 8)
        EXIT_VM("aqstl_pformat", "Invalid args, requires 1-6 arguments (object, indent, width, depth, compact, sort_dicts, underscore_numbers)");
    if (return_value >= object_table_size)
        EXIT_VM("aqstl_pformat", "Invalid return value slot");

    struct Object* obj = GetOriginData(object_table + args.index[0]);
    struct PrettyPrinter pp = {
      .indent = (int)(args.size >= 3 ? GetLongObjectData(object_table + args.index[2]) : 1),
      .width = (int)(args.size >= 4 ? GetLongObjectData(object_table + args.index[3]) : 80),
      .depth = (int)(args.size >= 5 ? GetLongObjectData(object_table + args.index[4]) : -1),
      .compact = (bool)(args.size >= 6 ? GetLongObjectData(object_table + args.index[5]) : false),
      .sort_dicts = (bool)(args.size >= 7 ? GetLongObjectData(object_table + args.index[6]) : true),
      .underscore_numbers = (bool)(args.size >= 8 ? GetLongObjectData(object_table + args.index[7]) : false),
      .memo = NULL
  };

    char buffer[1024] = {0};
    size_t buf_len = 0;
    pformat_object(obj, &pp, buffer, &buf_len);
    SetStringData(return_value, buffer);
}

// copy module implementation


// 浅拷贝函数实现
void aqstl_copy(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_copy", "Invalid args, requires 1 argument");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_copy", "Invalid return value slot");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_copy", "Null object");

  // 创建浅拷贝对象（直接复制引用）
  struct Object* copy_obj = (struct Object*)malloc(sizeof(struct Object));
  copy_obj->type = obj->type;
  copy_obj->const_type = obj->const_type;
  copy_obj->data = obj->data;
  SetObjectData(return_value, copy_obj);
}

// 深拷贝辅助函数（带备忘录）
static struct Object* deep_copy(struct Object* obj, struct Memo** memo) {
  if (!obj) return NULL;

  // 检查备忘录中是否已复制过该对象
  struct Memo* current = *memo;
  while (current) {
    if (current->original == obj) return current->copy;
    current = current->next;
  }

  // 创建新备忘录条目
  struct Memo* new_memo = (struct Memo*)malloc(sizeof(struct Memo));
  new_memo->original = obj;
  new_memo->next = *memo;
  *memo = new_memo;

  // 根据对象类型递归复制
  struct Object* copy_obj = (struct Object*)malloc(sizeof(struct Object));
  copy_obj->type = obj->type;
  copy_obj->const_type = obj->const_type;
  switch (obj->type[0]) {
    case 0x05:  // 字符串类型（不可变，浅拷贝即可）
      copy_obj->data.string_data = obj->data.string_data;
      break;
    case 0x06:  // 指针类型（假设为复合对象）
      copy_obj->data.ptr_data = deep_copy(obj->data.ptr_data, memo);
      break;
    default:
      copy_obj->data = obj->data;
      break;
  }
  new_memo->copy = copy_obj;
  return copy_obj;
}

// 深拷贝函数实现
void aqstl_deepcopy(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_deepcopy", "Invalid args, requires 1 argument");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_deepcopy", "Invalid return value slot");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_deepcopy", "Null object");

  struct Memo* memo = NULL;
  struct Object* copy_obj = deep_copy(obj, &memo);
  SetObjectData(return_value, copy_obj);

  // 释放备忘录
  struct Memo* current = memo;
  while (current) {
    struct Memo* temp = current;
    current = current->next;
    free(temp);
  }
}

// 替换函数实现（简化版，假设处理类似namedtuple的对象）
void aqstl_replace(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_replace", "Invalid args, requires 2 arguments (object, changes)");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_replace", "Invalid return value slot");

  struct Object* obj = object_table + args.index[0];
  struct Object* changes = object_table + args.index[1];
  obj = GetOriginData(obj);
  changes = GetOriginData(changes);

  if (obj->type[0] != 0x07)  // 假设0x07为可替换对象类型
    EXIT_VM("aqstl_replace", "Unsupported object type for replace");

  // 简化实现：创建新对象并替换指定字段
  struct Object* new_obj = (struct Object*)malloc(sizeof(struct Object));
  memcpy(new_obj, obj, sizeof(struct Object));
  // 实际应根据changes内容替换对应字段（需扩展changes解析逻辑）
  SetObjectData(return_value, new_obj);
}

/*void aqstl_tmpnam(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_tmpnam(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_tmpnam(InternalObject,size_t)", "Invalid return value.");
  SetStringData(return_value,tmpnam(GetStringData(*args.index)));
}*/
void aqstl_getchar(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_getchar(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_getchar(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value, getchar());
}
void aqstl_putchar(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_putchar(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_putchar(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value, putchar(GetLongData(*args.index)));
}
void aqstl_puts(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_puts(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_puts(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value, puts(GetStringData(*args.index)));
}
void aqstl_perror(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_perror(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_perror(InternalObject,size_t)", "Invalid return value.");
  perror(GetStringData(*args.index));
}
/*void aqstl_tmpnam_s(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_tmpnam_s(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_tmpnam_s(InternalObject,size_t)", "Invalid return value.");
  SetLongData(return_value,tmpnam_s(GetStringData(*args.index),GetUint64tData(*(args.index
+ 1))));
}
void aqstl_gets_s(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_gets_s(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_gets_s(InternalObject,size_t)", "Invalid return value.");
  SetStringData(return_value,gets_s(GetStringData(*args.index),GetUint64tData(*(args.index
+ 1))));
}*/

/*int remove(const char* filename);
int rename(const char* old, const char* new);
//FILE* tmpfile(void);
char* tmpnam(char* s);
//int fclose(FILE* stream);
//int fflush(FILE* stream);
//FILE* fopen(const char* restrict filename, const char* restrict mode);
//FILE* freopen(const char* restrict filename, const char* restrict mode,
//              FILE* restrict stream);
//void setbuf(FILE* restrict stream, char* restrict buf);
//int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size);
// int printf(const char* restrict format, ...);
// int scanf(const char* restrict format, ...);
// int snprintf(char* restrict s, size_t n, const char* restrict format, ...);
// int sprintf(char* restrict s, const char* restrict format, ...);
// int sscanf(const char* restrict s, const char* restrict format, ...);
// int vfprintf(FILE* restrict stream, const char* restrict format, va_list
// arg); int vfscanf(FILE* restrict stream, const char* restrict format, va_list
// arg); int vprintf(const char* restrict format, va_list arg); int vscanf(const
// char* restrict format, va_list arg); int vsnprintf(char* restrict s, size_t
// n, const char* restrict format, va_list arg); int vsprintf(char* restrict s,
// const char* restrict format, va_list arg); int vsscanf(const char* restrict
// s, const char* restrict format, va_list arg);
//int fgetc(FILE* stream);
//char* fgets(char* restrict s, int n, FILE* restrict stream);
//int fputc(int c, FILE* stream);
//int fputs(const char* restrict s, FILE* restrict stream);
//int getc(FILE* stream);
int getchar(void);
//int putc(int c, FILE* stream);
int putchar(int c);
int puts(const char* s);
//int ungetc(int c, FILE* stream);
//size_t fread(void* restrict ptr, size_t size, size_t nmemb,
//             FILE* restrict stream);
//size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb,
//              FILE* restrict stream);
//int fgetpos(FILE* restrict stream, fpos_t* restrict pos);
//int fseek(FILE* stream, long int offset, int whence);
//int fsetpos(FILE* stream, const fpos_t* pos);
//long int ftell(FILE* stream);
//void rewind(FILE* stream);
//void clearerr(FILE* stream);
//int feof(FILE* stream);
//int ferror(FILE* stream);
void perror(const char* s);
// int fprintf(FILE* restrict stream, const char* restrict format, ...);
// int fscanf(FILE* restrict stream, const char* restrict format, ...);
//errno_t tmpfile_s(FILE* restrict* restrict streamptr);
errno_t tmpnam_s(char* s, rsize_t maxsize);
//errno_t fopen_s(FILE* restrict* restrict streamptr,
//                const char* restrict filename, const char* restrict mode);
//errno_t freopen_s(FILE* restrict* restrict newstreamptr,
//                  const char* restrict filename, const char* restrict mode,
//                  FILE* restrict stream);
// int fprintf_s(FILE* restrict stream, const char* restrict format, ...);
// int fscanf_s(FILE* restrict stream, const char* restrict format, ...);
// int printf_s(const char* restrict format, ...);
// int scanf_s(const char* restrict format, ...);
// int snprintf_s(char* restrict s, rsize_t n, const char* restrict format,
// ...); int sprintf_s(char* restrict s, rsize_t n, const char* restrict format,
// ...); int sscanf_s(const char* restrict s, const char* restrict format, ...);
// int vfprintf_s(FILE* restrict stream, const char* restrict format, va_list
// arg); int vfscanf_s(FILE* restrict stream, const char* restrict format,
// va_list arg); int vprintf_s(const char* restrict format, va_list arg); int
// vscanf_s(const char* restrict format, va_list arg); int vsnprintf_s(char*
// restrict s, rsize_t n, const char* restrict format, va_list arg); int
// vsprintf_s(char* restrict s, rsize_t n, const char* restrict format, va_list
// arg); int vsscanf_s(const char* restrict s, const char* restrict format,
// va_list arg);
char* gets_s(char* s, rsize_t n);
*/

// String module constants
static const char* string_ascii_letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* string_ascii_lowercase = "abcdefghijklmnopqrstuvwxyz";
static const char* string_ascii_uppercase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* string_digits = "0123456789";
static const char* string_hexdigits = "0123456789abcdefABCDEF";
static const char* string_octdigits = "01234567";
static const char* string_punctuation = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const char* string_printable = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r\f\v";
static const char* string_whitespace = " \t\n\r\f\v";

void aqstl_string_ascii_letters(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_ascii_letters", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_ascii_letters", "Invalid return value.");
  SetStringData(return_value, string_ascii_letters);
}

void aqstl_string_ascii_lowercase(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_ascii_lowercase", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_ascii_lowercase", "Invalid return value.");
  SetStringData(return_value, string_ascii_lowercase);
}

void aqstl_string_ascii_uppercase(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_ascii_uppercase", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_ascii_uppercase", "Invalid return value.");
  SetStringData(return_value, string_ascii_uppercase);
}

void aqstl_string_digits(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_digits", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_digits", "Invalid return value.");
  SetStringData(return_value, string_digits);
}

void aqstl_string_hexdigits(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_hexdigits", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_hexdigits", "Invalid return value.");
  SetStringData(return_value, string_hexdigits);
}

void aqstl_isdigit(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1)
    EXIT_VM("aqstl_isdigit", "Invalid args, requires 1 argument");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_isdigit", "Invalid return value slot");
  
  struct Object* object = GetOriginData(object_table + *args.index);
  if (object == NULL || object->type[0] != 0x05)
    EXIT_VM("aqstl_isdigit", "Argument must be a string");
  
  const char* str = GetStringObjectData(object);
  for (size_t i = 0; str[i] != '\0'; i++) {
    if (!isdigit((unsigned char)str[i])) {
      SetLongData(return_value, 0);
      return;
    }
  }
  SetLongData(return_value, 1);
}

void aqstl_string_octdigits(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_octdigits", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_octdigits", "Invalid return value.");
  SetStringData(return_value, string_octdigits);
}

void aqstl_string_punctuation(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_punctuation", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_punctuation", "Invalid return value.");
  SetStringData(return_value, string_punctuation);
}

void aqstl_string_printable(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_printable", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_printable", "Invalid return value.");
  SetStringData(return_value, string_printable);
}

void aqstl_string_whitespace(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0)
    EXIT_VM("aqstl_string_whitespace", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_string_whitespace", "Invalid return value.");
  SetStringData(return_value, string_whitespace);
}



/*// 包含PCRE2头文件
#include <pcre2.h>

// 正则表达式模式结构体
struct Pattern {
    const char* pattern;
    int flags;
    pcre2_code* compiled; // 使用PCRE2编译后的正则表达式对象
    int error_code;       // 编译错误码
    PCRE2_SIZE error_offset; // 错误位置
};

// 匹配对象结构体（存储匹配位置、分组等信息）
struct Match {
    pcre2_match_data* match_data; // PCRE2匹配数据
    const char* subject;          // 被匹配的原始字符串
    size_t start;                 // 匹配起始位置
    size_t end;                   // 匹配结束位置
};

// 获取匹配的分组内容（对应Match.group()）
void aqstl_match_group(InternalObject args, size_t return_value) {

// 新增re.finditer函数实现
void aqstl_re_finditer(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_finditer", "需要2个参数（pattern对象, 待搜索字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_finditer", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_finditer", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待搜索字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_finditer", "待搜索内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    // 创建迭代器结构体存储匹配状态
    struct FindIter {
        pcre2_code* compiled;
        PCRE2_SPTR8 subject;
        size_t subject_length;
        pcre2_match_data* match_data;
        size_t start;
    };

    struct FindIter* iter = (struct FindIter*)malloc(sizeof(struct FindIter));
    iter->compiled = p->compiled;
    iter->subject = subject;
    iter->subject_length = subject_length;
    iter->match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    iter->start = 0;

    // 构造返回的迭代器对象（假设迭代器类型标识为0x0A）
    struct Object* result = object_table + return_value;
    result->type = (uint8_t*)malloc(2);
    result->type[0] = 0x0A;
    result->type[1] = 0x00;
    result->data.ptr_data = iter;
    result->const_type = false;
}

// 迭代器next方法实现（对应FindIter.__next__）
void aqstl_finditer_next(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_finditer_next", "需要1个参数（finditer迭代器对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_finditer_next", "无效的返回值位置");

    // 获取迭代器对象
    struct Object* iter_obj = object_table + args.index[0];
    iter_obj = GetOriginData(iter_obj);
    if (iter_obj->type[0] != 0x0A) EXIT_VM("aqstl_finditer_next", "参数必须是finditer迭代器对象");
    struct FindIter* iter = (struct FindIter*)iter_obj->data.ptr_data;

    // 执行下一次匹配
    int rc = pcre2_match_8(
        iter->compiled,
        iter->subject,
        iter->subject_length,
        iter->start,
        0,
        iter->match_data,
        NULL
    );

    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(iter->match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = iter->match_data;
        match->subject = (const char*)iter->subject;
        match->start = ovector[0];
        match->end = ovector[1];

        // 更新下一次匹配的起始位置
        iter->start = ovector[1];

        // 构造返回的Match对象
        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        // 无更多匹配，释放匹配数据并返回None（假设None类型标识为0x00）
        pcre2_match_data_free_8(iter->match_data);
        free(iter);
        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(1);
        result->type[0] = 0x00;
        result->const_type = true;
    }
}

// 新增re.escape函数实现
void aqstl_re_escape(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_re_escape", "需要1个参数（待转义字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_escape", "无效的返回值位置");

    // 获取待转义字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[0];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_escape", "输入必须是字符串类型");
    const char* input = GetStringObjectData(string_obj);

    // 预分配转义后字符串空间（假设最多每个字符转义一次）
    size_t input_len = strlen(input);
    char* escaped = (char*)malloc(input_len * 2 + 1);
    size_t pos = 0;

    // 定义需要转义的特殊字符集合
    const char* special_chars = ".^$*+?()[{\\|}";

    for (size_t i = 0; i < input_len; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            escaped[pos++] = '\\';
        }
        escaped[pos++] = input[i];
    }
    escaped[pos] = '\0';

    SetStringData(return_value, escaped);
    free(escaped);
}

// 获取匹配的分组内容（对应Match.group()）
void aqstl_match_group(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_match_group", "需要2个参数（match对象, 组号）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_group", "无效的返回值位置");

    // 获取match参数（Match类型）
    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_group", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    // 获取组号参数（整数类型）
    struct Object* group_obj = object_table + args.index[1];
    group_obj = GetOriginData(group_obj);
    if (group_obj->type[0] != 0x02) EXIT_VM("aqstl_match_group", "组号必须是整数类型");
    int group = (int)GetLongObjectData(group_obj);

    // 获取匹配的分组位置
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(m->match_data);
    if (group < 0 || group >= (int)pcre2_get_ovector_count_8(m->match_data)) {
        SetStringData(return_value, ""); // 无效组号返回空字符串
        return;
    }

    // 提取分组内容
    size_t start = ovector[2*group];
    size_t end = ovector[2*group + 1];
    size_t length = end - start;
    char* group_str = strndup(m->subject + start, length);
    SetStringData(return_value, group_str);
    free(group_str);
}

// 获取匹配的起始位置（对应Match.start()）
void aqstl_match_start(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_match_start", "需要1个参数（match对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_start", "无效的返回值位置");

    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_start", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    SetUint64tData(return_value, m->start);
}

// 获取匹配的结束位置（对应Match.end()）
void aqstl_match_end(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_match_end", "需要1个参数（match对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_end", "无效的返回值位置");

    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_end", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    SetUint64tData(return_value, m->end);
}

// 正则完全匹配函数（对应re.fullmatch）
void aqstl_re_fullmatch(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_fullmatch", "需要2个参数（pattern对象, 待匹配字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_fullmatch", "无效的返回值位置");

    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_fullmatch", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_fullmatch", "待匹配内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    if (match_data == NULL) EXIT_VM("aqstl_re_fullmatch", "无法创建匹配数据");

    int rc = pcre2_match_8(
        p->compiled,
        subject,
        subject_length,
        0,
        PCRE2_ANCHORED | PCRE2_ENDANCHORED,  // 强制匹配开头和结尾
        match_data,
        NULL
    );

    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = match_data;
        match->subject = target;
        match->start = ovector[0];
        match->end = ovector[1];

        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        pcre2_match_data_free_8(match_data);
        SetLongData(return_value, 0);
    }
}

// 转义特殊字符函数（对应re.escape）
void aqstl_re_escape(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_re_escape", "需要1个参数（待转义字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_escape", "无效的返回值位置");

    struct Object* string_obj = object_table + args.index[0];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_escape", "参数必须是字符串类型");
    const char* input = GetStringObjectData(string_obj);

    size_t input_len = strlen(input);
    char* output = (char*)malloc(input_len * 2 + 1); // 最多每个字符转义一次
    size_t output_len = 0;

    const char* special_chars = ".\\+*?[^]$(){}=!<>|:";
    for (size_t i = 0; i < input_len; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            output[output_len++] = '\\';
        }
        output[output_len++] = input[i];
    }
    output[output_len] = '\0';

    SetStringData(return_value, output);
    free(output);
}

// 编译正则表达式（对应re.compile）
void aqstl_re_compile(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_compile", "需要2个参数（pattern字符串, flags整数）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_compile", "无效的返回值位置");

    // 获取pattern参数（字符串类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x05) EXIT_VM("aqstl_re_compile", "pattern必须是字符串类型");
    const char* pattern = GetStringObjectData(pattern_obj);

    // 获取flags参数（整数类型）
    struct Object* flags_obj = object_table + args.index[1];
    flags_obj = GetOriginData(flags_obj);
    if (flags_obj->type[0] != 0x02) EXIT_VM("aqstl_re_compile", "flags必须是整数类型");
    int flags = (int)GetLongObjectData(flags_obj);

    // 转换为PCRE2标志
    uint32_t pcre2_flags = 0;
    if (flags & 1) pcre2_flags |= PCRE2_CASELESS;    // re.IGNORECASE
    if (flags & 2) pcre2_flags |= PCRE2_DOTALL;      // re.DOTALL
    if (flags & 4) pcre2_flags |= PCRE2_MULTILINE;   // re.MULTILINE
    if (flags & 8) pcre2_flags |= PCRE2_UTF;         // re.UNICODE
    if (flags & 16) pcre2_flags |= PCRE2_VERBOSE;    // re.VERBOSE
    if (flags & 32) pcre2_flags |= PCRE2_LOCALE;      // re.LOCALE
    if (flags & 256) pcre2_flags |= PCRE2_ASCII;      // re.ASCII
    if (flags & 128) pcre2_flags |= PCRE2_DEBUG;      // re.DEBUG

    // 使用PCRE2编译正则表达式
    PCRE2_SIZE error_offset;
    int error_code;
    pcre2_code* compiled = pcre2_compile_8(
        (PCRE2_SPTR8)pattern,
        PCRE2_ZERO_TERMINATED,
        pcre2_flags,
        &error_code,
        &error_offset,
        NULL
    );

    // 创建并初始化Pattern对象
    struct Pattern* p = (struct Pattern*)malloc(sizeof(struct Pattern));
    p->pattern = strdup(pattern);
    p->flags = flags;
    p->compiled = compiled;
    p->error_code = error_code;
    p->error_offset = error_offset;

    // 检查编译错误
    if (compiled == NULL) {
        char error_buf[256];
        pcre2_get_error_message_8(error_code, (PCRE2_UCHAR8*)error_buf, sizeof(error_buf));
        EXIT_VM("aqstl_re_compile", "正则表达式编译失败: %s (位置: %zu)", error_buf, error_offset);
    }

    struct Object* result = object_table + return_value;
    result->type = (uint8_t*)malloc(2);
    result->type[0] = 0x08; // 自定义类型标识
    result->type[1] = 0x00;
    result->data.ptr_data = p;
    result->const_type = false;
}

// 匹配对象结构体（存储匹配位置、分组等信息）
struct Match {
    pcre2_match_data* match_data; // PCRE2匹配数据
    const char* subject;          // 被匹配的原始字符串
    size_t start;                 // 匹配起始位置
    size_t end;                   // 匹配结束位置
};

// 获取匹配的分组内容（对应Match.group()）
void aqstl_match_group(InternalObject args, size_t return_value) {

// 新增re.finditer函数实现
void aqstl_re_finditer(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_finditer", "需要2个参数（pattern对象, 待搜索字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_finditer", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_finditer", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待搜索字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_finditer", "待搜索内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    // 创建迭代器结构体存储匹配状态
    struct FindIter {
        pcre2_code* compiled;
        PCRE2_SPTR8 subject;
        size_t subject_length;
        pcre2_match_data* match_data;
        size_t start;
    };

    struct FindIter* iter = (struct FindIter*)malloc(sizeof(struct FindIter));
    iter->compiled = p->compiled;
    iter->subject = subject;
    iter->subject_length = subject_length;
    iter->match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    iter->start = 0;

    // 构造返回的迭代器对象（假设迭代器类型标识为0x0A）
    struct Object* result = object_table + return_value;
    result->type = (uint8_t*)malloc(2);
    result->type[0] = 0x0A;
    result->type[1] = 0x00;
    result->data.ptr_data = iter;
    result->const_type = false;
}

// 迭代器next方法实现（对应FindIter.__next__）
void aqstl_finditer_next(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_finditer_next", "需要1个参数（finditer迭代器对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_finditer_next", "无效的返回值位置");

    // 获取迭代器对象
    struct Object* iter_obj = object_table + args.index[0];
    iter_obj = GetOriginData(iter_obj);
    if (iter_obj->type[0] != 0x0A) EXIT_VM("aqstl_finditer_next", "参数必须是finditer迭代器对象");
    struct FindIter* iter = (struct FindIter*)iter_obj->data.ptr_data;

    // 执行下一次匹配
    int rc = pcre2_match_8(
        iter->compiled,
        iter->subject,
        iter->subject_length,
        iter->start,
        0,
        iter->match_data,
        NULL
    );

    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(iter->match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = iter->match_data;
        match->subject = (const char*)iter->subject;
        match->start = ovector[0];
        match->end = ovector[1];

        // 更新下一次匹配的起始位置
        iter->start = ovector[1];

        // 构造返回的Match对象
        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        // 无更多匹配，释放匹配数据并返回None（假设None类型标识为0x00）
        pcre2_match_data_free_8(iter->match_data);
        free(iter);
        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(1);
        result->type[0] = 0x00;
        result->const_type = true;
    }
}

// 新增re.escape函数实现
void aqstl_re_escape(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_re_escape", "需要1个参数（待转义字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_escape", "无效的返回值位置");

    // 获取待转义字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[0];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_escape", "输入必须是字符串类型");
    const char* input = GetStringObjectData(string_obj);

    // 预分配转义后字符串空间（假设最多每个字符转义一次）
    size_t input_len = strlen(input);
    char* escaped = (char*)malloc(input_len * 2 + 1);
    size_t pos = 0;

    // 定义需要转义的特殊字符集合
    const char* special_chars = ".^$*+?()[{\\|}";

    for (size_t i = 0; i < input_len; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            escaped[pos++] = '\\';
        }
        escaped[pos++] = input[i];
    }
    escaped[pos] = '\0';

    SetStringData(return_value, escaped);
    free(escaped);
}

// 获取匹配的分组内容（对应Match.group()）
void aqstl_match_group(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_match_group", "需要2个参数（match对象, 组号）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_group", "无效的返回值位置");

    // 获取match参数（Match类型）
    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_group", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    // 获取组号参数（整数类型）
    struct Object* group_obj = object_table + args.index[1];
    group_obj = GetOriginData(group_obj);
    if (group_obj->type[0] != 0x02) EXIT_VM("aqstl_match_group", "组号必须是整数类型");
    int group = (int)GetLongObjectData(group_obj);

    // 获取匹配的分组位置
    PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(m->match_data);
    if (group < 0 || group >= (int)pcre2_get_ovector_count_8(m->match_data)) {
        SetStringData(return_value, ""); // 无效组号返回空字符串
        return;
    }

    // 提取分组内容
    size_t start = ovector[2*group];
    size_t end = ovector[2*group + 1];
    size_t length = end - start;
    char* group_str = strndup(m->subject + start, length);
    SetStringData(return_value, group_str);
    free(group_str);
}

// 获取匹配的起始位置（对应Match.start()）
void aqstl_match_start(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_match_start", "需要1个参数（match对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_start", "无效的返回值位置");

    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_start", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    SetUint64tData(return_value, m->start);
}

// 获取匹配的结束位置（对应Match.end()）
void aqstl_match_end(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_match_end", "需要1个参数（match对象）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_match_end", "无效的返回值位置");

    struct Object* match_obj = object_table + args.index[0];
    match_obj = GetOriginData(match_obj);
    if (match_obj->type[0] != 0x09) EXIT_VM("aqstl_match_end", "参数必须是Match对象");
    struct Match* m = (struct Match*)match_obj->data.ptr_data;

    SetUint64tData(return_value, m->end);
}

// 正则完全匹配函数（对应re.fullmatch）
void aqstl_re_fullmatch(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_fullmatch", "需要2个参数（pattern对象, 待匹配字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_fullmatch", "无效的返回值位置");

    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_fullmatch", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_fullmatch", "待匹配内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    if (match_data == NULL) EXIT_VM("aqstl_re_fullmatch", "无法创建匹配数据");

    int rc = pcre2_match_8(
        p->compiled,
        subject,
        subject_length,
        0,
        PCRE2_ANCHORED | PCRE2_ENDANCHORED,  // 强制匹配开头和结尾
        match_data,
        NULL
    );

    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = match_data;
        match->subject = target;
        match->start = ovector[0];
        match->end = ovector[1];

        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        pcre2_match_data_free_8(match_data);
        SetLongData(return_value, 0);
    }
}

// 转义特殊字符函数（对应re.escape）
void aqstl_re_escape(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_re_escape", "需要1个参数（待转义字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_escape", "无效的返回值位置");

    struct Object* string_obj = object_table + args.index[0];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_escape", "参数必须是字符串类型");
    const char* input = GetStringObjectData(string_obj);

    size_t input_len = strlen(input);
    char* output = (char*)malloc(input_len * 2 + 1); // 最多每个字符转义一次
    size_t output_len = 0;

    const char* special_chars = ".\\+*?[^]$(){}=!<>|:";
    for (size_t i = 0; i < input_len; i++) {
        if (strchr(special_chars, input[i]) != NULL) {
            output[output_len++] = '\\';
        }
        output[output_len++] = input[i];
    }
    output[output_len] = '\0';

    SetStringData(return_value, output);
    free(output);
}

// 正则匹配函数（对应re.match）
void aqstl_re_match(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_match", "需要2个参数（pattern对象, 待匹配字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_match", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_match", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待匹配字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_match", "待匹配内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    // 创建PCRE2匹配数据
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    if (match_data == NULL) EXIT_VM("aqstl_re_match", "无法创建匹配数据");

    // 执行匹配（仅匹配字符串开头）
    int rc = pcre2_match_8(
        p->compiled,
        subject,
        subject_length,
        0,               // 起始位置
        PCRE2_ANCHORED,  // 强制匹配开头
        match_data,
        NULL
    );

    // 构造返回的匹配对象
    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = match_data;
        match->subject = target;
        match->start = ovector[0];
        match->end = ovector[1];

        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        pcre2_match_data_free_8(match_data);
        SetLongData(return_value, 0); // 无匹配返回0
    }
}

// 正则搜索函数（对应re.search）
void aqstl_re_search(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_search", "需要2个参数（pattern对象, 待搜索字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_search", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_search", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待搜索字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_search", "待搜索内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);
    PCRE2_SPTR8 subject = (PCRE2_SPTR8)target;
    size_t subject_length = strlen(target);

    // 创建PCRE2匹配数据
    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern_8(p->compiled, NULL);
    if (match_data == NULL) EXIT_VM("aqstl_re_search", "无法创建匹配数据");

    // 执行搜索（任意位置匹配）
    int rc = pcre2_match_8(
        p->compiled,
        subject,
        subject_length,
        0,               // 起始位置
        0,               // 无额外标志
        match_data,
        NULL
    );

    // 构造返回的匹配对象
    if (rc >= 0) {
        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer_8(match_data);
        struct Match* match = (struct Match*)malloc(sizeof(struct Match));
        match->match_data = match_data;
        match->subject = target;
        match->start = ovector[0];
        match->end = ovector[1];

        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = match;
        result->const_type = false;
    } else {
        pcre2_match_data_free_8(match_data);
        SetLongData(return_value, 0); // 无匹配返回0
    }
}
}

// 正则搜索函数（对应re.search）
void aqstl_re_search(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_search", "需要2个参数（pattern对象, 待搜索字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_search", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_search", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待搜索字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_search", "待搜索内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);

    // 示例：任意位置匹配逻辑（实际需调用正则引擎实现）
    bool found = (strstr(target, p->pattern) != NULL); // 仅示例，实际应使用编译后的compiled数据搜索

    // 构造返回的匹配对象（假设0x09为Match类型标识）
    if (found) {
        struct Object* result = object_table + return_value;
        result->type = (uint8_t*)malloc(2);
        result->type[0] = 0x09;
        result->type[1] = 0x00;
        result->data.ptr_data = NULL; // 实际应填充匹配位置、分组等信息
        result->const_type = false;
    } else {
        SetLongData(return_value, 0); // 无匹配返回0（可根据需要调整返回类型）
    }
}

// 正则替换函数（对应re.sub）
void aqstl_re_sub(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 4) EXIT_VM("aqstl_re_sub", "需要4个参数（pattern对象, 替换字符串, 待替换字符串, 替换次数）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_sub", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_sub", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取替换字符串参数（字符串类型）
    struct Object* repl_obj = object_table + args.index[1];
    repl_obj = GetOriginData(repl_obj);
    if (repl_obj->type[0] != 0x05) EXIT_VM("aqstl_re_sub", "替换内容必须是字符串类型");
    const char* repl = GetStringObjectData(repl_obj);

    // 获取待替换字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[2];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_sub", "待替换内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);

    // 获取替换次数参数（整数类型）
    struct Object* count_obj = object_table + args.index[3];
    count_obj = GetOriginData(count_obj);
    if (count_obj->type[0] != 0x02) EXIT_VM("aqstl_re_sub", "替换次数必须是整数类型");
    int count = (int)GetLongObjectData(count_obj);

    // 示例：简单替换逻辑（实际需调用正则引擎实现）
    char* result_str = strdup(target); // 复制原始字符串
    char* pos = strstr(result_str, p->pattern);
    int replaced = 0;
    while (pos && (count == 0 || replaced < count)) {
        size_t pattern_len = strlen(p->pattern);
        size_t repl_len = strlen(repl);
        memmove(pos + repl_len, pos + pattern_len, strlen(pos + pattern_len) + 1);
        memcpy(pos, repl, repl_len);
        pos = strstr(pos + repl_len, p->pattern);
        replaced++;
    }

    // 返回替换后的字符串
    SetStringData(return_value, result_str);
    free(result_str); // 避免内存泄漏
}

// 正则分割函数（对应re.split）
void aqstl_re_split(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 3) EXIT_VM("aqstl_re_split", "需要3个参数（pattern对象, 待分割字符串, 最大分割次数）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_split", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_split", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待分割字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_split", "待分割内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);

    // 获取最大分割次数参数（整数类型）
    struct Object* maxsplit_obj = object_table + args.index[2];
    maxsplit_obj = GetOriginData(maxsplit_obj);
    if (maxsplit_obj->type[0] != 0x02) EXIT_VM("aqstl_re_split", "最大分割次数必须是整数类型");
    int maxsplit = (int)GetLongObjectData(maxsplit_obj);

    // 示例：简单分割逻辑（实际需调用PCRE2实现）
    char* str = strdup(target);
    char* token = strtok(str, p->pattern);
    int count = 0;
    struct Object* result_list = object_table + return_value;
    result_list->type = (uint8_t*)malloc(2);
    result_list->type[0] = 0x0A; // 列表类型标识
    result_list->type[1] = 0x00;
    result_list->data.ptr_data = malloc(10 * sizeof(struct Object*)); // 初始分配10个元素空间
    ((struct Object**)result_list->data.ptr_data)[count++] = CreateStringObject(token);

    while (token != NULL && (maxsplit == 0 || count < maxsplit)) {
        token = strtok(NULL, p->pattern);
        if (token != NULL) {
            if (count >= 10) {
                result_list->data.ptr_data = realloc(result_list->data.ptr_data, (count + 10) * sizeof(struct Object*));
            }
            ((struct Object**)result_list->data.ptr_data)[count++] = CreateStringObject(token);
        }
    }
    free(str);
    SetUint64tData(return_value + 1, count); // 假设列表长度存储在return_value+1位置
}

// 正则查找所有匹配函数（对应re.findall）
void aqstl_re_findall(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_re_findall", "需要2个参数（pattern对象, 待查找字符串）");
    if (return_value >= object_table_size) EXIT_VM("aqstl_re_findall", "无效的返回值位置");

    // 获取pattern参数（Pattern类型）
    struct Object* pattern_obj = object_table + args.index[0];
    pattern_obj = GetOriginData(pattern_obj);
    if (pattern_obj->type[0] != 0x08) EXIT_VM("aqstl_re_findall", "pattern必须是编译后的Pattern对象");
    struct Pattern* p = (struct Pattern*)pattern_obj->data.ptr_data;

    // 获取待查找字符串参数（字符串类型）
    struct Object* string_obj = object_table + args.index[1];
    string_obj = GetOriginData(string_obj);
    if (string_obj->type[0] != 0x05) EXIT_VM("aqstl_re_findall", "待查找内容必须是字符串类型");
    const char* target = GetStringObjectData(string_obj);

    // 示例：简单查找逻辑（实际需调用PCRE2实现）
    char* str = strdup(target);
    char* pos = strstr(str, p->pattern);
    int count = 0;
    struct Object* result_list = object_table + return_value;
    result_list->type = (uint8_t*)malloc(2);
    result_list->type[0] = 0x0A; // 列表类型标识
    result_list->type[1] = 0x00;
    result_list->data.ptr_data = malloc(10 * sizeof(struct Object*)); // 初始分配10个元素空间

    while (pos != NULL) {
        size_t match_len = strlen(p->pattern);
        char* match = strndup(pos, match_len);
        if (count >= 10) {
            result_list->data.ptr_data = realloc(result_list->data.ptr_data, (count + 10) * sizeof(struct Object*));
        }
        ((struct Object**)result_list->data.ptr_data)[count++] = CreateStringObject(match);
        free(match);
        pos = strstr(pos + match_len, p->pattern);
    }
    free(str);
    SetUint64tData(return_value + 1, count); // 假设列表长度存储在return_value+1位置
}
*/

void aqstl_abs(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_abs", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_abs", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_abs", "Null object.");

  switch (obj->type[0]) {
    case 0x01:  // Byte
      SetLongData(return_value, abs(GetByteObjectData(obj)));
      break;
    case 0x02:  // Long
      SetLongData(return_value, llabs(GetLongObjectData(obj)));
      break;
    case 0x03:  // Double
      SetDoubleData(return_value, fabs(GetDoubleObjectData(obj)));
      break;
    case 0x04:  // Uint64t
      SetUint64tData(return_value, GetUint64tObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_abs", "Unsupported type for absolute value.");
  }
}

void aqstl_ascii(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_ascii", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_ascii", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_ascii", "Null object.");

  char buffer[64];
  switch (obj->type[0]) {
    case 0x05:  // String
      snprintf(buffer, sizeof(buffer), "%s", GetStringObjectData(obj));
      break;
    default:
      snprintf(buffer, sizeof(buffer), "%p", GetPtrObjectData(obj));
      break;
  }
  SetStringData(return_value, buffer);
}

void aqstl_bin(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_bin", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_bin", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_bin", "Null object.");

  char buffer[64];
  switch (obj->type[0]) {
    case 0x02:  // Long
      snprintf(buffer, sizeof(buffer), "0b%lld", GetLongObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_bin", "Unsupported type for binary conversion.");
  }
  SetStringData(return_value, buffer);
}

void aqstl_bool(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_bool", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_bool", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_bool", "Null object.");

  int value = 0;
  switch (obj->type[0]) {
    case 0x01:
      value = !!GetByteObjectData(obj);
      break;
    case 0x02:
      value = !!GetLongObjectData(obj);
      break;
    case 0x03:
      value = !!GetDoubleObjectData(obj);
      break;
    default:
      value = 1;
      break;  // Non-null objects are truthy
  }
  SetByteData(return_value, value);
}

void aqstl_chr(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_chr", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_chr", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_chr", "Null object.");

  char buffer[2];
  switch (obj->type[0]) {
    case 0x02:  // Long
      snprintf(buffer, sizeof(buffer), "%c", (char)GetLongObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_chr", "Unsupported type for character conversion.");
  }
  SetStringData(return_value, buffer);
}

void aqstl_float(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_float", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_float", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_float", "Null object.");

  double value = 0.0;
  switch (obj->type[0]) {
    case 0x01:
      value = GetByteObjectData(obj);
      break;
    case 0x02:
      value = GetLongObjectData(obj);
      break;
    case 0x05:
      value = atof(GetStringObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_float", "Unsupported type for float conversion.");
  }
  SetDoubleData(return_value, value);
}

#define FNV_OFFSET_BASIS 0xCBF29CE484222325ULL
#define FNV_PRIME 0x100000001B3ULL

static uint64_t fnv_seed = 0;

void fnv_srand() { fnv_seed = (uint64_t)time(NULL) ^ (uint64_t)rand(); }

// FNV-1a 哈希计算（带随机种子）
uint64_t fnv1a_hash(const void* data, size_t length) {
  if (fnv_seed == 0) {
    srand((unsigned int)time(NULL));
    fnv_srand();
  }
  const uint8_t* bytes = (const uint8_t*)data;
  uint64_t hash = FNV_OFFSET_BASIS ^ fnv_seed;

  for (size_t i = 0; i < length; i++) {
    hash ^= bytes[i];
    hash *= FNV_PRIME;
    // hash &= 0xFFFFFFFFFFFFFFFFULL;
  }

  return hash;
}

void aqstl_hash(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_hash", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_hash", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_hash", "Null object.");

  uint64_t hash = 0;
  switch (obj->type[0]) {
    case 0x01:
      hash = GetByteObjectData(obj);
      break;
    case 0x02:
      hash = GetLongObjectData(obj);
      break;
    case 0x03:
      hash = (uint64_t)GetDoubleObjectData(obj);
      break;
    case 0x04:
      hash = GetUint64tObjectData(obj);
      break;
    case 0x05:
      hash = fnv1a_hash(GetStringObjectData(obj),
                        strlen(GetStringObjectData(obj)));
      break;
    default:
      EXIT_VM("aqstl_hash", "Unsupported type for hashing.");
  }

  if (obj->type[0] != 0x05) hash = fnv1a_hash(&hash, sizeof(hash));
  SetUint64tData(return_value, hash);
}

void aqstl_int(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_int", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_int", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_int", "Null object.");

  long value = 0;
  switch (obj->type[0]) {
    case 0x01:
      value = GetByteObjectData(obj);
      break;
    case 0x02:
      value = GetLongObjectData(obj);
      break;
    case 0x03:
      value = (long)GetDoubleObjectData(obj);
      break;
    case 0x05:
      value = strtol(GetStringObjectData(obj), NULL, 10);
      break;
    default:
      EXIT_VM("aqstl_int", "Unsupported type for integer conversion.");
  }
  SetLongData(return_value, value);
}

void aqstl_len(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_len", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_len", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_len", "Null object.");

  size_t length = 0;
  switch (obj->type[0]) {
    case 0x05:
      length = strlen(GetStringObjectData(obj));
      break;
    // case 0x06: length = GetUint64tObjectData(obj->data.ptr_data); break;
    default:
      EXIT_VM("aqstl_len", "Unsupported type for length.");
  }
  SetLongData(return_value, length);
}

void aqstl_max(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1) EXIT_VM("aqstl_max", "Requires at least 1 argument.");

  long current_max = LONG_MIN;
  for (size_t i = 0; i < args.size; i++) {
    struct Object* obj = object_table + args.index[i];
    obj = GetOriginData(obj);
    if (!obj) EXIT_VM("aqstl_max", "Null object in arguments.");

    long value = 0;
    switch (obj->type[0]) {
      case 0x01:
        value = GetByteObjectData(obj);
        break;
      case 0x02:
        value = GetLongObjectData(obj);
        break;
      default:
        EXIT_VM("aqstl_max", "Unsupported type for max.");
    }
    if (value > current_max) current_max = value;
  }
  SetLongData(return_value, current_max);
}

void aqstl_min(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1) EXIT_VM("aqstl_min", "Requires at least 1 argument.");

  long current_min = LONG_MAX;
  for (size_t i = 0; i < args.size; i++) {
    struct Object* obj = object_table + args.index[i];
    obj = GetOriginData(obj);
    if (!obj) EXIT_VM("aqstl_min", "Null object in arguments.");

    long value = 0;
    switch (obj->type[0]) {
      case 0x01:
        value = GetByteObjectData(obj);
        break;
      case 0x02:
        value = GetLongObjectData(obj);
        break;
      default:
        EXIT_VM("aqstl_min", "Unsupported type for min.");
    }
    if (value < current_min) current_min = value;
  }
  SetLongData(return_value, current_min);
}

void aqstl_oct(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_oct", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_oct", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_oct", "Null object.");

  char buffer[64];
  switch (obj->type[0]) {
    case 0x02:  // Long
      snprintf(buffer, sizeof(buffer), "0o%llo", GetLongObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_oct", "Unsupported type for octal conversion.");
  }
  SetStringData(return_value, buffer);
}

void aqstl_ord(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_ord", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_ord", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_ord", "Null object.");

  if (obj->type[0] != 0x05)  // String
    EXIT_VM("aqstl_ord", "Expected string argument.");

  const char* str = GetStringObjectData(obj);
  if (strlen(str) != 1)
    EXIT_VM("aqstl_ord", "String must have exactly one character.");

  SetByteData(return_value, (unsigned char)str[0]);
}

void aqstl_pow(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2) EXIT_VM("aqstl_pow", "Requires exactly 2 arguments.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_pow", "Invalid return value slot.");

  struct Object* base_obj = object_table + args.index[0];
  struct Object* exp_obj = object_table + args.index[1];
  base_obj = GetOriginData(base_obj);
  exp_obj = GetOriginData(exp_obj);
  if (!base_obj || !exp_obj) EXIT_VM("aqstl_pow", "Null object in arguments.");

  double result =
      pow(GetDoubleObjectData(base_obj), GetDoubleObjectData(exp_obj));
  SetDoubleData(return_value, result);
}

void aqstl_round(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_round", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_round", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_round", "Null object.");

  double value = GetDoubleObjectData(obj);
  long rounded = lround(value);
  SetLongData(return_value, rounded);
}

void aqstl_str(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_str", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_str", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_str", "Null object.");

  char* buffer = (char*)malloc(64 * sizeof(char));
  switch (obj->type[0]) {
    case 0x01:
      snprintf(buffer, sizeof(buffer), "%d", GetByteObjectData(obj));
      break;
    case 0x02:
      snprintf(buffer, sizeof(buffer), "%lld", GetLongObjectData(obj));
      break;
    case 0x03:
      snprintf(buffer, sizeof(buffer), "%.15f", GetDoubleObjectData(obj));
      break;
    case 0x04:
      snprintf(buffer, sizeof(buffer), "%zu", GetUint64tObjectData(obj));
      break;
    case 0x05:
      free(buffer);
      SetStringData(return_value, GetStringObjectData(obj));
      return;
    default:
      snprintf(buffer, sizeof(buffer), "<Object %p>", obj);
      break;
  }
  SetStringData(return_value, buffer);
}

void aqstl_math_factorial(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_factorial", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_factorial", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x02) EXIT_VM("aqstl_math_factorial", "Argument must be integer.");
  int64_t n = GetLongObjectData(obj);
  if (n < 0) EXIT_VM("aqstl_math_factorial", "Factorial of negative number is undefined.");

  int64_t result = 1;
  for (int64_t i = 2; i <= n; i++) result *= i;
  SetLongData(return_value, result);
}
int64_t compute_gcd(int64_t a, int64_t b) {
  while (b != 0) {
    int64_t temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

void aqstl_math_gcd(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 2) EXIT_VM("aqstl_math_gcd", "Requires at least 2 arguments.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_gcd", "Invalid return value slot.");



  int64_t gcd_val = 0;
  for (size_t i = 0; i < args.size; i++) {
    struct Object* obj = GetOriginData(object_table + args.index[i]);
    if (obj->type[0] != 0x02 && obj->type[0] != 0x04) {
      EXIT_VM("aqstl_math_gcd", "All arguments must be integers (type 0x02 or 0x04).");
    }
    int64_t num;
    if (obj->type[0] == 0x02) {
      num = llabs(GetLongObjectData(obj));
    } else { // 0x04 (uint64_t)
      num = llabs((int64_t)GetUint64tObjectData(obj));
    }
    gcd_val = (gcd_val == 0) ? num : compute_gcd(gcd_val, num);
  }
  SetLongData(return_value, gcd_val);
}

void aqstl_sum(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1) EXIT_VM("aqstl_sum", "Requires at least 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_sum", "Invalid return value slot.");

  long total = 0;
  for (size_t i = 0; i < args.size; i++) {
    struct Object* obj = object_table + args.index[i];
    obj = GetOriginData(obj);
    if (!obj) EXIT_VM("aqstl_sum", "Null object in arguments.");

    switch (obj->type[0]) {
      case 0x01:
        total += GetByteObjectData(obj);
        break;
      case 0x02:
        total += GetLongObjectData(obj);
        break;
      default:
        EXIT_VM("aqstl_sum", "Unsupported type for summation.");
    }
  }
  SetLongData(return_value, total);
}

// 文件比较缓存结构体
typedef struct {
  const char* path;
  time_t mtime;
  size_t size;
} FileCacheEntry;

static FileCacheEntry* file_cache = NULL;
static size_t file_cache_size = 0;

// 清除文件比较缓存（对应Python filecmp.clear_cache）
void aqstl_filecmp_clear_cache(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 0) EXIT_VM("aqstl_filecmp_clear_cache", "不需要参数");
  if (return_value >= object_table_size) EXIT_VM("aqstl_filecmp_clear_cache", "无效的返回值槽位");

  free(file_cache);
  file_cache = NULL;
  file_cache_size = 0;
  SetLongData(return_value, 0);
}

// 文件比较核心函数（对应Python filecmp.cmp）
void aqstl_filecmp_cmp(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 2 || args.size > 3) EXIT_VM("aqstl_filecmp_cmp", "需要2-3个参数（f1, f2, shallow=True）");
  if (return_value >= object_table_size) EXIT_VM("aqstl_filecmp_cmp", "无效的返回值槽位");

  // 参数校验：确保f1和f2是字符串类型
  struct Object* f1_obj = GetOriginData(object_table + args.index[0]);
  struct Object* f2_obj = GetOriginData(object_table + args.index[1]);
  if (f1_obj->type[0] != 0x05 || f2_obj->type[0] != 0x05) {
    EXIT_VM("aqstl_filecmp_cmp", "文件路径参数必须为字符串类型");
  }
  const char* f1 = GetStringObjectData(f1_obj);
  const char* f2 = GetStringObjectData(f2_obj);
  bool shallow = (args.size >= 3) ? (bool)GetLongObjectData(object_table + args.index[2]) : true;

  int64_t result = 0;

  // 检查缓存（shallow模式有效）
  if (shallow) {
    for (size_t i = 0; i < file_cache_size; i++) {
      if (strcmp(file_cache[i].path, f1) == 0) {
        time_t f1_mtime = file_cache[i].mtime;
        size_t f1_size = file_cache[i].size;
        for (size_t j = 0; j < file_cache_size; j++) {
          if (strcmp(file_cache[j].path, f2) == 0) {
            result = (f1_mtime == file_cache[j].mtime) && (f1_size == file_cache[j].size);
            SetLongData(return_value, result);
            return;
          }
        }
      }
    }
  }

  // 获取文件元数据（跨平台实现）
  #ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA f1_attr, f2_attr;
    if (!GetFileAttributesExA(f1, GetFileExInfoStandard, &f1_attr) || !GetFileAttributesExA(f2, GetFileExInfoStandard, &f2_attr)) {
      EXIT_VM("aqstl_filecmp_cmp", "无法获取文件属性");
    }
    time_t f1_mtime = FileTimeToUnixTime(&f1_attr.ftLastWriteTime);
    time_t f2_mtime = FileTimeToUnixTime(&f2_attr.ftLastWriteTime);
    size_t f1_size = (f1_attr.nFileSizeHigh << 32) | f1_attr.nFileSizeLow;
    size_t f2_size = (f2_attr.nFileSizeHigh << 32) | f2_attr.nFileSizeLow;
  #else
    struct stat f1_stat, f2_stat;
    if (stat(f1, &f1_stat) != 0 || stat(f2, &f2_stat) != 0) {
      EXIT_VM("aqstl_filecmp_cmp", "无法获取文件状态");
    }
    time_t f1_mtime = f1_stat.st_mtime;
    time_t f2_mtime = f2_stat.st_mtime;
    size_t f1_size = f1_stat.st_size;
    size_t f2_size = f2_stat.st_size;
  #endif

  // shallow模式比较元数据
  if (shallow) {
    result = (f1_mtime == f2_mtime) && (f1_size == f2_size);
    // 更新缓存
    FileCacheEntry* new_cache = (FileCacheEntry*)realloc(file_cache, (file_cache_size + 2) * sizeof(FileCacheEntry));
    if (new_cache) {
      file_cache = new_cache;
      file_cache[file_cache_size++] = (FileCacheEntry){f1, f1_mtime, f1_size};
      file_cache[file_cache_size++] = (FileCacheEntry){f2, f2_mtime, f2_size};
    }
  } else {
    // 内容比较逻辑（逐块读取比较）
    FILE* f1_fp = fopen(f1, "rb");
    FILE* f2_fp = fopen(f2, "rb");
    if (!f1_fp || !f2_fp) {
      if (f1_fp) fclose(f1_fp);
      if (f2_fp) fclose(f2_fp);
      EXIT_VM("aqstl_filecmp_cmp", "无法打开文件进行内容比较");
    }

    char buffer1[4096], buffer2[4096];
    size_t bytes_read1, bytes_read2;
    result = true;
    do {
      bytes_read1 = fread(buffer1, 1, sizeof(buffer1), f1_fp);
      bytes_read2 = fread(buffer2, 1, sizeof(buffer2), f2_fp);
      if (bytes_read1 != bytes_read2 || memcmp(buffer1, buffer2, bytes_read1) != 0) {
        result = false;
        break;
      }
    } while (bytes_read1 == sizeof(buffer1) && bytes_read2 == sizeof(buffer2));

    fclose(f1_fp);
    fclose(f2_fp);
  }
  SetLongData(return_value, (int64_t)result);
}

// 多文件比较函数（对应Python filecmp.cmpfiles）
void aqstl_filecmp_cmpfiles(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 3 || args.size > 4) EXIT_VM("aqstl_filecmp_cmpfiles", "需要3-4个参数（dir1, dir2, common, shallow=True）");
  if (return_value >= object_table_size) EXIT_VM("aqstl_filecmp_cmpfiles", "无效的返回值槽位");

  // 参数校验：确保dir1、dir2是字符串类型，common是数组类型
  struct Object* dir1_obj = GetOriginData(object_table + args.index[0]);
  struct Object* dir2_obj = GetOriginData(object_table + args.index[1]);
  struct Object* common_obj = GetOriginData(object_table + args.index[2]);
  if (dir1_obj->type[0] != 0x05 || dir2_obj->type[0] != 0x05 || common_obj->type[0] != 0x0A) {
    EXIT_VM("aqstl_filecmp_cmpfiles", "参数类型错误（dir1/dir2应为字符串，common应为数组）");
  }
  const char* dir1 = GetStringObjectData(dir1_obj);
  const char* dir2 = GetStringObjectData(dir2_obj);
  bool shallow = (args.size >= 4) ? (bool)GetLongObjectData(object_table + args.index[3]) : true;

  // 实际比较逻辑：遍历common列表比较文件
  size_t common_count = GetArraySize(common_obj);
  for (size_t i = 0; i < common_count; i++) {
    struct Object* file_obj = GetArrayElement(common_obj, i);
    if (file_obj->type[0] != 0x05) EXIT_VM("aqstl_filecmp_cmpfiles", "common数组元素应为字符串");
    const char* file = GetStringObjectData(file_obj);

    // 构造完整路径
    char f1_path[MAX_PATH], f2_path[MAX_PATH];
    snprintf(f1_path, sizeof(f1_path), "%s/%s", dir1, file);
    snprintf(f2_path, sizeof(f2_path), "%s/%s", dir2, file);

    // 调用文件比较函数
    InternalObject cmp_args = {.size = 3, .index = malloc(3 * sizeof(size_t))};
    cmp_args.index[0] = CreateStringObject(f1_path);
    cmp_args.index[1] = CreateStringObject(f2_path);
    cmp_args.index[2] = CreateLongObject((int64_t)shallow);
    size_t cmp_result_slot = AllocateObjectSlot();
    aqstl_filecmp_cmp(cmp_args, cmp_result_slot);
    bool is_equal = (bool)GetLongObjectData(object_table + cmp_result_slot);

    // TODO: 收集比较结果到返回值（需要定义结果结构）
    free(cmp_args.index);
  }
  SetLongData(return_value, 0);
}

// 目录比较类结构体（对应Python dircmp）
typedef struct {
  const char* left;
  const char* right;
  char** common;
  char** left_only;
  char** right_only;
  char** common_dirs;
  char** common_files;
  char** common_funny;
} DirCmp;

// 初始化目录比较对象（内部函数）
static DirCmp* dircmp_init(const char* left, const char* right) {
  DirCmp* dc = (DirCmp*)malloc(sizeof(DirCmp));
  dc->left = strdup(left);
  dc->right = strdup(right);

  // TODO: 实现目录内容收集逻辑（跨平台获取文件/目录列表）
  // 示例：使用POSIX opendir/readdir（Windows需要用FindFirstFile/FindNextFile）
  #ifdef _WIN32
    // Windows目录遍历实现
  #else
    DIR* dir = opendir(left);
    if (!dir) EXIT_VM("dircmp_init", "无法打开左目录");
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
      // 过滤.和..
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
      // 分类文件/目录
    }
    closedir(dir);
  #endif

  return dc;
}

// 目录比较类接口函数（对应Python dircmp）
void aqstl_filecmp_dircmp(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 2 || args.size > 3) EXIT_VM("aqstl_filecmp_dircmp", "需要2-3个参数（a, b, ignore=None）");
  if (return_value >= object_table_size) EXIT_VM("aqstl_filecmp_dircmp", "无效的返回值槽位");

  // 参数校验
  struct Object* a_obj = GetOriginData(object_table + args.index[0]);
  struct Object* b_obj = GetOriginData(object_table + args.index[1]);
  if (a_obj->type[0] != 0x05 || b_obj->type[0] != 0x05) EXIT_VM("aqstl_filecmp_dircmp", "目录路径参数必须为字符串类型");
  const char* a = GetStringObjectData(a_obj);
  const char* b = GetStringObjectData(b_obj);

  // 初始化目录比较对象
  DirCmp* dc = dircmp_init(a, b);
  // TODO: 实现差异比较逻辑并返回结果对象
  SetLongData(return_value, 0);
}

// 示例三角函数实现
void aqstl_math_sin(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_sin", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_sin", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_sin", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, sin(x));
}

// 示例浮点运算实现
void aqstl_math_ceil(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_ceil", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_ceil", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_ceil", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, ceil(x));
}

// 组合数计算
void aqstl_math_comb(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2) EXIT_VM("aqstl_math_comb", "Requires 2 arguments (n, k).");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_comb", "Invalid return value slot.");

  struct Object* n_obj = GetOriginData(object_table + args.index[0]);
  struct Object* k_obj = GetOriginData(object_table + args.index[1]);
  if (n_obj->type[0] != 0x02 || k_obj->type[0] != 0x02) EXIT_VM("aqstl_math_comb", "Arguments must be integers.");
  int64_t n = GetLongObjectData(n_obj);
  int64_t k = GetLongObjectData(k_obj);
  if (n < 0 || k < 0 || k > n) EXIT_VM("aqstl_math_comb", "Invalid combination parameters.");

  if (k > n - k) k = n - k;
  int64_t result = 1;
  for (int64_t i = 1; i <= k; i++) {
    result *= n - k + i;
    result /= i;
  }
  SetLongData(return_value, result);
}

// 最小公倍数计算
void aqstl_math_lcm(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 2) EXIT_VM("aqstl_math_lcm", "Requires at least 2 arguments.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_lcm", "Invalid return value slot.");

  int64_t lcm_val = 1;
  for (size_t i = 0; i < args.size; i++) {
    struct Object* obj = GetOriginData(object_table + args.index[i]);
    if (obj->type[0] != 0x02) EXIT_VM("aqstl_math_lcm", "All arguments must be integers.");
    int64_t num = llabs(GetLongObjectData(obj));
    if (num == 0) {
      SetLongData(return_value, 0);
      return;
    }
    lcm_val = (lcm_val / compute_gcd(lcm_val, num)) * num;
  }
  SetLongData(return_value, lcm_val);
}

// 平方根计算
void aqstl_math_sqrt(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_sqrt", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_sqrt", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_sqrt", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  if (x < 0) EXIT_VM("aqstl_math_sqrt", "Square root of negative number is undefined.");
  SetDoubleData(return_value, sqrt(x));
}

// 排列数计算
// Mersenne Twister 状态结构体

// 路径类型标识（新增）
#define PATH_TYPE 0x0B

// 路径对象结构体（新增）
struct PathObject {
    const char* path_str;  // 路径字符串
    bool is_absolute;      // 是否为绝对路径
};

// 路径拼接函数（新增）
void aqstl_path_join(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 2) EXIT_VM("aqstl_path_join", "需要至少两个路径参数");

    // 参数校验
    struct Object* base_obj = GetOriginData(object_table + args.index[0]);
    if (!base_obj || base_obj->type[0] != 0x05) EXIT_VM("aqstl_path_join", "基础路径必须为字符串类型");
    const char* base_path = GetStringObjectData(base_obj);

    // 构建拼接后的路径（简化示例，实际需处理Windows路径分隔符）
    char joined_path[1024] = {0};
    snprintf(joined_path, sizeof(joined_path), "%s\\%s", base_path, GetStringObjectData(GetOriginData(object_table + args.index[1])));

    // 创建路径对象并设置返回值
    struct Object* path_obj = (struct Object*)calloc(1, sizeof(struct Object));
    path_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    path_obj->type[0] = PATH_TYPE;
    path_obj->data.ptr_data = (struct Object*)malloc(strlen(joined_path) + 1);
    strcpy((char*)path_obj->data.ptr_data, joined_path);
    SetObjectData(return_value, path_obj);
}

// 路径存在性检查函数（新增）
void aqstl_path_exists(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_exists", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {  // 字符串类型
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {  // 自定义路径类型
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_exists", "参数必须是字符串或路径对象");
    }

    // 检查路径是否存在（Windows API）
    DWORD attr = GetFileAttributesA(path);
    bool exists = (attr != INVALID_FILE_ATTRIBUTES);

    // 设置返回值（长整型，1存在/0不存在）
    SetLongData(return_value, exists ? 1 : 0);
}

// 路径是否为目录检查函数（新增）
void aqstl_path_is_dir(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_is_dir", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {  // 字符串类型
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {  // 自定义路径类型
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_is_dir", "参数必须是字符串或路径对象");
    }

    // 检查是否为目录（跨平台实现）
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    bool is_dir = (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    bool is_dir = (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
#endif

    // 设置返回值（长整型，1是目录/0不是）
    SetLongData(return_value, is_dir ? 1 : 0);
}

void aqstl_path_abspath(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_abspath", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* rel_path;
    if (path_obj->type[0] == 0x05) {
        rel_path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        rel_path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_abspath", "参数必须是字符串或路径对象");
    }

    // 获取绝对路径（跨平台实现）
    char abs_path[1024];
#ifdef _WIN32
    GetFullPathNameA(rel_path, sizeof(abs_path), abs_path, NULL);
    // 统一为反斜杠
    for (char* p = abs_path; *p; p++) if (*p == '/') *p = '\\';
#else
    realpath(rel_path, abs_path);
#endif

    // 创建返回字符串对象
    SetStringData(return_value, abs_path);
}

void aqstl_path_basename(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_basename", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_basename", "参数必须是字符串或路径对象");
    }

    // 查找最后一个路径分隔符（跨平台）
    const char* last_sep = NULL;
#ifdef _WIN32
    last_sep = strpbrk(path, "/\\");
    while (last_sep) {
        const char* next = strpbrk(last_sep + 1, "/\\");
        if (!next) break;
        last_sep = next;
    }
#else
    last_sep = strrchr(path, '/');
#endif
    const char* basename = (last_sep && last_sep[1]) ? last_sep + 1 : path;
    if (last_sep && !last_sep[1]) basename = (last_sep > path) ? last_sep - 1 : path;

    // 创建返回字符串对象
    SetStringData(return_value, basename);
}

void aqstl_path_dirname(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_dirname", "需要一个路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_dirname", "参数必须是字符串或路径对象");
    }

    // 查找最后一个路径分隔符（跨平台）
    const char* last_sep = NULL;
#ifdef _WIN32
    last_sep = strpbrk(path, "/\\");
    while (last_sep) {
        const char* next = strpbrk(last_sep + 1, "/\\");
        if (!next) break;
        last_sep = next;
    }
#else
    last_sep = strrchr(path, '/');
#endif
    char dirname[1024];
    if (last_sep) {
        strncpy(dirname, path, last_sep - path);
        dirname[last_sep - path] = '\0';
    } else {
        strcpy(dirname, ".");
    }

    // 创建返回字符串对象
    SetStringData(return_value, dirname);
}

// 目录遍历函数（新增）
void aqstl_path_iterdir(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_path_iterdir", "需要一个目录路径参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    const char* path;
    if (path_obj->type[0] == 0x05) {  // 字符串类型
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {  // 自定义路径类型
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_iterdir", "参数必须是字符串或路径对象");
    }

#ifdef _WIN32
    // Windows系统实现
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        EXIT_VM("aqstl_path_iterdir", "路径不存在或不是目录");
    }

    // 拼接搜索路径（添加通配符）
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        EXIT_VM("aqstl_path_iterdir", "无法遍历目录");
    }

    // 创建结果列表
    struct Object* list_obj = (struct Object*)calloc(1, sizeof(struct Object));
    list_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    list_obj->type[0] = 0x0A;  // 列表类型
    list_obj->data.ptr_data = NULL;
    size_t list_size = 0;
#else
    // POSIX系统实现
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        EXIT_VM("aqstl_path_iterdir", "路径不存在或不是目录");
    }

    // 打开目录
    DIR* dir = opendir(path);
    if (dir == NULL) {
        EXIT_VM("aqstl_path_iterdir", "无法打开目录");
    }

    // 创建结果列表
    struct Object* list_obj = (struct Object*)calloc(1, sizeof(struct Object));
    list_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    list_obj->type[0] = 0x0A;  // 列表类型
    list_obj->data.ptr_data = NULL;
    size_t list_size = 0;

    // 遍历目录项
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 创建路径字符串对象
        struct Object* item_obj = (struct Object*)calloc(1, sizeof(struct Object));
        item_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
        item_obj->type[0] = 0x05;  // 字符串类型
        item_obj->data.string_data = strdup(entry->d_name);

        // 添加到列表
        list_size++;
        list_obj->data.ptr_data = realloc(list_obj->data.ptr_data, list_size * sizeof(struct Object*));
        ((struct Object**)list_obj->data.ptr_data)[list_size - 1] = item_obj;
    }
    closedir(dir);
#endif

    // 设置列表长度（公共部分）
    struct Object* size_obj = (struct Object*)calloc(1, sizeof(struct Object));
    size_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    size_obj->type[0] = 0x02;  // 长整型
    size_obj->data.long_data = list_size;

    // 返回列表对象（假设列表结构为[size, items...]）
    struct Object* result_obj = (struct Object*)calloc(2, sizeof(struct Object));
    result_obj[0] = *size_obj;
    result_obj[1] = *list_obj;
    SetObjectData(return_value, result_obj);
}

// 路径模式匹配函数（新增）
void aqstl_path_glob(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 2) EXIT_VM("aqstl_path_glob", "需要目录路径和模式两个参数");

    // 参数校验
    struct Object* path_obj = GetOriginData(object_table + args.index[0]);
    struct Object* pattern_obj = GetOriginData(object_table + args.index[1]);
    const char* path;
    const char* pattern;

    // 校验路径类型
    if (path_obj->type[0] == 0x05) {
        path = GetStringObjectData(path_obj);
    } else if (path_obj->type[0] == 0x0B) {
        path = (const char*)path_obj->data.ptr_data;
    } else {
        EXIT_VM("aqstl_path_glob", "路径参数必须是字符串或路径对象");
    }

    // 校验模式类型
    if (pattern_obj->type[0] != 0x05) {
        EXIT_VM("aqstl_path_glob", "模式参数必须是字符串");
    }
    pattern = GetStringObjectData(pattern_obj);

    // 拼接搜索路径
    char search_path[1024];
    snprintf(search_path, sizeof(search_path), "%s\\%s", path, pattern);

    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        EXIT_VM("aqstl_path_glob", "无匹配文件或路径无效");
    }

    // 创建结果列表
    struct Object* list_obj = (struct Object*)calloc(1, sizeof(struct Object));
    list_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    list_obj->type[0] = 0x0A;  // 列表类型
    list_obj->data.ptr_data = NULL;
    size_t list_size = 0;

    // 收集匹配项
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }

        // 创建路径字符串对象
        struct Object* item_obj = (struct Object*)calloc(1, sizeof(struct Object));
        item_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
        item_obj->type[0] = 0x05;  // 字符串类型
        item_obj->data.string_data = strdup(find_data.cFileName);

        // 添加到列表
        list_size++;
        list_obj->data.ptr_data = (struct Object*)realloc(list_obj->data.ptr_data, list_size * sizeof(struct Object*));
        ((struct Object**)list_obj->data.ptr_data)[list_size - 1] = item_obj;
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);

    // 设置列表长度
    struct Object* size_obj = (struct Object*)calloc(1, sizeof(struct Object));
    size_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
    size_obj->type[0] = 0x02;  // 长整型
    size_obj->data.long_data = list_size;

    // 返回列表对象（结构：[size, items...]）
    struct Object* result_obj = (struct Object*)calloc(2, sizeof(struct Object));
    result_obj[0] = *size_obj;
    result_obj[1] = *list_obj;
    SetObjectData(return_value, result_obj);
}

// 实现os.path.commonpath函数
void aqstl_path_commonpath(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 2) EXIT_VM("aqstl_path_commonpath", "需要至少两个路径参数");

    // 收集路径列表
    const char* paths[args.size];
    for (size_t i = 0; i < args.size; i++) {
        struct Object* path_obj = GetOriginData(object_table + args.index[i]);
        if (path_obj->type[0] != 0x05 && path_obj->type[0] != 0x0B) {
            EXIT_VM("aqstl_path_commonpath", "参数必须是字符串或路径对象");
        }
        paths[i] = (path_obj->type[0] == 0x05) ? GetStringObjectData(path_obj) : (const char*)path_obj->data.ptr_data;
    }

    // 计算最长公共子路径（跨平台）
    char common[1024] = {0};
#ifdef _WIN32
    // Windows路径处理（统一为反斜杠）
    char normalized_paths[args.size][1024];
    for (size_t i = 0; i < args.size; i++) {
        strncpy(normalized_paths[i], paths[i], sizeof(normalized_paths[i])-1);
        for (char* p = normalized_paths[i]; *p; p++) if (*p == '/') *p = '\\';
    }
    // 取第一个路径的前缀逐步匹配
    strncpy(common, normalized_paths[0], sizeof(common)-1);
    for (size_t i = 1; i < args.size; i++) {
        size_t min_len = strlen(common) < strlen(normalized_paths[i]) ? strlen(common) : strlen(normalized_paths[i]);
        for (size_t j = 0; j < min_len; j++) {
            if (common[j] != normalized_paths[i][j]) {
                common[j] = '\0';
                break;
            }
        }
    }
#else
    // Unix路径处理（统一为正斜杠）
    char normalized_paths[args.size][1024];
    for (size_t i = 0; i < args.size; i++) {
        strncpy(normalized_paths[i], paths[i], sizeof(normalized_paths[i])-1);
        for (char* p = normalized_paths[i]; *p; p++) if (*p == '\\') *p = '/';
    }
    // 取第一个路径的前缀逐步匹配
    strncpy(common, normalized_paths[0], sizeof(common)-1);
    for (size_t i = 1; i < args.size; i++) {
        size_t min_len = strlen(common) < strlen(normalized_paths[i]) ? strlen(common) : strlen(normalized_paths[i]);
        for (size_t j = 0; j < min_len; j++) {
            if (common[j] != normalized_paths[i][j]) {
                common[j] = '\0';
                break;
            }
        }
    }
#endif

    // 创建返回字符串对象
    SetStringData(return_value, common);
}

// 实现os.path.commonprefix函数
void aqstl_path_commonprefix(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 2) EXIT_VM("aqstl_path_commonprefix", "需要至少两个路径参数");

    // 收集路径列表
    const char* paths[args.size];
    for (size_t i = 0; i < args.size; i++) {
        struct Object* path_obj = GetOriginData(object_table + args.index[i]);
        if (path_obj->type[0] != 0x05 && path_obj->type[0] != 0x0B) {
            EXIT_VM("aqstl_path_commonprefix", "参数必须是字符串或路径对象");
        }
        paths[i] = (path_obj->type[0] == 0x05) ? GetStringObjectData(path_obj) : (const char*)path_obj->data.ptr_data;
    }

    // 计算最长公共前缀（逐字符比较）
    size_t min_len = strlen(paths[0]);
    for (size_t i = 1; i < args.size; i++) {
        size_t len = strlen(paths[i]);
        if (len < min_len) min_len = len;
    }

    char prefix[1024] = {0};
    for (size_t i = 0; i < min_len; i++) {
        char c = paths[0][i];
        for (size_t j = 1; j < args.size; j++) {
            if (paths[j][i] != c) {
                prefix[i] = '\0';
                goto end;
            }
        }
        prefix[i] = c;
    }
end:

    // 创建返回字符串对象
    SetStringData(return_value, prefix);
}

typedef struct {
    uint32_t state[624];
    int index;
} MTRandomState;

static MTRandomState mt_state;

// 初始化Mersenne Twister生成器
static void mt_init(uint32_t seed) {
    mt_state.index = 624;
    mt_state.state[0] = seed;
    for (int i = 1; i < 624; i++) {
        mt_state.state[i] = 0x6c078965 * (mt_state.state[i-1] ^ (mt_state.state[i-1] >> 30)) + i;
    }
}

// 生成下一个32位随机数
static uint32_t mt_next(void) {
    if (mt_state.index >= 624) {
        for (int i = 0; i < 624; i++) {
            uint32_t y = (mt_state.state[i] & 0x80000000) | (mt_state.state[(i+1)%624] & 0x7fffffff);
            mt_state.state[i] = mt_state.state[(i+397)%624] ^ (y >> 1);
            if (y & 0x00000001) {
                mt_state.state[i] ^= 0x9908b0df;
            }
        }
        mt_state.index = 0;
    }
    uint32_t y = mt_state.state[mt_state.index++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680;
    y ^= (y << 15) & 0xefc60000;
    y ^= (y >> 18);
    return y;
}

// 实现random.seed函数
void aqstl_random_seed(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 1 || args.size > 1) EXIT_VM("aqstl_random_seed", "Requires 1 argument (seed).");
    if (return_value >= object_table_size) EXIT_VM("aqstl_random_seed", "Invalid return value slot.");

    struct Object* seed_obj = GetOriginData(object_table + args.index[0]);
    uint32_t seed;
    switch (seed_obj->type[0]) {
        case 0x02: // 整数类型
            seed = (uint32_t)GetLongObjectData(seed_obj);
            break;
        case 0x05: { // 字符串类型
            const char* str = GetStringObjectData(seed_obj);
            seed = 0;
            for (size_t i = 0; str[i] != '\0'; i++) {
                seed = seed * 131 + str[i];
            }
            break;
        }
        default:
            EXIT_VM("aqstl_random_seed", "Unsupported seed type.");
    }
    mt_init(seed);
    SetLongData(return_value, 0); // 无返回值，返回0表示成功
}

// 实现random.randrange函数
void aqstl_random_randrange(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size < 1 || args.size > 3) EXIT_VM("aqstl_random_randrange", "Requires 1-3 arguments (stop) or (start, stop[, step]).");
    if (return_value >= object_table_size) EXIT_VM("aqstl_random_randrange", "Invalid return value slot.");

    int64_t start = 0, stop, step = 1;
    if (args.size == 1) {
        stop = GetLongObjectData(GetOriginData(object_table + args.index[0]));
    } else if (args.size >= 2) {
        start = GetLongObjectData(GetOriginData(object_table + args.index[0]));
        stop = GetLongObjectData(GetOriginData(object_table + args.index[1]));
        if (args.size == 3) step = GetLongObjectData(GetOriginData(object_table + args.index[2]));
    }

    if (step == 0) EXIT_VM("aqstl_random_randrange", "step cannot be zero");
    if ((stop - start) * step <= 0) EXIT_VM("aqstl_random_randrange", "empty range");

    int64_t count = (stop - start + step - (step > 0 ? 1 : -1)) / step;
    int64_t rand_idx = mt_next() % count;
    int64_t result = start + rand_idx * step;
    SetLongData(return_value, result);
}

// 实现random.choice函数
void aqstl_random_choice(InternalObject args, size_t return_value) {
    TRACE_FUNCTION;
    if (args.size != 1) EXIT_VM("aqstl_random_choice", "Requires 1 argument (sequence).");
    if (return_value >= object_table_size) EXIT_VM("aqstl_random_choice", "Invalid return value slot.");

    struct Object* seq_obj = GetOriginData(object_table + args.index[0]);
    if (seq_obj->type[0] != 0x06) EXIT_VM("aqstl_random_choice", "sequence must be a list");

    size_t length = GetUint64tObjectData(seq_obj);
    if (length == 0) EXIT_VM("aqstl_random_choice", "cannot choose from an empty sequence");

    size_t idx = mt_next() % length;
    struct Object* elem = GetPtrObjectData(seq_obj) + idx + 1;
    SetObjectData(return_value, elem);
}

void aqstl_math_perm(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1 || args.size > 2) EXIT_VM("aqstl_math_perm", "Requires 1-2 arguments (n, k).");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_perm", "Invalid return value slot.");

  struct Object* n_obj = GetOriginData(object_table + args.index[0]);
  if (n_obj->type[0] != 0x02) EXIT_VM("aqstl_math_perm", "n must be integer.");
  int64_t n = GetLongObjectData(n_obj);
  int64_t k = (args.size == 2) ? GetLongObjectData(GetOriginData(object_table + args.index[1])) : n;
  if (n < 0 || k < 0 || k > n) EXIT_VM("aqstl_math_perm", "Invalid permutation parameters.");

  int64_t result = 1;
  for (int64_t i = 0; i < k; i++) result *= (n - i);
  SetLongData(return_value, result);
}

// 整数平方根计算
void aqstl_math_isqrt(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_isqrt", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_isqrt", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x02) EXIT_VM("aqstl_math_isqrt", "Argument must be integer.");
  int64_t n = GetLongObjectData(obj);
  if (n < 0) EXIT_VM("aqstl_math_isqrt", "Integer square root of negative number is undefined.");

  int64_t x = n;
  int64_t y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  SetLongData(return_value, x);
}

// 指数函数计算
void aqstl_math_exp(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_exp", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_exp", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_exp", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, exp(x));
}

// 余弦函数计算
void aqstl_math_cos(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_cos", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_cos", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_cos", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, cos(x));
}

// 正切函数计算
void aqstl_math_tan(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_tan", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_tan", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_tan", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  if (fmod(x + M_PI/2, M_PI) == 0) EXIT_VM("aqstl_math_tan", "Tangent undefined at odd multiples of π/2.");
  SetDoubleData(return_value, tan(x));
}

// 自然对数计算
void aqstl_math_log(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_log", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_log", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_log", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  if (x <= 0) EXIT_VM("aqstl_math_log", "Logarithm of non-positive number is undefined.");
  SetDoubleData(return_value, log(x));
}

// 向下取整计算
void aqstl_math_floor(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_floor", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_floor", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_floor", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, floor(x));
}

// 反余弦函数计算
void aqstl_math_acos(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_acos", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_acos", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_acos", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  if (x < -1 || x > 1) EXIT_VM("aqstl_math_acos", "Argument must be in [-1, 1].");
  SetDoubleData(return_value, acos(x));
}

// 反正弦函数计算
void aqstl_math_asin(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_asin", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_asin", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_asin", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  if (x < -1 || x > 1) EXIT_VM("aqstl_math_asin", "Argument must be in [-1, 1].");
  SetDoubleData(return_value, asin(x));
}

// 反正切函数计算
void aqstl_math_atan(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_atan", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_atan", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_atan", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, atan(x));
}

// 双曲正弦函数计算
void aqstl_math_sinh(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_sinh", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_sinh", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_sinh", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, sinh(x));
}

// 双曲余弦函数计算
void aqstl_math_cosh(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_math_cosh", "Requires 1 argument.");
  if (return_value >= object_table_size) EXIT_VM("aqstl_math_cosh", "Invalid return value slot.");

  struct Object* obj = GetOriginData(object_table + args.index[0]);
  if (obj->type[0] != 0x03) EXIT_VM("aqstl_math_cosh", "Argument must be float.");
  double x = GetDoubleObjectData(obj);
  SetDoubleData(return_value, cosh(x));
}

void aqstl_type(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_type", "Requires exactly 1 argument.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_type", "Invalid return value slot.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_type", "Null object.");

  const char* type_names[] = {"BYTE", "LONG", "DOUBLE", "STRING", "POINTER"};
  const char* type_name = "UNKNOWN";
  if (obj->type[0] < 0x05) type_name = type_names[obj->type[0]];

  SetStringData(return_value, type_name);
}

void aqstl_hex(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_hex", "Requires exactly 1 argument.");

  struct Object* obj = object_table + args.index[0];
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_hex", "Null object.");

  char buffer[64];
  switch (obj->type[0]) {
    case 0x02:  // Long
      snprintf(buffer, sizeof(buffer), "%llx",
               (unsigned long long)GetLongObjectData(obj));
      break;
    case 0x01:  // Byte
      snprintf(buffer, sizeof(buffer), "%llx",
               (unsigned long long)GetByteObjectData(obj));
      break;
    case 0x04:  // Uint64t
      snprintf(buffer, sizeof(buffer), "%llx",
               (unsigned long long)GetUint64tObjectData(obj));
      break;
    case 0x03:  // Double
      snprintf(buffer, sizeof(buffer), "%llx",
               (unsigned long long)GetDoubleObjectData(obj));
      break;
    default:
      EXIT_VM("aqstl_hex", "Unsupported type for hex conversion.");
  }
  SetStringData(return_value, buffer);
}

void aqstl_id(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_id", "Requires exactly 1 argument.");

  struct Object* obj = object_table + args.index[0];
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_id", "Null object.");

  SetLongData(return_value, args.index[0]);
}

void aqstl_input(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size > 1)
    EXIT_VM("aqstl_input", "Supports at most 1 prompt argument.");

  const char* prompt = "";
  if (args.size == 1) {
    struct Object* prompt_obj = object_table + args.index[0];
    prompt_obj = GetOriginData(prompt_obj);
    if (!prompt_obj || prompt_obj->type[0] != 0x05)  // String
      EXIT_VM("aqstl_input", "Prompt must be a string.");
    prompt = GetStringObjectData(prompt_obj);
  }

  char* buffer = (char*)malloc(1024 * sizeof(char));
  printf("%s", prompt);
  fflush(stdout);
  if (!fgets(buffer, sizeof(buffer), stdin)) {
    EXIT_VM("aqstl_input", "Input read failed.");
  }

  size_t len = strlen(buffer);
  if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

  SetStringData(return_value, buffer);
}

void aqstl_open(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size < 1 || args.size > 3)
    EXIT_VM("aqstl_open", "Requires 1-3 arguments (path, mode, buffering).");

  struct Object* path_obj = object_table + args.index[0];
  path_obj = GetOriginData(path_obj);
  if (!path_obj || path_obj->type[0] != 0x05)  // String
    EXIT_VM("aqstl_open", "Path must be a string.");

  const char* path = GetStringObjectData(path_obj);
  const char* mode = "r";
  int buffering = -1;

  if (args.size >= 2) {
    struct Object* mode_obj = object_table + args.index[1];
    mode_obj = GetOriginData(mode_obj);
    if (!mode_obj || mode_obj->type[0] != 0x05)  // String
      EXIT_VM("aqstl_open", "Mode must be a string.");
    mode = GetStringObjectData(mode_obj);
  }

  if (args.size >= 3) {
    struct Object* buf_obj = object_table + args.index[2];
    buf_obj = GetOriginData(buf_obj);
    if (!buf_obj || buf_obj->type[0] != 0x02)  // Long
      EXIT_VM("aqstl_open", "Buffering must be an integer.");
    buffering = (int)GetLongObjectData(buf_obj);
  }

  FILE* file = fopen(path, mode);
  if (!file) EXIT_VM("aqstl_open", "Failed to open file.");

  struct Object* file_obj = (struct Object*)calloc(3, sizeof(struct Object));

  file_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
  file_obj->type[0] = 0x05;  // File type
  file_obj->data.string_data = "!FILE!";
  file_obj++;
  file_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
  file_obj->type[0] = 0x06;  // File type
  file_obj->data.ptr_data = (struct Object*)file;
  file_obj++;
  file_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
  file_obj->type[0] = 0x02;  // File type
  file_obj->data.long_data = buffering;

  // size_t file_index = CreateFileObject(file, buffering);
  SetObjectData(return_value, file_obj - 2);
}

void aqstl_close(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 1) EXIT_VM("aqstl_close", "Invalid arguments.");

  if (return_value >= object_table_size)
    EXIT_VM("aqstl_close", "Invalid return value.");

  struct Object* obj = object_table + *args.index;
  obj = GetOriginData(obj);
  if (!obj) EXIT_VM("aqstl_close", "Null object.");

  if (obj->type[0] != 0x09) EXIT_VM("aqstl_close", "Object is not a file.");

  obj = GetObjectObjectData(obj);
  obj = GetOriginData(obj);

  if (obj->type[0] != 0x05 ||
      strcmp(obj->data.string_data, "!FILE!") != 0)  // File type
    EXIT_VM("aqstl_close", "Object is not a file.");

  obj++;

  FILE* file = (FILE*)obj->data.ptr_data;
  if (file) {
    fclose(file);
    SetPtrObjectData(obj, NULL);
  } else {
    EXIT_VM("aqstl_close", "Failed to close file.");
  }
}

void aqstl_read(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 2)
    EXIT_VM("aqstl_read", "Requires exactly 2 arguments (file, size).");

  struct Object* file_obj = object_table + args.index[0];
  file_obj = GetOriginData(file_obj);
  if (!file_obj || file_obj->type[0] != 0x09)  // File type
    EXIT_VM("aqstl_read", "First argument must be a file.");

  struct Object* size_obj = object_table + args.index[1];
  size_obj = GetOriginData(size_obj);
  if (!size_obj || size_obj->type[0] != 0x02)  // Long
    EXIT_VM("aqstl_read", "Second argument must be an integer.");

  FILE* file = (FILE*)GetObjectObjectData(file_obj)->data.ptr_data;
  size_t size = (size_t)GetLongObjectData(size_obj);

  char* buffer = (char*)malloc(size * sizeof(char));
  size_t read_size = fread(buffer, sizeof(char), size, file);
  if (read_size != size) {
    free(buffer);
    EXIT_VM("aqstl_read", "Failed to read from file.");
  }

  struct Object* str_obj = (struct Object*)calloc(1, sizeof(struct Object));
  str_obj->type = (uint8_t*)calloc(1, sizeof(uint8_t));
  str_obj->type[0] = 0x05;  // String type
  str_obj->data.string_data = buffer;

  SetObjectData(return_value, str_obj);
}

void aqstl_write(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size != 3)
    EXIT_VM("aqstl_write",
            "Requires exactly 3 arguments (file, string, size).");

  struct Object* file_obj = object_table + args.index[0];
  file_obj = GetOriginData(file_obj);
  if (!file_obj || file_obj->type[0] != 0x09)  // File type
    EXIT_VM("aqstl_write", "First argument must be a file.");

  struct Object* str_obj = object_table + args.index[1];
  str_obj = GetOriginData(str_obj);

  if (!str_obj || str_obj->type[0] != 0x05)  // String
    EXIT_VM("aqstl_write", "Second argument must be a string.");

  const char* str = GetStringObjectData(str_obj);
  size_t str_len = strlen(str);
  if (str_len == 0) EXIT_VM("aqstl_write", "String is empty.");

  if (str_len > 1024) EXIT_VM("aqstl_write", "String is too long.");

  struct Object* size_obj = object_table + args.index[2];

  size_obj = GetOriginData(size_obj);

  if (!size_obj || size_obj->type[0] != 0x02)  // Long
    EXIT_VM("aqstl_write", "Third argument must be an integer.");

  size_t size = (size_t)GetLongObjectData(size_obj);

  FILE* file = (FILE*)GetObjectObjectData(file_obj)->data.ptr_data;

  size_t write_size = fwrite(str, sizeof(char), size, file);
  if (write_size != size) {
    EXIT_VM("aqstl_write", "Failed to write to file.");
  }

  SetLongData(return_value, write_size);
}

unsigned int hash(const char* str) {
  TRACE_FUNCTION;
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash % 256;
}

void AddFuncToNameTable(char* name, func_ptr func) {
  TRACE_FUNCTION;
  unsigned int name_hash = hash(name);
  struct LinkedList* table = &name_table[name_hash];
  while (table->next != NULL) {
    table = table->next;
  }
  table->pair.first = name;
  table->pair.second = func;
  table->next = (struct LinkedList*)malloc(sizeof(struct LinkedList));
  AddFreePtr(table->next);
  table->next->next = NULL;
  table->next->pair.first = NULL;
  table->next->pair.second = NULL;
}

void InitializeNameTable(struct LinkedList* list) {
  AddFuncToNameTable("__builtin_print", aqstl_print);
  AddFuncToNameTable("__builtin_vaprint", aqstl_vaprint);
  AddFuncToNameTable("__builtin_remove", aqstl_remove);
  AddFuncToNameTable("__builtin_rename", aqstl_rename);
  AddFuncToNameTable("__builtin_getchar", aqstl_getchar);
  AddFuncToNameTable("__builtin_putchar", aqstl_putchar);
  AddFuncToNameTable("__builtin_puts", aqstl_puts);
  AddFuncToNameTable("__builtin_perror", aqstl_perror);
  // abs，ascii，bin，bool，chr，float，format，hash，hex，id，input，int，len，max，min，oct，open，ord，pow，print，round，str，sum，type
  AddFuncToNameTable("__builtin_abs", aqstl_abs);
  AddFuncToNameTable("__builtin_ascii", aqstl_ascii);
  AddFuncToNameTable("__builtin_bin", aqstl_bin);
  AddFuncToNameTable("__builtin_bool", aqstl_bool);
  AddFuncToNameTable("__builtin_chr", aqstl_chr);
  AddFuncToNameTable("__builtin_float", aqstl_float);
  // AddFuncToNameTable("__builtin_format", aqstl_format);
  AddFuncToNameTable("__builtin_hash", aqstl_hash);
  AddFuncToNameTable("__builtin_hex", aqstl_hex);
  AddFuncToNameTable("__builtin_id", aqstl_id);
  AddFuncToNameTable("__builtin_input", aqstl_input);
  AddFuncToNameTable("__builtin_int", aqstl_int);
  AddFuncToNameTable("__builtin_len", aqstl_len);
  AddFuncToNameTable("__builtin_max", aqstl_max);
  AddFuncToNameTable("__builtin_min", aqstl_min);
  AddFuncToNameTable("__builtin_oct", aqstl_oct);
  AddFuncToNameTable("__builtin_open", aqstl_open);
  AddFuncToNameTable("__builtin_close", aqstl_close);
  AddFuncToNameTable("__builtin_ord", aqstl_ord);
  AddFuncToNameTable("__builtin_pow", aqstl_pow);
  AddFuncToNameTable("__builtin_print", aqstl_print);
  AddFuncToNameTable("__builtin_round", aqstl_round);
  AddFuncToNameTable("__builtin_str", aqstl_str);
  AddFuncToNameTable("__builtin_sum", aqstl_sum);
  AddFuncToNameTable("__builtin_type", aqstl_type);
  AddFuncToNameTable("__builtin_read", aqstl_read);
  AddFuncToNameTable("__builtin_write", aqstl_write);
}