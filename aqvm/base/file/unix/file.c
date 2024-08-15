#ifdef __unix__
#include "aqvm/base/file/windows/file.h"

#include <sys/stat.h>
#include <unistd.h>

#include "aqvm/base/file/file.h"

struct stat AqvmBaseFileUnix_ConvertFileToStat(
    const struct AqvmBaseFile_File* file) {
  struct stat st;
  if (fstat(_fileno(file->file), &st) == -1) {
    // TODO(logging)
  }
  return st;
}
#endif