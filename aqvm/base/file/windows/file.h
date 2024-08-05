#ifdef _WIN32
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_WINDOWS_FILE_H_
#define AQ_AQVM_BASE_FILE_WINDOWS_FILE_H_

#include <stdio.h>
#include <Windows.h>

#include "aqvm/base/file/file.h"

HANDLE AqvmBaseFileWindows_ConvertFileToHandle(struct AqvmBaseFile_File* file);

// fopen freopen setbuf tmpfile tmpnam vsprintf

#endif
#endif