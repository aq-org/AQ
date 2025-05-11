// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
  void* origin_data;
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
  const char* first;
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

void aqstl_abs(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size!= 1)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid return value.");

  struct Object* object = object_table + *args.index;
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_abs(InternalObject,size_t)", "Invalid object.");

  switch (object->type[0]) {
    case 0x01:
      SetByteData(return_value, abs(GetByteData(*args.index)));
      break;
    case 0x02:
      SetLongData(return_value, llabs(GetLongData(*args.index)));
      break;
    case 0x03:
      SetDoubleData(return_value, fabs(GetDoubleData(*args.index)));
      break;
    case 0x04:
      SetUint64tData(return_value, GetUint64tData(*args.index));
      break;
    default:
      EXIT_VM("aqstl_abs(InternalObject,size_t)", "Unsupported type.");
  }
}

void aqstl_open(InternalObject args, size_t return_value) {
  TRACE_FUNCTION;
  if (args.size!= 1)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid args.");
  if (return_value >= object_table_size)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid return value.");
  struct Object* object = object_table + *args.index;
  object = GetOriginData(object);

  if (object == NULL)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid object.");

  if (object->type[0]!= 0x05)
    EXIT_VM("aqstl_open(InternalObject,size_t)", "Invalid object.");
}

#ifdef __unix__
#include <ncurses.h>

struct Object* curses_error;

bool curses_initialised = false;

int aqstl_CursesCheckERR(int code, char* fname) {
  char buf[100];

  if (code != ERR) {
    return 0;
  } else {
    if (fname == NULL) {
      EXIT_VM("aqstl_CursesCheckERR", "curses function returned ERR");
    } else {
      strcpy(buf, fname);
      strcat(buf, "() returned ERR");
      EXIT_VM("aqstl_CursesCheckERR", buf);
    }
    return -1;
  }
}

int aqstl_CursesInitialised() {
  if (curses_initialised == true)
    return 1;
  else {
    EXIT_VM("aqstl_CursesInitialised", "must call initscr() first");
    return 0;
  }
}

struct Object* aqstl_CursesWindowNew(WINDOW* win) {
  struct Object* wo;

  wo = calloc(1, sizeof(struct Object));
  if (wo == NULL)
    EXIT_VM("aqstl_CursesWindowNew", "Failed to allocate memory.");
  wo->type = calloc(1, sizeof(uint8_t));
  if (wo->type == NULL)
    EXIT_VM("aqstl_CursesWindowNew", "Failed to allocate memory.");
  *(wo->type) = 0x0A;

  wo->data.origin_data = calloc(2, sizeof(WINDOW*));
  if (wo->data.origin_data == NULL)
    EXIT_VM("aqstl_CursesWindowNew", "Failed to allocate memory.");
  *(WINDOW**)(wo->data.origin_data) = win;
  *(((WINDOW**)(wo->data.origin_data)) + 1) = NULL;
  return wo;
}

void aqstl_CursesWindowDealloc(struct Object* wo) {
  if (wo->data.origin_data != stdscr) delwin(wo->data.origin_data);
  free(wo->type);
  free(wo);
}

void aqstl_CursesWindowRefresh(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowRefresh", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wrefresh(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "wrefresh"));
}

void aqstl_CursesWindowNoOutRefresh(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowNoOutRefresh", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wnoutrefresh(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "wnoutrefresh"));
}

void aqstl_CursesWindowMoveWin(InternalObject args, size_t return_value) {
  if (args.size != 3)
    EXIT_VM("aqstl_CursesWindowMoveWin", "Unsupported argument count.");
  int x = GetLongData(args.index[1]);
  int y = GetLongData(args.index[2]);
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          mvwin(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x),
          "mvwin"));
}

void aqstl_CursesWindowMove(InternalObject args, size_t return_value) {
  if (args.size != 3)
    EXIT_VM("aqstl_CursesWindowMove", "Unsupported argument count.");
  int x = GetLongData(args.index[1]);
  int y = GetLongData(args.index[2]);
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wmove(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x),
          "wmove"));
}

