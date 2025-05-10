
#ifdef HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

struct Object* curses_error;

bool curses_initialised = false;

int aqstl_CursesCheckERR(int code, char* fname) {
  char buf[100];

  if (code != ERR) {
    return 0;
  } else {
    if (fname == NULL) {
      // SetStringObjectData(curses_error, "curses function returned ERR");
      EXIT_VM("aqstl_CursesCheckERR", "curses function returned ERR");
    } else {
      strcpy(buf, fname);
      strcat(buf, "() returned ERR");
      // SetStringObjectData(curses_error, buf);
      EXIT_VM("aqstl_CursesCheckERR", buf);
    }
    return -1;
  }
}

int aqstl_CursesInitialised() {
  if (curses_initialised == true)
    return 1;
  else {
    // SetStringObjectData(curses_error, "must call initscr() first");
    EXIT_VM("aqstl_CursesInitialised", "must call initscr() first");
    return 0;
  }
}

void aqstl_CursesWindowNew(WINDOW* win) {
  struct Object* wo;

  wo = calloc(1, sizeof(struct Object));
  if (wo == NULL) return NULL;
  wo->type = calloc(1, sizeof(uint8_t));
  if (wo->type == NULL) return NULL;
  *(wo->type) = 0x0A;

  wo->data.origin_data = calloc(2, sizeof(WINDOWS*));
  if (wo->data.origin_data == NULL) return NULL;
  *(WINDOWS**)(wo->data.origin_data) = win;
  *(wo->data.origin_data + 1) = NULL;
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
  int x, y;
  x = GetLongData(args.index[1]);
  y = GetLongData(args.index[2]);
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          mvwin(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x),
          "mvwin"));
}

void aqstl_CursesWindowMove(InternalObject args, size_t return_value) {
  if (args.size != 3)
    EXIT_VM("aqstl_CursesWindowMove", "Unsupported argument count.");
  int x, y;
  x = GetLongData(args.index[1]);
  y = GetLongData(args.index[2]);
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
    // SetStringObjectData(curses_error, "curses function returned NULL");
    EXIT_VM("aqstl_CursesWindowSubWin", "curses function returned NULL");
  }
  rtn_win = aqstl_CursesWindowNew(win);
  *(((WINDOWS**)rtn_win->data.origin_data) + 1) =
      *(WINDOW**)object_table[args.index[0]].data.origin_data;
  SetReferenceDta(return_value, rtn_win);
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
      // SetStringObjectData(PyExc_TypeError, "addch requires 1 to 4
      // arguments");

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
      // SetStringObjectData(PyExc_TypeError, "insch requires 1 to 4
      // arguments");
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
      // SetStringObjectData(PyExc_TypeError, "delch requires 0 or 2
      // arguments");
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
      i = GetLongData(args.index[1]);
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
      // SetStringObjectData(PyExc_TypeError, "echochar requires 1 or 2
      // arguments");
      EXIT_VM("aqstl_CursesWindowEchoChar",
              "echochar requires 1 or 2 arguments");
  }

  SetLongData(return_value, aqstl_CursesCheckERR(rtn, "wechochar"));
}

void aqstl_CursesWindowAddStr(InternalObject args, size_t return_value) {
  int rtn;
  int x, y;
  char* str;
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
      // SetStringObjectData(PyExc_TypeError, "addstr requires 1 to 4
      // arguments");
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
  if (args.size != 1) return NULL;
  wstandend(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowStandOut(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  wstandout(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowBorder(InternalObject args, size_t return_value) {
  int ls, rs, ts, bs, tl, tr, bl, br;
  ls = rs = ts = bs = tl = tr = bl = br = 0;
  ls = GetLongData(args.index[1]);
  rs = GetLongData(args.index[2]);
  ts = GetLongData(args.index[3]);
  bs = GetLongData(args.index[4]);
  tl = GetLongData(args.index[5]);
  tr = GetLongData(args.index[6]);
  bl = GetLongData(args.index[7]);
  br = GetLongData(args.index[8]);
  wborder(*(WINDOW**)object_table[args.index[0]].data.origin_data, ls, rs, ts,
          bs, tl, tr, bl, br);
}

void aqstl_CursesWindowBox(InternalObject args, size_t return_value) {
  int ch1 = 0, ch2 = 0;
  if (args.size != 1) {
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
      // SetStringObjectData(PyExc_TypeError, "hline requires 2 or 4
      // arguments");
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
      // SetStringObjectData(PyExc_TypeError, "vline requires 2 or 4
      // arguments");
      EXIT_VM("aqstl_CursesWindowVline", "vline requires 2 or 4 arguments");
  }
  if (code != ERR)
    wvline(*(WINDOW**)object_table[args.index[0]].data.origin_data, ch, n);
  SetLongData(return_value, aqstl_CursesCheckERR(code, "wmove"));
}

void aqstl_CursesWindowErase(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  werase(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowDeleteLine(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          wdeleteln(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "wdeleteln"));
}

void aqstl_CursesWindowInsertLine(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          winsertln(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "winsertln"));
}

void aqstl_CursesWindowGetYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1) return NULL;
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
  return_array->data.ptr_data->data.int_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.int_data = x;
  SetReferenceDta(return_value, return_array);
}

void aqstl_CursesWindowGetBegYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1) return NULL;
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
  return_array->data.ptr_data->data.int_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetBegYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.int_data = x;
  SetReferenceDta(return_value, return_array);
}

