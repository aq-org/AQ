// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_REGISTER_H_
#define AQ_AQVM_MEMORY_REGISTER_H_

#include <stdbool.h>

#include "aqvm/memory/types.h"

/*
// plan 1:
struct register{
  uint8_t type;
  void* value_ptr;
};
void* value;
register array[];

// plan 2:
void* value;
// value to the memory address of index 0 is int, the index 0 to the index 1 is
// float, etc.
size_t type[];
*/

enum AqvmMemoryRegister_ValueType {
  // TODO(Register): Waiting for the improvement of the register.
  AqvmMemoryRegisterValueType_INT,
  AqvmMemoryRegisterValueType_CONSTINT,
  AqvmMemoryRegisterValueType_FLOAT,
  AqvmMemoryRegisterValueType_CONSTFLOAT,
  AqvmMemoryRegisterValueType_DOUBLE,
  AqvmMemoryRegisterValueType_CONSTDOUBLE,
  AqvmMemoryRegisterValueType_LONG,
  AqvmMemoryRegisterValueType_CONSTLONG,
  AqvmMemoryRegisterValueType_CHARACTER,
  AqvmMemoryRegisterValueType_CONSTCHARACTER,
  AqvmMemoryRegisterValueType_BOOLEAN,
  AqvmMemoryRegisterValueType_CONSTBOOLEAN
};

union AqvmMemoryRegister_Value {
  // TODO(Register): Waiting for the improvement of the register.
  int int_value;
  const int const_int_value;
  float float_value;
  const float const_float_value;
  double double_value;
  const double const_double_value;
  long long_value;
  const long const_long_value;
  char character_value;
  const char const_character_value;
  bool boolean_value;
  const bool const_boolean_value;
};

struct AqvmMemoryRegister_Register {
  enum AqvmMemoryRegister_ValueType type;
  union AqvmMemoryRegister_Value value;
};

/*
struct AqvmMemoryRegister_Register {
  int* value;
  size_t size;
};

int AqvmMemoryRegister_IntStore(int value,
                                struct AqvmMemoryRegister_Register* register,
                                size_t index) {
  if (register->size < index * 4) {
    // TODO(ERROR): Out of Memory.
    return -1;
  }
  register->value[index] = value;
  return 0;
}
int AqvmMemoryRegister_FloatStore(float value,
                                  struct AqvmMemoryRegister_Register* register,
                                  size_t index) {
  if (register->size < index * 4) {
    // TODO(ERROR): Out of Memory.
    return -1;
  }
  register->value[index] = (int)value;
  return 0;
}

// Wait for development.
*/

#endif