void aqstl_CursesWindowSubWin(InternalObject args, size_t return_value) {
  WINDOW* win;
  struct Object* rtn_win;
  int nlines, ncols, begin_y, begin_x;

  nlines = 0;
  ncols = 0;
  switch (args.size) {
    case 2:
      begin_y = GetLongData(args.index[1]);
      begin_x = GetLongData(args.index[2]);
      break;
    case 4:
      nlines = GetLongData(args.index[1]);
      ncols = GetLongData(args.index[2]);
      begin_y = GetLongData(args.index[3]);
      begin_x = GetLongData(args.index[4]);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowMoveWin", "Unsupported argument count.");
  }
  win = subwin(*(WINDOW**)object_table[args.index[0]].data.origin_data, nlines,
               ncols, begin_y, begin_x);
  if (win == NULL) {
    EXIT_VM("aqstl_CursesWindowSubWin", "curses function returned NULL");
  }
  rtn_win = aqstl_CursesWindowNew(win);
  *(((WINDOW**)rtn_win->data.origin_data) + 1) =
      *(WINDOW**)object_table[args.index[0]].data.origin_data;
  SetReferenceData(return_value, rtn_win);
}

void aqstl_CursesWindowAddCh(InternalObject args, size_t return_value) {
  int rtn;
  int x, y;
  int ch;
  int attr, attr_old;
  int use_xy = false, use_attr = false;

  switch (args.size) {
    case 1:
      ch = GetLongData(args.index[1]);
      break;
    case 2:
      ch = GetLongData(args.index[1]);
      attr = GetLongData(args.index[2]);
      use_attr = true;
      break;
    case 3:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      use_xy = true;
      break;
    case 4:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      attr = GetLongData(args.index[4]);
      use_xy = use_attr = true;
      break;
    default:
      EXIT_VM("aqstl_CursesWindowAddCh", "addch requires 1 to 4 arguments");
  }

  if (use_attr == true) {
    attr_old =
        getattrs(*(WINDOW**)object_table[args.index[0]].data.origin_data);
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr);
  }
  if (use_xy == true)
    rtn = mvwaddch(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                   x, ch);
  else
    rtn = waddch(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
  if (use_attr == true)
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr_old);

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "[mv]waddch"));
}

void aqstl_CursesWindowInsCh(InternalObject args, size_t return_value) {
  int rtn;
  int x, y;
  int ch;
  int attr, attr_old;
  int use_xy = true, use_attr = false;

  switch (args.size) {
    case 1:
      ch = GetLongData(args.index[1]);
      break;
    case 2:
      ch = GetLongData(args.index[1]);
      attr = GetLongData(args.index[2]);
      use_attr = true;
      break;
    case 3:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      use_xy = true;
      break;
    case 4:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      attr = GetLongData(args.index[4]);
      use_xy = use_attr = true;
      break;
    default:
      EXIT_VM("aqstl_CursesWindowInsCh", "insch requires 1 to 4 arguments");
  }

  if (use_attr == true) {
    attr_old =
        getattrs(*(WINDOW**)object_table[args.index[0]].data.origin_data);
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr);
  }
  if (use_xy == true)
    rtn = mvwinsch(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                   x, ch);
  else
    rtn = winsch(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
  if (use_attr == true)
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr_old);

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "[mv]winsch"));
}

void aqstl_CursesWindowDelCh(InternalObject args, size_t return_value) {
  int rtn;
  int x, y;

  switch (args.size) {
    case 0:
      rtn = wdelch(*(WINDOW**)object_table[args.index[0]].data.origin_data);
      break;
    case 2:
      x = GetLongData(args.index[1]);
      y = GetLongData(args.index[2]);
      rtn = mvwdelch(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                     x);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowDelCh", "delch requires 0 or 2 arguments");
  }

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "[mv]wdelch"));
}

void aqstl_CursesWindowEchoChar(InternalObject args, size_t return_value) {
  int rtn;
  int ch;
  int attr, attr_old;

  switch (args.size) {
    case 1:
      ch = GetLongData(args.index[1]);
      rtn = wechochar(*(WINDOW**)object_table[args.index[0]].data.origin_data,
                      ch);
      break;
    case 2:
      ch = GetLongData(args.index[1]);
      attr = GetLongData(args.index[2]);

      attr_old =
          getattrs(*(WINDOW**)object_table[args.index[0]].data.origin_data);
      wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr);
      rtn = wechochar(*(WINDOW**)object_table[args.index[0]].data.origin_data,
                      ch);
      wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data,
               attr_old);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowEchoChar",
              "echochar requires 1 or 2 arguments");
  }

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "wechochar"));
}