void aqstl_CursesWindowGetMaxYX(InternalObject args, size_t return_value) {
  int x, y;
  if (args.size != 1) return NULL;
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
  return_array->data.ptr_data->data.int_data = y;
  (return_array->data.ptr_data + 1)->type = calloc(1, sizeof(uint8_t));
  if ((return_array->data.ptr_data + 1)->type == NULL)
    EXIT_VM("aqstl_CursesWindowGetMaxYX", "calloc failed");
  *((return_array->data.ptr_data + 1)->type) = 0x02;
  (return_array->data.ptr_data + 1)->const_type = false;
  (return_array->data.ptr_data + 1)->data.int_data = x;
  SetReferenceDta(return_value, return_array);
}

void aqstl_CursesWindowClear(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  wclear(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowClearToBottom(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  wclrtobot(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowClearToEOL(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  wclrtoeol(*(WINDOW**)object_table[args.index[0]].data.origin_data);
}

void aqstl_CursesWindowScroll(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
  SetLongData(
      return_value,
      aqstl_CursesCheckERR(
          scroll(*(WINDOW**)object_table[args.index[0]].data.origin_data),
          "scroll"));
}

void aqstl_CursesWindowTouchWin(InternalObject args, size_t return_value) {
  if (args.size != 1) return NULL;
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
      // SetStringObjectData(PyExc_TypeError, "getch requires 0 or 2
      // arguments");
      EXIT_VM("aqstl_CursesWindowGetCh", "getch requires 0 or 2 arguments");
  }

  SetLongData(return_value, rtn);
}

void aqstl_CursesWindowGetStr(InternalObject args, size_t return_value) {
  int x, y;
  char rtn = calloc(1024, sizeof(char));
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
      // SetStringObjectData(PyExc_TypeError, "getstr requires 0 or 2
      // arguments");
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
      rtn =
          mvinch(*(WINDOW**)object_table[args.index[0]].data.origin_data, y, x);
      break;
    default:
      // SetStringObjectData(PyExc_TypeError, "inch requires 0 or 2 arguments");
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

static PyMethodDef aqstl_CursesWindowMethods[] = {
    {"refresh", (PyCFunction)aqstl_CursesWindowRefresh},
    {"nooutrefresh", (PyCFunction)aqstl_CursesWindowNoOutRefresh},
    {"mvwin", (PyCFunction)aqstl_CursesWindowMoveWin},
    {"move", (PyCFunction)aqstl_CursesWindowMove},
    {"subwin", (PyCFunction)aqstl_CursesWindowSubWin},
    {"addch", (PyCFunction)aqstl_CursesWindowAddCh},
    {"insch", (PyCFunction)aqstl_CursesWindowInsCh},
    {"delch", (PyCFunction)aqstl_CursesWindowDelCh},
    {"echochar", (PyCFunction)aqstl_CursesWindowEchoChar},
    {"addstr", (PyCFunction)aqstl_CursesWindowAddStr},
    {"attron", (PyCFunction)aqstl_CursesWindowAttrOn},
    {"attroff", (PyCFunction)aqstl_CursesWindowAttrOff},
    {"attrset", (PyCFunction)aqstl_CursesWindowAttrSet},
    {"standend", (PyCFunction)aqstl_CursesWindowStandEnd},
    {"standout", (PyCFunction)aqstl_CursesWindowStandOut},
    {"border", (PyCFunction)aqstl_CursesWindowBorder, METH_VARARGS},
    {"box", (PyCFunction)aqstl_CursesWindowBox},
    {"hline", (PyCFunction)aqstl_CursesWindowHline},
    {"vline", (PyCFunction)aqstl_CursesWindowVline},
    {"erase", (PyCFunction)aqstl_CursesWindowErase},
    {"deleteln", (PyCFunction)aqstl_CursesWindowDeleteLine},
    {"insertln", (PyCFunction)aqstl_CursesWindowInsertLine},
    {"getyx", (PyCFunction)aqstl_CursesWindowGetYX},
    {"getbegyx", (PyCFunction)aqstl_CursesWindowGetBegYX},
    {"getmaxyx", (PyCFunction)aqstl_CursesWindowGetMaxYX},
    {"clear", (PyCFunction)aqstl_CursesWindowClear},
    {"clrtobot", (PyCFunction)aqstl_CursesWindowClearToBottom},
    {"clrtoeol", (PyCFunction)aqstl_CursesWindowClearToEOL},
    {"scroll", (PyCFunction)aqstl_CursesWindowScroll},
    {"touchwin", (PyCFunction)aqstl_CursesWindowTouchWin},
    {"touchline", (PyCFunction)aqstl_CursesWindowTouchLine},
    {"getch", (PyCFunction)aqstl_CursesWindowGetCh},
    {"getstr", (PyCFunction)aqstl_CursesWindowGetStr},
    {"inch", (PyCFunction)aqstl_CursesWindowInCh},
    {"clearok", (PyCFunction)aqstl_CursesWindowClearOk},
    {"idlok", (PyCFunction)aqstl_CursesWindowIdlOk},
    {"leaveok", (PyCFunction)aqstl_CursesWindowLeaveOk},
    {"scrollok", (PyCFunction)aqstl_CursesWindowScrollOk},
    {"setscrreg", (PyCFunction)aqstl_CursesWindowSetScrollRegion},
    {"keypad", (PyCFunction)aqstl_CursesWindowKeyPad},
    {"nodelay", (PyCFunction)aqstl_CursesWindowNoDelay},
    {"notimeout", (PyCFunction)aqstl_CursesWindowNoTimeout},
    {NULL, NULL}};

/*void aqstl_CursesWindowGetAttr(struct Object* self, char* name) {
  return Py_FindMethod(aqstl_CursesWindowMethods, self, name);
}*/

struct Object* ModDict;

void aqstl_CursesInitScr(InternalObject args, size_t return_value) {
  WINDOW* win;
  if (args.size != 1) EXIT_VM("aqstl_CursesInitScr", "initscr requires 1 argument");
  if (curses_initialised == true) {
    wrefresh(stdscr);
    SetReferenceData(return_value, aqstl_CursesWindowNew(stdscr));
  }

  win = initscr();
  if (win == NULL) {
    // SetStringObjectData(curses_error, "curses function returned NULL");
    EXIT_VM("aqstl_CursesInitScr", "curses function returned NULL");
  }

  curses_initialised = true;

/*#define SetDictInt(string, ch) \
  PyDict_SetItemString(ModDict, string, PyInt_FromLong((long)(ch)));

  SetDictInt("ACS_ULCORNER", (ACS_ULCORNER));
  SetDictInt("ACS_ULCORNER", (ACS_ULCORNER));
  SetDictInt("ACS_LLCORNER", (ACS_LLCORNER));
  SetDictInt("ACS_URCORNER", (ACS_URCORNER));
  SetDictInt("ACS_LRCORNER", (ACS_LRCORNER));
  SetDictInt("ACS_RTEE", (ACS_RTEE));
  SetDictInt("ACS_LTEE", (ACS_LTEE));
  SetDictInt("ACS_BTEE", (ACS_BTEE));
  SetDictInt("ACS_TTEE", (ACS_TTEE));
  SetDictInt("ACS_HLINE", (ACS_HLINE));
  SetDictInt("ACS_VLINE", (ACS_VLINE));
  SetDictInt("ACS_PLUS", (ACS_PLUS));
  SetDictInt("ACS_S1", (ACS_S1));
  SetDictInt("ACS_S9", (ACS_S9));
  SetDictInt("ACS_DIAMOND", (ACS_DIAMOND));
  SetDictInt("ACS_CKBOARD", (ACS_CKBOARD));
  SetDictInt("ACS_DEGREE", (ACS_DEGREE));
  SetDictInt("ACS_PLMINUS", (ACS_PLMINUS));
  SetDictInt("ACS_BULLET", (ACS_BULLET));
  SetDictInt("ACS_LARROW", (ACS_RARROW));
  SetDictInt("ACS_DARROW", (ACS_DARROW));
  SetDictInt("ACS_UARROW", (ACS_UARROW));
  SetDictInt("ACS_BOARD", (ACS_BOARD));
  SetDictInt("ACS_LANTERN", (ACS_LANTERN));
  SetDictInt("ACS_BLOCK", (ACS_BLOCK));*/

  SetReferenceData(return_value, aqstl_CursesWindowNew(win));
}

void aqstl_CursesEndWinInternalObject (InternalObject args, size_t return_value)
{
  if (args.size!=1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesEndWin", "endwin requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(endwin(), "endwin"));
}

void aqstl_CursesIsEndWinInternalObject(InternalObject args, size_t return_value)
{
  if (args.size!=1) return NULL;
  if (isendwin() == false) {
    SetByteData(return_value, 0);
  }
  SetByteData(return_value, 1);
}

void aqstl_CursesDoUpdate(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesDoUpdate", "doupdate requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(doupdate(), "doupdate"));
}

void aqstl_CursesNewWindow(InternalObject args, size_t return_value) {
  WINDOW* win;
  int nlines, ncols, begin_y, begin_x;

  if (!aqstl_CursesInitialised()) return NULL;
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
      // SetStringObjectData(PyExc_TypeError, "newwin requires 2 or 4
      // arguments");
      EXIT_VM("aqstl_CursesNewWindow", "newwin requires 2 or 4 arguments");
  }

  win = newwin(nlines, ncols, begin_y, begin_x);
  if (win == NULL) {
    // SetStringObjectData(curses_error, "curses function returned NULL");
    EXIT_VM("aqstl_CursesNewWindow", "curses function returned NULL")
  }

  SetReferenceData(return_value,aqstl_CursesWindowNew(win));
}

void aqstl_CursesBeep(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesBeep", "beep requires 1 argument");
  beep();
}

void aqstl_CursesFlash(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesFlash", "flash requires 1 argument");
  flash();
}

void aqstl_CursesUngetCh(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value,aqstl_CursesCheckERR(ungetch(ch), "ungetch"));
}

void aqstl_CursesFlushInp(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesFlushInp", "flushinp requires 1 argument");
  flushinp();
}

void aqstl_CursesCBreak(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesCBreak", "cbreak requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(cbreak(), "cbreak"));
}

