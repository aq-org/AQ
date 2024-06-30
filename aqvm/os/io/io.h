// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_OS_IO_IO_H_
#define AQ_AQVM_OS_IO_IO_H_

// Read a file and return the contents as a char buffer. Returns 0 for success,
// -1 for operating system file handler not found. other values for errors.
int AqvmOsIoFile_ReadFile(char *filename, char **buffer);

// Write a file with the contents of a char buffer. Returns 0 for success,
// -1 for operating system file handler not found. other values for errors.
int AqvmOsIoFile_WriteFile(char *filename, char *buffer);

#endif