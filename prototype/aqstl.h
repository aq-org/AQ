// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


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
const char* GetStringData(size_t index);
uint64_t GetUint64tObjectData(struct Object* object);
uint64_t GetUint64tData(size_t index);
double GetDoubleData(size_t index);
int64_t GetLongData(size_t index);
int8_t GetByteObjectData(struct Object* data);
int8_t GetByteData(size_t index);
struct Object* GetPtrData(size_t index);
void SetByteData(size_t, int8_t);
void SetLongData(size_t, int64_t);
void SetDoubleData(size_t, double);
void SetUint64tData(size_t, uint64_t);
void SetPtrData(size_t index, struct Object* ptr);
void SetStringData(size_t index, const char* string);
void SetReferenceData(size_t index, struct Object* object);
void SetConstData(size_t index, struct Object* object);
void SetObjectData(size_t index, struct Object* object);

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
  if (args.size < 1)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid return value.");
  for (size_t i = 0; i < args.size; i++) {
    struct Object* object = object_table + args.index[i];
    object = GetOriginData(object);
    if (object == NULL)
      EXIT_VM("aqstl_vaprint(InternalObject,size_t)", "Invalid object.");

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
}