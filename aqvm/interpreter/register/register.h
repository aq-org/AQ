// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_
#define AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_

enum AqvmInterpreterRegister_ValueType {
  // TODO(Register): Waiting for the improvement of the register.
  AqvmInterpreterRegisterValueType_INT,
  AqvmInterpreterRegisterValueType_FLOAT,
  AqvmInterpreterRegisterValueType_DOUBLE,
  AqvmInterpreterRegisterValueType_LONG,
  AqvmInterpreterRegisterValueType_CHARACTER,
  AqvmInterpreterRegisterValueType_BOOLEAN
};

union AqvmInterpreterRegister_Value {
  // TODO(Register): Waiting for the improvement of the register.
  int int_value;
  float float_value;
  double double_value;
  long long_value;
  char character_value;
  bool boolean_value;
};

struct AqvmInterpreterRegister_Register {
  enum AqvmInterpreterRegister_ValueType type;
  union AqvmInterpreterRegister_Value value;
};

#endif
