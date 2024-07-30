#include "aqvm/base/file/windows/file.h"

#include <io.h>
#include <stdio.h>
#include <windows.h>

HANDLE AqvmBaseFileWindows_FileToHandle(FILE* file) {
  HANDLE handle = (HANDLE)_get_osfhandle(_fileno(file));
  if (handle == INVALID_HANDLE_VALUE) {
    // TODO
  }
  return handle;
}