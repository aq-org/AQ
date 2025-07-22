// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "generator/builtin.h"

#include <string>

#include "generator/generator.h"

namespace Aq {
namespace Generator {
void AddBuiltInFunctionDeclaration(Generator& generator, std::string name) {
  generator.context.functions.insert(name);
}

void InitBuiltInFunctionDeclaration(Generator& generator) {
  AddBuiltInFunctionDeclaration(generator, "__builtin_print");
  AddBuiltInFunctionDeclaration(generator, "__builtin_vaprint");
  AddBuiltInFunctionDeclaration(generator, "__builtin_abs");
  AddBuiltInFunctionDeclaration(generator, "__builtin_open");
  AddBuiltInFunctionDeclaration(generator, "__builtin_close");
  AddBuiltInFunctionDeclaration(generator, "__builtin_read");
  AddBuiltInFunctionDeclaration(generator, "__builtin_write");
  AddBuiltInFunctionDeclaration(generator, "__builtin_input");

  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_refresh");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_nooutrefresh");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_mvwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_move");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_subwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_addch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_insch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_delch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_echochar");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_addstr");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_attron");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_attroff");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_attrset");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_standend");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_standout");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_border");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_box");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_hline");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_vline");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_erase");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_deleteln");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_insertln");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_getyx");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_getbegyx");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_getmaxyx");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_clear");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_clrtobot");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_clrtoeol");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_scroll");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_touchwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_touchline");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_getch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_getstr");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_inch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_clearok");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_idlok");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_leaveok");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_scrollok");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_setscrreg");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_keypad");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_nodelay");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_notimeout");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_initscr");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_endwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_isendwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_doupdate");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_newwin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_beep");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_flash");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_ungetch");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_flushinp");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_cbreak");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_nocbreak");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_echo");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_noecho");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_nl");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_nonl");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_raw");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_noraw");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_intrflush");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_meta");
  AddBuiltInFunctionDeclaration(generator, "__builtin_curses_keyname");

  AddBuiltInFunctionDeclaration(generator, "__builtin_GUI_CreateWindow");

  AddBuiltInFunctionDeclaration(generator, "__builtin_math_acos");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_asin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_atan");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_atan2");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_ceil");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_cos");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_cosh");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_exp");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_fabs");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_floor");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_fmod");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_frexp");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_hypot");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_ldexp");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_log");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_log10");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_modf");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_pow");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_sin");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_sinh");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_sqrt");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_tan");
  AddBuiltInFunctionDeclaration(generator, "__builtin_math_tanh");
}
}  // namespace Generator
}  // namespace Aq
