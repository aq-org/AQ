// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/os/io/file/file.h"

#include <stdio.h>
#include <stdlib.h>

int AqvmOsIoFile_ReadFile(const char *filename, char *buffer) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    return -1;
  }
  buffer = malloc(1024);
  // TODO: Not complete.
}

int AqvmOsIoFile_WriteFile(const char *filename, char *buffer) {
  // TODO: Not complete.
  return -1;
}
