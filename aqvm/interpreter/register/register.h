// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_
#define AQ_AQVM_INTERPRETER_REGISTER_REGISTER_H_

typedef struct {
enum AqvmInterpreterRegisterRegister_ValueType {
// TODO(Register): Waiting for the implementation of the register.
};
typedef union {
// TODO(Register): Waiting for the implementation of the register.
} AqvmInterpreterRegisterRegister_Value;

AqvmInterpreterRegisterRegister_ValueType type;
AqvmInterpreterRegisterRegister_Value value;

// TODO(Register): Wait developing some functions for the register.

}AqvmInterpreterRegister_Register;

#endif
