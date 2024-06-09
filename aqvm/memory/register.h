// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_REGISTER_H_
#define AQ_AQVM_MEMORY_REGISTER_H_

#include <stdbool.h>

#include "aqvm/memory/types.h"

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
