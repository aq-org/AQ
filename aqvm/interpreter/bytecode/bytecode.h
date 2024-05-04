// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_BYTECODE_BYTECODE_H_
#define AQ_AQVM_INTERPRETER_BYTECODE_BYTECODE_H_

#include <stddef.h>

#include "aqvm/interpreter/register/register.h"

enum AqvmInterpreterBytecode_Type {
  AqvmInterpreterBytecodeType_NOP = 0x00,
  AqvmInterpreterBytecodeType_LDC,
  AqvmInterpreterBytecodeType_LOAD,
  AqvmInterpreterBytecodeType_STORE,
  AqvmInterpreterBytecodeType_NEW,
  AqvmInterpreterBytecodeType_FREE,
  AqvmInterpreterBytecodeType_SIZE,
  AqvmInterpreterBytecodeType_ADD,
  AqvmInterpreterBytecodeType_SUB,
  AqvmInterpreterBytecodeType_MUL,
  AqvmInterpreterBytecodeType_DIV,
  AqvmInterpreterBytecodeType_MOD,
  AqvmInterpreterBytecodeType_NEG,
  AqvmInterpreterBytecodeType_SHL,
  AqvmInterpreterBytecodeType_SHR,
  AqvmInterpreterBytecodeType_SAR,
  AqvmInterpreterBytecodeType_IF,
  AqvmInterpreterBytecodeType_WHILE,
  AqvmInterpreterBytecodeType_NOT,
  AqvmInterpreterBytecodeType_AND,
  AqvmInterpreterBytecodeType_OR,
  AqvmInterpreterBytecodeType_XOR,
  AqvmInterpreterBytecodeType_CMP,
  AqvmInterpreterBytecodeType_INVOKE,
  AqvmInterpreterBytecodeType_RETURN,
  AqvmInterpreterBytecodeType_GOTO,
  AqvmInterpreterBytecodeType_THROW,
  AqvmInterpreterBytecodeType_WIDE = 0xFF
};

// TODO(Bytecode): Change these functions after completing bytecode development.
int AqvmInterpreterBytecode_NOP();
int AqvmInterpreterBytecode_LDC(void* value,
                                AqvmInterpreterRegister_Register* register);
int AqvmInterpreterBytecode_LOAD(AqvmInterpreterRegister_Register* ptr,
                                 AqvmInterpreterRegister_Register* register);
int AqvmInterpreterBytecode_STORE(AqvmInterpreterRegister_Register** ptr,
                                  AqvmInterpreterRegister_Register* register);
int AqvmInterpreterBytecode_NEW(void** ptr, size_t size);
int AqvmInterpreterBytecode_FREE(void* ptr, size_t size);
int AqvmInterpreterBytecode_SIZE();
int AqvmInterpreterBytecode_ADD(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_SUB(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_MUL(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_DIV(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_MOD(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_NEG(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_SHL(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_SHR(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_SAR(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_IF();
int AqvmInterpreterBytecode_WHILE();
int AqvmInterpreterBytecode_NOT(bool result, bool opcode);
int AqvmInterpreterBytecode_AND(bool result, bool opcode1, bool opcode2);
int AqvmInterpreterBytecode_OR(bool result, bool opcode1, bool opcode2);
int AqvmInterpreterBytecode_XOR(bool result, bool opcode1, bool opcode2);
int AqvmInterpreterBytecode_CMP(bool result, int operator,
                                AqvmInterpreterRegister_Register * opcode1,
                                AqvmInterpreterRegister_Register* opcode2);
int AqvmInterpreterBytecode_INVOKE();
int AqvmInterpreterBytecode_RETURN();
int AqvmInterpreterBytecode_GOTO();
int AqvmInterpreterBytecode_THROW();

// TODO(WIDE): Update this function if an extended instruction set is required.
int AqvmInterpreterBytecode_WIDE();

#endif
