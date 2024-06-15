// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_BYTECODE_OPCODE_OPCODE_H_
#define AQ_AQVM_INTERPRETER_BYTECODE_OPCODE_OPCODE_H_

#include <stdbool.h>
#include <stddef.h>


enum AqvmInterpreterBytecodeOpcode_Type {
  AqvmInterpreterBytecodeOpcodeType_NOP = 0x00,
  AqvmInterpreterBytecodeOpcodeType_LOAD,
  AqvmInterpreterBytecodeOpcodeType_STORE,
  AqvmInterpreterBytecodeOpcodeType_NEW,
  AqvmInterpreterBytecodeOpcodeType_FREE,
  AqvmInterpreterBytecodeOpcodeType_SIZE,
  AqvmInterpreterBytecodeOpcodeType_ADD,
  AqvmInterpreterBytecodeOpcodeType_SUB,
  AqvmInterpreterBytecodeOpcodeType_MUL,
  AqvmInterpreterBytecodeOpcodeType_DIV,
  AqvmInterpreterBytecodeOpcodeType_REM,
  AqvmInterpreterBytecodeOpcodeType_NEG,
  AqvmInterpreterBytecodeOpcodeType_SHL,
  AqvmInterpreterBytecodeOpcodeType_SHR,
  AqvmInterpreterBytecodeOpcodeType_SAR,
  AqvmInterpreterBytecodeOpcodeType_IF,
  AqvmInterpreterBytecodeOpcodeType_AND,
  AqvmInterpreterBytecodeOpcodeType_OR,
  AqvmInterpreterBytecodeOpcodeType_XOR,
  AqvmInterpreterBytecodeOpcodeType_CMP,
  AqvmInterpreterBytecodeOpcodeType_INVOKE,
  AqvmInterpreterBytecodeOpcodeType_RETURN,
  AqvmInterpreterBytecodeOpcodeType_GOTO,
  AqvmInterpreterBytecodeOpcodeType_THROW,
  AqvmInterpreterBytecodeOpcodeType_WIDE = 0xFF
};

// TODO(Bytecode): Change these functions after completing bytecode development.
int AqvmInterpreterBytecodeOpcode_NOP();
int AqvmInterpreterBytecodeOpcode_LOAD(
    struct AqvmMemoryRegister_Register* ptr,
    struct AqvmMemoryRegister_Register* operand);
int AqvmInterpreterBytecodeOpcode_STORE(
    struct AqvmMemoryRegister_Register** ptr,
    struct AqvmMemoryRegister_Register* operand);
int AqvmInterpreterBytecodeOpcode_NEW(void** ptr, size_t size);
int AqvmInterpreterBytecodeOpcode_FREE(void* ptr, size_t size);
int AqvmInterpreterBytecodeOpcode_SIZE();
int AqvmInterpreterBytecodeOpcode_ADD(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_SUB(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_MUL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_DIV(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_REM(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_NEG(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_SHL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_SHR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_SAR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_IF();
int AqvmInterpreterBytecodeOpcode_AND(bool result, bool operand1,
                                      bool operand2);
int AqvmInterpreterBytecodeOpcode_OR(bool result, bool operand1, bool operand2);
int AqvmInterpreterBytecodeOpcode_XOR(bool result, bool operand1,
                                      bool operand2);
int AqvmInterpreterBytecodeOpcode_CMP(
    bool result, int opcode, struct AqvmMemoryRegister_Register * operand1,
    struct AqvmMemoryRegister_Register* operand2);
int AqvmInterpreterBytecodeOpcode_INVOKE();
int AqvmInterpreterBytecodeOpcode_RETURN();
int AqvmInterpreterBytecodeOpcode_GOTO();
int AqvmInterpreterBytecodeOpcode_THROW();

// TODO(WIDE): Update this function if an extended instruction set is required.
int AqvmInterpreterBytecodeOpcode_WIDE();

#endif