void aqstl_CursesNoCBreak(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesNoCBreak", "nocbreak requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(nocbreak(), "nocbreak"));
}

void aqstl_CursesEcho(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesEcho", "echo requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(echo(), "echo"));
}

void aqstl_CursesNoEcho(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesNoEcho", "noecho requires 1 argument")
  SetLongData(return_value,aqstl_CursesCheckERR(noecho(), "noecho"));
}

void aqstl_CursesNl(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesNl", "nl requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(nl(), "nl"));
}

void aqstl_CursesNoNl(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesNoNl", "nonl requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(nonl(), "nonl"));
}

void aqstl_CursesRaw(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised())EXIT_VM("aqstl_CursesRaw", "raw requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(raw(), "raw"));
}

void aqstl_CursesNoRaw(InternalObject args, size_t return_value) {
  if (args.size != 1 || !aqstl_CursesInitialised()) EXIT_VM("aqstl_CursesNoRaw", "noraw requires 1 argument");
  SetLongData(return_value,aqstl_CursesCheckERR(noraw(), "noraw"));
}

void aqstl_CursesIntrFlush(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value,aqstl_CursesCheckERR(intrflush(NULL, ch), "intrflush"));
}

void aqstl_CursesMeta(InternalObject args, size_t return_value) {
  int ch = GetLongData(args.index[1]);
  SetLongData(return_value,aqstl_CursesCheckERR(meta(stdscr, ch), "meta"));
}

