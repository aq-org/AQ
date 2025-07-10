// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_UTILS_UTILS_H_
#define AQ_VM_UTILS_UTILS_H_

#include <cstdint>
#include <cstring>

namespace Aq {
namespace Vm {
inline int64_t SwapLong(int64_t x) {
  uint64_t ux = (uint64_t)x;
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  return (int64_t)ux;
}

inline double SwapDouble(double x) {
  uint64_t ux;
  memcpy(&ux, &x, sizeof(uint64_t));
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  double result;
  memcpy(&result, &ux, sizeof(double));
  return result;
}

inline uint64_t SwapUint64t(uint64_t x) {
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}

inline size_t DecodeUleb128(const uint8_t* input, size_t* result) {
  *result = 0;
  size_t shift = 0;
  size_t count = 0;

  while (1) {
    uint8_t byte = input[count++];
    *result |= (byte & 0x7F) << shift;
    if ((byte & 0x80) == 0) {
      break;
    }
    shift += 7;
  }

  return count;
}
}  // namespace Vm
}  // namespace Aq

#endif