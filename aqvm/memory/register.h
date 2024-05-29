// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_MEMORY_REGISTER_H_
#define AQ_AQVM_MEMORY_REGISTER_H_

enum AqvmMemoryRegister_ValueType {
  // TODO(Register): Waiting for the improvement of the register.
  AqvmMemoryRegisterValueType_INT,
  AqvmMemoryRegisterValueType_FLOAT,
  AqvmMemoryRegisterValueType_DOUBLE,
  AqvmMemoryRegisterValueType_LONG,
  AqvmMemoryRegisterValueType_CHARACTER,
  AqvmMemoryRegisterValueType_BOOLEAN
};

union AqvmMemoryRegister_Value {
  // TODO(Register): Waiting for the improvement of the register.
  int int_value;
  float float_value;
  double double_value;
  long long_value;
  char character_value;
  bool boolean_value;
};

struct AqvmMemoryRegister_Register {
  enum AqvmMemoryRegister_ValueType type;
  union AqvmMemoryRegister_Value value;
  bool const_status;
};

#endif