void aqstl_CursesKeyName(InternalObject args, size_t return_value) {
  char* knp;
  int ch = GetLongData(args.index[1]);
  knp = keyname(ch);
  SetStringData(return_value,(knp == NULL) ? "" : knp);
}


static PyMethodDef PyCurses_methods[] = {
    {"initscr", (PyCFunction)PyCurses_InitScr},
    {"endwin", (PyCFunction)PyCurses_EndWin},
    {"isendwin", (PyCFunction)PyCurses_IsEndWin},
    {"doupdate", (PyCFunction)PyCurses_DoUpdate},
    {"newwin", (PyCFunction)PyCurses_NewWindow},
    {"beep", (PyCFunction)PyCurses_Beep},
    {"flash", (PyCFunction)PyCurses_Flash},
    {"ungetch", (PyCFunction)PyCurses_UngetCh},
    {"flushinp", (PyCFunction)PyCurses_FlushInp},
    {"cbreak", (PyCFunction)PyCurses_CBreak},
    {"nocbreak", (PyCFunction)PyCurses_NoCBreak},
    {"echo", (PyCFunction)PyCurses_Echo},
    {"noecho", (PyCFunction)PyCurses_NoEcho},
    {"nl", (PyCFunction)PyCurses_Nl},
    {"nonl", (PyCFunction)PyCurses_NoNl},
    {"raw", (PyCFunction)PyCurses_Raw},
    {"noraw", (PyCFunction)PyCurses_NoRaw},
    {"intrflush", (PyCFunction)PyCurses_IntrFlush},
    {"meta", (PyCFunction)PyCurses_Meta},
    {"keyname", (PyCFunction)PyCurses_KeyName},
    {NULL, NULL}
};