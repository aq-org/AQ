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