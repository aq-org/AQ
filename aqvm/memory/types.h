// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_TYPES_H_
#define AQ_AQVM_MEMORY_TYPES_H_

#include <stdint.h>

// null - 0x00 - null type
// The null type simply represents an unknown type or a type that is not needed
// (e.g., returns nothing). Has no length.
typedef void aqnull;

// byte - 0x01 - 1 byte (8-bit) signed integer type
// Using two's complement storage. Generally used to store bool or char. From
// -128 to 127 (-2^7 to 2^7 - 1), inclusive.
typedef int8_t aqbyte;

// int - 0x02 - 4-byte (32-bit) signed integer type
// Stored in two's complement notation. From -2147483648 to 2147483647 (-2^31 to
// 2^31 - 1), inclusive.
typedef int aqint;

// long - 0x03 - 8-byte (64-bit) signed integer type
// Stored in two's complement notation. From -9223372036854775808 to
// 9223372036854775807 (-2^63 to 2^63 - 1), inclusive.
typedef int64_t aqlong;

// float - 0x04 - 4-byte (32-bit) single-precision floating point type
// Using ISO/IEC 60559 Information technology — Microprocessor Systems —
// Floating-Point arithmetic standard.
typedef float aqfloat;

// double - 0x05 - 8-byte (64-bit) double-precision floating point type
// Using ISO/IEC 60559 Information technology — Microprocessor Systems —
// Floating-Point arithmetic standard.
typedef double aqdouble;

// The part beyond 0x05 and within 0x0F is currently designated as a reserved
// type. The part beyond 0x0F cannot be used because it exceeds the 4-bit size
// limit.

#endif