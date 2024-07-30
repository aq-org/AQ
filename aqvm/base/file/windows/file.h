// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_WINDOWS_FILE_H_
#define AQ_AQVM_BASE_FILE_WINDOWS_FILE_H_

#include <stdio.h>
#include <windows.h>

HANDLE AqvmBaseFileWindows_FileToHandle(FILE* file);

#endif