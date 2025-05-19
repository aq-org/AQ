// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <locale.h>

#ifdef _WIN32
#include <windows.h>
int aqvm_win32_init() {
  if (SetConsoleCP(CP_UTF8) == 0 || SetConsoleOutputCP(CP_UTF8) == 0) {
    return -1;
  }

  return 0;
}
#endif

int aqvm_init() {
#ifdef _WIN32
  return aqvm_win32_init();
#endif
  // POSIX系统：尝试多种UTF-8区域设置
  const char* locales[] = {"C.UTF-8", ".UTF8", "en_US.UTF-8", "UTF-8"};
  for (size_t i = 0; i < sizeof(locales)/sizeof(locales[0]); i++) {
    if (setlocale(LC_ALL, locales[i]) != NULL) {
      break; // 成功设置
    }
    if (i == sizeof(locales)/sizeof(locales[0]) - 1) { // 最后一个仍失败
      return -1;
    }
  }
  return 0;
}