void aqstl_CursesWindowAddStr(InternalObject args, size_t return_value) {
  int rtn;
  int x, y;
  const char* str;
  int attr, attr_old;
  int use_xy = false, use_attr = false;

  switch (args.size) {
    case 1:
      str = GetStringData(args.index[1]);
      break;
    case 2:
      str = GetStringData(args.index[1]);
      attr = GetLongData(args.index[2]);
      use_attr = true;
      break;
    case 3:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      str = GetStringData(args.index[3]);
      use_xy = true;
      break;
    case 4:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      str = GetStringData(args.index[3]);
      attr = GetLongData(args.index[4]);
      use_xy = use_attr = true;
      break;
    default:
      EXIT_VM("aqstl_CursesWindowAddStr", "addstr requires 1 to 4 arguments");
  }

  if (use_attr == true) {
    attr_old =
        getattrs(*(WINDOW**)object_table[args.index[0]].data.origin_data);
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr);
  }
  if (use_xy == true)
    rtn = mvwaddstr(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                    x, str);
  else
    rtn = waddstr(*(WINDOW**)object_table[args.index[0]].data.origin_data, str);
  if (use_attr == true)
    wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, attr_old);

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "[mv]waddstr"));
}

void aqstl_CursesWindowAttrOn(InternalObject args, size_t return_value) {
  int ch;
  ch = GetLongData(args.index[1]);
  wattron(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesWindowAttrOff(InternalObject args, size_t return_value) {
  int ch;
  ch = GetLongData(args.index[1]);
  wattroff(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesWindowAttrSet(InternalObject args, size_t return_value) {
  int ch;
  ch = GetLongData(args.index[1]);
  wattrset(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesWindowStandEnd(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowStandEnd", "Unsupported argument count.");
  wstandend(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowStandOut(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowStandOut", "Unsupported argument count.");
  wstandout(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowBorder(InternalObject args, size_t return_value) {
  int ls = GetLongData(args.index[1]);
  int rs = GetLongData(args.index[2]);
  int ts = GetLongData(args.index[3]);
  int bs = GetLongData(args.index[4]);
  int tl = GetLongData(args.index[5]);
  int tr = GetLongData(args.index[6]);
  int bl = GetLongData(args.index[7]);
  int br = GetLongData(args.index[8]);
  wborder(*(WINDOW**)object_table[args.index[0]].data.origin_data, ls, rs, ts,
          bs, tl, tr, bl, br);
}

void aqstl_CursesWindowBox(InternalObject args, size_t return_value) {
  int ch1 = 0, ch2 = 0;
  if (args.size != 3) {
    ch1 = GetLongData(args.index[1]);
    ch2 = GetLongData(args.index[2]);
  }
  box(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch1, ch2);
}

void aqstl_CursesWindowHline(InternalObject args, size_t return_value) {
  int ch, n, x, y, code = OK;
  switch (args.size) {
    case 2:
      ch = GetLongData(args.index[1]);
      n = GetLongData(args.index[2]);
      break;
    case 4:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      n = GetLongData(args.index[4]);
      code =
          wmove(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowHline", "hline requires 2 or 4 arguments");
  }
  if (code != ERR)
    whline(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch, n);
  SetLongData(return_value, aqstl_CursesCheckERR(code, "wmove"));
}

void aqstl_CursesWindowVline(InternalObject args, size_t return_value) {
  int ch, n, x, y, code = OK;
  switch (args.size) {
    case 2:
      ch = GetLongData(args.index[1]);
      n = GetLongData(args.index[2]);
      break;
    case 4:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      ch = GetLongData(args.index[3]);
      n = GetLongData(args.index[4]);
      code =
          wmove(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowVline", "vline requires 2 or 4 arguments");
  }
  if (code != ERR)
    wvline(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch, n);
  SetLongData(return_value, aqstl_CursesCheckERR(code, "wmove"));
}

void aqstl_CursesWindowErase(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowErase", "Unsupported argument count.");
  werase(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowDeleteLine(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowDeleteLine", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wdeleteln(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "wdeleteln"));
}

void aqstl_CursesWindowInsertLine(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowInsertLine", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          winsertln(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "winsertln"));
}

void aqstl_CursesWindowGetYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowGetYX", "Unsupported argument count.");
  getyx(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);

  struct Object* return_array = calloc(1, sizeof(struct Object));
  if (return_array == NULL) EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  return_array->type = calloc(1, sizeof(uint8_t));
  if (return_array->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  *(return_array->type) = 0x06;
  return_array->const_type = false;
  return_array->data.ptr_data = calloc(2, sizeof(struct Object*));
  if (return_array->data.ptr_data == NULL)
    EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  return_array->data.ptr_data->type = calloc(1, sizeof(uint8_t));
  if (return_array->data.ptr_data->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  *(return_array->data.ptr_data->type) = 0x02;
  return_array->data.ptr_data->const_type = false;
  return_array->data.ptr_data->data.long_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.long_data = x;
  SetReferenceData(return_value, return_array);
}

void aqstl_CursesWindowGetBegYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowGetYX", "Unsupported argument count.");
  getbegyx(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);

  struct Object* return_array = calloc(1, sizeof(struct Object));
  if (return_array == NULL) EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  return_array->type = calloc(1, sizeof(uint8_t));
  if (return_array->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetBegYX", "calloc failed");
  *(return_array->type) = 0x06;
  return_array->const_type = false;
  return_array->data.ptr_data = calloc(2, sizeof(struct Object*));
  if (return_array->data.ptr_data == NULL)
    EXIT_VM("aqstl_CursesWindowGetBegYX", "calloc failed");
  return_array->data.ptr_data->type = calloc(1, sizeof(uint8_t));
  if (return_array->data.ptr_data->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetBegYX", "calloc failed");
  *(return_array->data.ptr_data->type) = 0x02;
  return_array->data.ptr_data->const_type = false;
  return_array->data.ptr_data->data.long_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetBegYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.long_data = x;
  SetReferenceData(return_value, return_array);
}

void aqstl_CursesWindowGetMaxYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "Unsupported argument count.");
  getmaxyx(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);

  struct Object* return_array = calloc(1, sizeof(struct Object));
  if (return_array == NULL) EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  return_array->type = calloc(1, sizeof(uint8_t));
  if (return_array->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "calloc failed");
  *(return_array->type) = 0x06;
  return_array->const_type = false;
  return_array->data.ptr_data = calloc(2, sizeof(struct Object*));
  if (return_array->data.ptr_data == NULL)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "calloc failed");
  return_array->data.ptr_data->type = calloc(1, sizeof(uint8_t));
  if (return_array->data.ptr_data->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "calloc failed");
  *(return_array->data.ptr_data->type) = 0x02;
  return_array->data.ptr_data->const_type = false;
  return_array->data.ptr_data->data.long_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.long_data = x;
  SetReferenceData(return_value, return_array);
}

void aqstl_CursesWindowClear(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowClear", "Unsupported argument count.");
  wclear(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowClearToBottom(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowClearToBottom", "Unsupported argument count.");
  wclrtobot(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowClearToEOL(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowClearToEOL", "Unsupported argument count.");
  wclrtoeol(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowScroll(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowScroll", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          scroll(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "scroll"));
}

void aqstl_CursesWindowTouchWin(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesWindowTouchWin", "Unsupported argument count.");
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          touchwin(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "touchwin"));
}

void aqstl_CursesWindowTouchLine(InternalObject args, size_t return_value) {
  int st, cnt;
  st = GetLongData(args.index[1]);
  cnt = GetLongData(args.index[2]);
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          touchline(*(WINDOW**)object_table[args.index[0]].data.origin_data, st,
                    cnt),
          "touchline"));
}

void aqstl_CursesWindowGetCh(InternalObject args, size_t return_value) {
  int x, y;
  int rtn;

  switch (args.size) {
    case 0:
      rtn = wgetch(*(WINDOW**)object_table[args.index[0]].data.origin_data);
      break;
    case 2:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      rtn = mvwgetch(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                     x);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowGetCh", "getch requires 0 or 2 arguments");
  }

  SetLongData(return_value, rtn);
}

void aqstl_CursesWindowGetStr(InternalObject args, size_t return_value) {
  int x, y;
  char* rtn = (char*)calloc(1024, sizeof(char));
  int rtn2;

  switch (args.size) {
    case 0:
      rtn2 =
          wgetstr(*(WINDOW**)object_table[args.index[0]].data.origin_data, rtn);
      break;
    case 2:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      rtn2 = mvwgetstr(*(WINDOW**)object_table[args.index[0]].data.origin_data,
                       y, x, rtn);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowGetStr", "getstr requires 0 or 2 arguments");
  }

  if (rtn2 == ERR) rtn[0] = 0;
  SetStringData(return_value, rtn);
}

void aqstl_CursesWindowInCh(InternalObject args, size_t return_value) {
  int x, y, rtn;

  switch (args.size) {
    case 0:
      rtn = winch(*(WINDOW**)object_table[args.index[0]].data.origin_data);
      break;
    case 2:
      y = GetLongData(args.index[1]);
      x = GetLongData(args.index[2]);
      rtn = mvwinch(*((WINDOW**)object_table[args.index[0]].data.origin_data), y,
                   x);
      break;
    default:
      EXIT_VM("aqstl_CursesWindowInCh", "inch requires 0 or 2 arguments");
  }

  SetLongData(return_value, rtn);
}

void aqstl_CursesWindowClearOk(InternalObject args, size_t return_value) {
  int val = GetLongData(args.index[1]);
  clearok(*(WINDOW**)object_table[args.index[0]].data.origin_data, val);
}

void aqstl_CursesWindowIdlOk(InternalObject args, size_t return_value) {
  int val = GetLongData(args.index[1]);
  idlok(*(WINDOW**)object_table[args.index[0]].data.origin_data, val);
}

void aqstl_CursesWindowLeaveOk(InternalObject args, size_t return_value) {
  int val = GetLongData(args.index[1]);
  leaveok(*(WINDOW**)object_table[args.index[0]].data.origin_data, val);
}

void aqstl_CursesWindowScrollOk(InternalObject args, size_t return_value) {
  int val = GetLongData(args.index[1]);
  scrollok(*(WINDOW**)object_table[args.index[0]].data.origin_data, val);
}

void aqstl_CursesWindowSetScrollRegion(InternalObject args,
                                       size_t return_value) {
  int y = GetLongData(args.index[1]);
  int x = GetLongData(args.index[2]);
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wsetscrreg(*(WINDOW**)object_table[args.index[0]].data.origin_data, y,
                     x),
          "wsetscrreg"));
}

void aqstl_CursesWindowKeyPad(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  keypad(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesWindowNoDelay(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  nodelay(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesWindowNoTimeout(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  notimeout(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch);
}

void aqstl_CursesInitScr(InternalObject args, size_t return_value) {
  WINDOW* win;
  if (args.size != 1)
    EXIT_VM("aqstl_CursesInitScr", "initscr requires 1 argument");
  if (curses_initialised == true) {
    wrefresh(stdscr);
    SetReferenceData(return_value, aqstl_CursesWindowNew(stdscr));
  }

  win = initscr();
  if (win == NULL) {
    EXIT_VM("aqstl_CursesInitScr", "curses function returned NULL");
  }

  curses_initialised = true;

  SetReferenceData(return_value, aqstl_CursesWindowNew(win));
}

void aqstl_CursesEndWin(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesEndWin", "endwin requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(endwin(), "endwin"));
}

void aqstl_CursesIsEndWin(InternalObject args, size_t return_value) {
  if (args.size != 1)
    EXIT_VM("aqstl_CursesIsEndWin", "isendwin requires 1 argument");
  if (isendwin() == false) {
    SetByteData(return_value, 0);
  }
  SetByteData(return_value, 1);
}

void aqstl_CursesDoUpdate(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesDoUpdate", "doupdate requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(doupdate(), "doupdate"));
}

void aqstl_CursesNewWindow(InternalObject args, size_t return_value) {
  WINDOW* win;
  int nlines, ncols, begin_y, begin_x;

  if (!aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNewWindow", "newwin requires initialisation");
  nlines = ncols = 0;
  switch (args.size) {
    case 2:
      begin_y = GetLongData(args.index[1]);
      begin_x = GetLongData(args.index[2]);
      break;
    case 4:
      nlines = GetLongData(args.index[1]);
      ncols = GetLongData(args.index[2]);
      begin_y = GetLongData(args.index[3]);
      begin_x = GetLongData(args.index[4]);
      break;
    default:
      EXIT_VM("aqstl_CursesNewWindow", "newwin requires 2 or 4 arguments");
  }

  win = newwin(nlines, ncols, begin_y, begin_x);
  if (win == NULL) {
    EXIT_VM("aqstl_CursesNewWindow", "curses function returned NULL");
  }

  SetReferenceData(return_value, aqstl_CursesWindowNew(win));
}

void aqstl_CursesBeep(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesBeep", "beep requires 1 argument");
  beep();
}

void aqstl_CursesFlash(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesFlash", "flash requires 1 argument");
  flash();
}

void aqstl_CursesUngetCh(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value, aqstl_CursesCheckERR(ungetch(ch), "ungetch"));
}

void aqstl_CursesFlushInp(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesFlushInp", "flushinp requires 1 argument");
  flushinp();
}

void aqstl_CursesCBreak(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesCBreak", "cbreak requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(cbreak(), "cbreak"));
}

void aqstl_CursesNoCBreak(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNoCBreak", "nocbreak requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(nocbreak(), "nocbreak"));
}

void aqstl_CursesEcho(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesEcho", "echo requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(echo(), "echo"));
}

void aqstl_CursesNoEcho(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNoEcho", "noecho requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(noecho(), "noecho"));
}

void aqstl_CursesNl(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNl", "nl requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(nl(), "nl"));
}

void aqstl_CursesNoNl(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNoNl", "nonl requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(nonl(), "nonl"));
}

void aqstl_CursesRaw(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesRaw", "raw requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(raw(), "raw"));
}

void aqstl_CursesNoRaw(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())
    EXIT_VM("aqstl_CursesNoRaw", "noraw requires 1 argument");
  SetLongData(return_value, aqstl_CursesCheckERR(noraw(), "noraw"));
}

void aqstl_CursesIntrFlush(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value,
              aqstl_CursesCheckERR(intrflush(NULL, ch), "intrflush"));
}

void aqstl_CursesMeta(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value, aqstl_CursesCheckERR(meta(stdscr, ch), "meta"));
}

void aqstl_CursesKeyName(InternalObject args, size_t return_value) {
  const char* knp;
  int ch = GetLongData(args.index[1]);
  knp = keyname(ch);
  SetStringData(return_value, (knp == NULL) ? "" : knp);
}

#endif

void aqstl_acos(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_acos", "acos requires 1 argument");
  SetDoubleData(return_value, acos(GetDoubleData(args.index[0])));
}

void aqstl_asin(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_asin", "asin requires 1 argument");
  SetDoubleData(return_value, asin(GetDoubleData(args.index[0])));
}

void aqstl_atan(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_atan", "atan requires 1 argument");
  SetDoubleData(return_value, atan(GetDoubleData(args.index[0])));
}

void aqstl_atan2(InternalObject args, size_t return_value) {
  if (args.size != 2) EXIT_VM("aqstl_atan2", "atan2 requires 2 arguments");
  SetDoubleData(return_value, atan2(GetDoubleData(args.index[0]),
                                    GetDoubleData(args.index[2])));
}

void aqstl_ceil(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_ceil", "ceil requires 1 argument");
  SetDoubleData(return_value, ceil(GetDoubleData(args.index[0])));
}

void aqstl_cos(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_cos", "cos requires 1 argument");
  SetDoubleData(return_value, cos(GetDoubleData(args.index[0])));
}

void aqstl_cosh(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_cosh", "cosh requires 1 argument");
  SetDoubleData(return_value, cosh(GetDoubleData(args.index[0])));
}

void aqstl_exp(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_exp", "exp requires 1 argument");
  SetDoubleData(return_value, exp(GetDoubleData(args.index[0])));
}

void aqstl_fabs(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_fabs", "fabs requires 1 argument");
  SetDoubleData(return_value, fabs(GetDoubleData(args.index[0])));
}

void aqstl_floor(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_floor", "floor requires 1 argument");
  SetDoubleData(return_value, floor(GetDoubleData(args.index[0])));
}

void aqstl_fmod(InternalObject args, size_t return_value) {
  if (args.size != 2) EXIT_VM("aqstl_fmod", "fmod requires 2 arguments");
  SetDoubleData(return_value, fmod(GetDoubleData(args.index[0]),
                                   GetDoubleData(args.index[2])));
}

void aqstl_frexp(InternalObject args, size_t return_value) {
  int exp;
  if (args.size != 2) EXIT_VM("aqstl_frexp", "frexp requires 2 arguments");
  SetDoubleData(return_value, frexp(GetDoubleData(args.index[0]), &exp));
  SetLongData(args.index[1], exp);
}

void aqstl_hypot(InternalObject args, size_t return_value) {
  if (args.size != 2) EXIT_VM("aqstl_hypot", "hypot requires 2 arguments");
  SetDoubleData(return_value, hypot(GetDoubleData(args.index[0]),
                                    GetDoubleData(args.index[1])));
}

void aqstl_ldexp(InternalObject args, size_t return_value) {
  if (args.size != 2) EXIT_VM("aqstl_ldexp", "ldexp requires 2 arguments");
  SetDoubleData(return_value, ldexp(GetDoubleData(args.index[0]),
                                    GetDoubleData(args.index[1])));
}

void aqstl_log(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_log", "log requires 1 argument");
  SetDoubleData(return_value, log(GetDoubleData(args.index[0])));
}

void aqstl_log10(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_log10", "log10 requires 1 argument");
  SetDoubleData(return_value, log10(GetDoubleData(args.index[0])));
}

void aqstl_modf(InternalObject args, size_t return_value) {
  double x;
  if (args.size != 2) EXIT_VM("aqstl_modf", "modf requires 2 arguments");
  SetDoubleData(return_value, modf(GetDoubleData(args.index[0]), &x));
}

void aqstl_pow(InternalObject args, size_t return_value) {
  if (args.size != 2) EXIT_VM("aqstl_pow", "pow requires 2 arguments");
  SetDoubleData(return_value, pow(GetDoubleData(args.index[0]),
                                  GetDoubleData(args.index[1])));
}

void aqstl_sin(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_sin", "sin requires 1 argument");
  SetDoubleData(return_value, sin(GetDoubleData(args.index[0])));
}

void aqstl_sinh(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_sinh", "sinh requires 1 argument");
  SetDoubleData(return_value, sinh(GetDoubleData(args.index[0])));
}

void aqstl_sqrt(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_sqrt", "sqrt requires 1 argument");
  SetDoubleData(return_value, sqrt(GetDoubleData(args.index[0])));
}

void aqstl_tan(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_tan", "tan requires 1 argument");
  SetDoubleData(return_value, tan(GetDoubleData(args.index[0])));
}

void aqstl_tanh(InternalObject args, size_t return_value) {
  if (args.size != 1) EXIT_VM("aqstl_tanh", "tanh requires 1 argument");
  SetDoubleData(return_value, tanh(GetDoubleData(args.index[0])));
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

void AddFuncToNameTable(const char* name, func_ptr func) {
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
  AddFuncToNameTable("__builtin_abs", aqstl_abs);

#ifdef __unix__
  AddFuncToNameTable("__builtin_curses_refresh", aqstl_CursesWindowRefresh);
  AddFuncToNameTable("__builtin_curses_nooutrefresh",
                     aqstl_CursesWindowNoOutRefresh);
  AddFuncToNameTable("__builtin_curses_mvwin", aqstl_CursesWindowMoveWin);
  AddFuncToNameTable("__builtin_curses_move", aqstl_CursesWindowMove);
  AddFuncToNameTable("__builtin_curses_subwin", aqstl_CursesWindowSubWin);
  AddFuncToNameTable("__builtin_curses_addch", aqstl_CursesWindowAddCh);
  AddFuncToNameTable("__builtin_curses_insch", aqstl_CursesWindowInsCh);
  AddFuncToNameTable("__builtin_curses_delch", aqstl_CursesWindowDelCh);
  AddFuncToNameTable("__builtin_curses_echochar", aqstl_CursesWindowEchoChar);
  AddFuncToNameTable("__builtin_curses_addstr", aqstl_CursesWindowAddStr);
  AddFuncToNameTable("__builtin_curses_attron", aqstl_CursesWindowAttrOn);
  AddFuncToNameTable("__builtin_curses_attroff", aqstl_CursesWindowAttrOff);
  AddFuncToNameTable("__builtin_curses_attrset", aqstl_CursesWindowAttrSet);
  AddFuncToNameTable("__builtin_curses_standend", aqstl_CursesWindowStandEnd);
  AddFuncToNameTable("__builtin_curses_standout", aqstl_CursesWindowStandOut);
  AddFuncToNameTable("__builtin_curses_border", aqstl_CursesWindowBorder);
  AddFuncToNameTable("__builtin_curses_box", aqstl_CursesWindowBox);
  AddFuncToNameTable("__builtin_curses_hline", aqstl_CursesWindowHline);
  AddFuncToNameTable("__builtin_curses_vline", aqstl_CursesWindowVline);
  AddFuncToNameTable("__builtin_curses_erase", aqstl_CursesWindowErase);
  AddFuncToNameTable("__builtin_curses_deleteln", aqstl_CursesWindowDeleteLine);
  AddFuncToNameTable("__builtin_curses_insertln", aqstl_CursesWindowInsertLine);
  AddFuncToNameTable("__builtin_curses_getyx", aqstl_CursesWindowGetYX);
  AddFuncToNameTable("__builtin_curses_getbegyx", aqstl_CursesWindowGetBegYX);
  AddFuncToNameTable("__builtin_curses_getmaxyx", aqstl_CursesWindowGetMaxYX);
  AddFuncToNameTable("__builtin_curses_clear", aqstl_CursesWindowClear);
  AddFuncToNameTable("__builtin_curses_clrtobot",
                     aqstl_CursesWindowClearToBottom);
  AddFuncToNameTable("__builtin_curses_clrtoeol", aqstl_CursesWindowClearToEOL);
  AddFuncToNameTable("__builtin_curses_scroll", aqstl_CursesWindowScroll);
  AddFuncToNameTable("__builtin_curses_touchwin", aqstl_CursesWindowTouchWin);
  AddFuncToNameTable("__builtin_curses_touchline", aqstl_CursesWindowTouchLine);
  AddFuncToNameTable("__builtin_curses_getch", aqstl_CursesWindowGetCh);
  AddFuncToNameTable("__builtin_curses_getstr", aqstl_CursesWindowGetStr);
  AddFuncToNameTable("__builtin_curses_inch", aqstl_CursesWindowInCh);
  AddFuncToNameTable("__builtin_curses_clearok", aqstl_CursesWindowClearOk);
  AddFuncToNameTable("__builtin_curses_idlok", aqstl_CursesWindowIdlOk);
  AddFuncToNameTable("__builtin_curses_leaveok", aqstl_CursesWindowLeaveOk);
  AddFuncToNameTable("__builtin_curses_scrollok", aqstl_CursesWindowScrollOk);
  AddFuncToNameTable("__builtin_curses_setscrreg",
                     aqstl_CursesWindowSetScrollRegion);
  AddFuncToNameTable("__builtin_curses_keypad", aqstl_CursesWindowKeyPad);
  AddFuncToNameTable("__builtin_curses_nodelay", aqstl_CursesWindowNoDelay);
  AddFuncToNameTable("__builtin_curses_notimeout", aqstl_CursesWindowNoTimeout);
  AddFuncToNameTable("__builtin_curses_initscr", aqstl_CursesInitScr);
  AddFuncToNameTable("__builtin_curses_endwin", aqstl_CursesEndWin);
  AddFuncToNameTable("__builtin_curses_isendwin", aqstl_CursesIsEndWin);
  AddFuncToNameTable("__builtin_curses_doupdate", aqstl_CursesDoUpdate);
  AddFuncToNameTable("__builtin_curses_newwin", aqstl_CursesNewWindow);
  AddFuncToNameTable("__builtin_curses_beep", aqstl_CursesBeep);
  AddFuncToNameTable("__builtin_curses_flash", aqstl_CursesFlash);
  AddFuncToNameTable("__builtin_curses_ungetch", aqstl_CursesUngetCh);
  AddFuncToNameTable("__builtin_curses_flushinp", aqstl_CursesFlushInp);
  AddFuncToNameTable("__builtin_curses_cbreak", aqstl_CursesCBreak);
  AddFuncToNameTable("__builtin_curses_nocbreak", aqstl_CursesNoCBreak);
  AddFuncToNameTable("__builtin_curses_echo", aqstl_CursesEcho);
  AddFuncToNameTable("__builtin_curses_noecho", aqstl_CursesNoEcho);
  AddFuncToNameTable("__builtin_curses_nl", aqstl_CursesNl);
  AddFuncToNameTable("__builtin_curses_nonl", aqstl_CursesNoNl);
  AddFuncToNameTable("__builtin_curses_raw", aqstl_CursesRaw);
  AddFuncToNameTable("__builtin_curses_noraw", aqstl_CursesNoRaw);
  AddFuncToNameTable("__builtin_curses_intrflush", aqstl_CursesIntrFlush);
  AddFuncToNameTable("__builtin_curses_meta", aqstl_CursesMeta);
  AddFuncToNameTable("__builtin_curses_keyname", aqstl_CursesKeyName);
#endif

  AddFuncToNameTable("__builtin_math_acos", aqstl_acos);
  AddFuncToNameTable("__builtin_math_asin", aqstl_asin);
  AddFuncToNameTable("__builtin_math_atan", aqstl_atan);
  AddFuncToNameTable("__builtin_math_atan2", aqstl_atan2);
  AddFuncToNameTable("__builtin_math_ceil", aqstl_ceil);
  AddFuncToNameTable("__builtin_math_cos", aqstl_cos);
  AddFuncToNameTable("__builtin_math_cosh", aqstl_cosh);
  AddFuncToNameTable("__builtin_math_exp", aqstl_exp);
  AddFuncToNameTable("__builtin_math_fabs", aqstl_fabs);
  AddFuncToNameTable("__builtin_math_floor", aqstl_floor);
  AddFuncToNameTable("__builtin_math_fmod", aqstl_fmod);
  AddFuncToNameTable("__builtin_math_frexp", aqstl_frexp);
  AddFuncToNameTable("__builtin_math_hypot", aqstl_hypot);
  AddFuncToNameTable("__builtin_math_ldexp", aqstl_ldexp);
  AddFuncToNameTable("__builtin_math_log", aqstl_log);
  AddFuncToNameTable("__builtin_math_log10", aqstl_log10);
  AddFuncToNameTable("__builtin_math_modf", aqstl_modf);
  AddFuncToNameTable("__builtin_math_pow", aqstl_pow);
  AddFuncToNameTable("__builtin_math_sin", aqstl_sin);
  AddFuncToNameTable("__builtin_math_sinh", aqstl_sinh);
  AddFuncToNameTable("__builtin_math_sqrt", aqstl_sqrt);
  AddFuncToNameTable("__builtin_math_tan", aqstl_tan);
  AddFuncToNameTable("__builtin_math_tanh", aqstl_tanh);
}