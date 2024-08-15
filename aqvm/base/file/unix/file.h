#ifdef __unix__
// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_BASE_FILE_UNIX_FILE_H_
#define AQ_AQVM_BASE_FILE_UNIX_FILE_H_

#include <sys/stat.h>

#include "aqvm/base/file/file.h"

struct stat AqvmBaseFileUnix_ConvertFileToStat(
    const struct AqvmBaseFile_File* file);

#endif
#endif