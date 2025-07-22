// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/builtin.h"

#include <string>

#include "interpreter/interpreter.h"

namespace Aq {
namespace Interpreter {
void AddBuiltInFunctionDeclaration(Interpreter& interpreter, std::string name) {
  interpreter.context.functions.insert(name);
}

void InitBuiltInFunctionDeclaration(Interpreter& interpreter) {
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_print");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_vaprint");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_abs");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_open");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_close");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_read");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_write");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_input");

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_refresh");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_nooutrefresh");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_mvwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_move");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_subwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_addch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_insch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_delch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_echochar");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_addstr");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_attron");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_attroff");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_attrset");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_standend");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_standout");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_border");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_box");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_hline");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_vline");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_erase");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_deleteln");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_insertln");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_getyx");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_getbegyx");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_getmaxyx");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_clear");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_clrtobot");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_clrtoeol");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_scroll");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_touchwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_touchline");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_getch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_getstr");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_inch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_clearok");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_idlok");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_leaveok");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_scrollok");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_setscrreg");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_keypad");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_nodelay");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_notimeout");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_initscr");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_endwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_isendwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_doupdate");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_newwin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_beep");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_flash");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_ungetch");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_flushinp");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_cbreak");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_nocbreak");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_echo");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_noecho");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_nl");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_nonl");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_raw");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_noraw");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_intrflush");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_meta");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_curses_keyname");

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_GUI_CreateWindow");

  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_acos");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_asin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_atan2");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ceil");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cos");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_cosh");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_exp");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fabs");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_floor");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_fmod");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_frexp");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_hypot");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_ldexp");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_log10");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_modf");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_pow");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sin");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sinh");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_sqrt");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tan");
  AddBuiltInFunctionDeclaration(interpreter, "__builtin_math_tanh");
}
}  // namespace Interpreter
}  // namespace Aq
