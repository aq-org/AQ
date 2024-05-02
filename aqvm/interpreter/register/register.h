// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_
#define AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_

struct AqvmInterpreterRegister_Register{
AqvmInterpreterRegister_ValueType type;
AqvmInterpreterRegister_Value value;

// TODO(Register): Wait developing some functions for the register.

};

enum AqvmInterpreterRegister_ValueType {
// TODO(Register): Waiting for the implementation of the register.
};

union AqvmInterpreterRegister_Value{
// TODO(Register): Waiting for the implementation of the register.
};

#endif
