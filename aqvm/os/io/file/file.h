// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_OS_IO_FILE_FILE_H_
#define AQ_AQVM_OS_IO_FILE_FILE_H_

#include "aqvm/os/io/file/file.h"

// Read a file and return the contents as a char buffer. Returns 0 for success,
// other values for errors.
int AqvmOsIoFile_ReadFile(const char *filename, char *buffer);

// Write a file with the contents of a char buffer. Returns 0 for success, other
// values for errors.
int AqvmOsIoFile_WriteFile(const char *filename, char *buffer);

#endif