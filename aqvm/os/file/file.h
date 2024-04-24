// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_OS_FILE_FILE_H_
#define AQ_AQVM_OS_FILE_FILE_H_

// Read a file and return the contents as a char buffer.
int AqvmOsFile_ReadFile(char *filename, char **buffer);

#endif