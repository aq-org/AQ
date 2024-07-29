// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_INTERPRETER_OPERATOR_OPERATOR_H_
#define AQ_AQVM_INTERPRETER_OPERATOR_OPERATOR_H_

#include <stddef.h>

#include "aqvm/memory/types.h"
#include "aqvm/memory/memory.h"

enum AqvmInterpreterOperator_Type {
  AqvmInterpreterOperatorType_NOP = 0x00,
  AqvmInterpreterOperatorType_LOAD,
  AqvmInterpreterOperatorType_STORE,
  AqvmInterpreterOperatorType_NEW,
  AqvmInterpreterOperatorType_FREE,
  AqvmInterpreterOperatorType_SIZE,
  AqvmInterpreterOperatorType_ADD,
  AqvmInterpreterOperatorType_SUB,
  AqvmInterpreterOperatorType_MUL,
  AqvmInterpreterOperatorType_DIV,
  AqvmInterpreterOperatorType_REM,
  AqvmInterpreterOperatorType_NEG,
  AqvmInterpreterOperatorType_SHL,
  AqvmInterpreterOperatorType_SHR,
  AqvmInterpreterOperatorType_SAR,
  AqvmInterpreterOperatorType_IF,
  AqvmInterpreterOperatorType_AND,
  AqvmInterpreterOperatorType_OR,
  AqvmInterpreterOperatorType_XOR,
  AqvmInterpreterOperatorType_CMP,
  AqvmInterpreterOperatorType_INVOKE,
  AqvmInterpreterOperatorType_RETURN,
  AqvmInterpreterOperatorType_GOTO,
  AqvmInterpreterOperatorType_THROW,
  AqvmInterpreterOperatorType_WIDE = 0xFF
};

// TODO(Bytecode): Change these functions after completing bytecode development.
/* int AqvmInterpreterOperator_NOP();
int AqvmInterpreterOperator_LOAD(
    struct AqvmMemory_Memory* ptr,
    struct AqvmMemory_Memory* operand);
int AqvmInterpreterOperator_STORE(
    struct AqvmMemory_Memory** ptr,
    struct AqvmMemory_Memory* operand);
int AqvmInterpreterOperator_NEW(void** ptr, size_t size);
int AqvmInterpreterOperator_FREE(void* ptr, size_t size);
int AqvmInterpreterOperator_SIZE();
int AqvmInterpreterOperator_ADD(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_SUB(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_MUL(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_DIV(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_REM(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_NEG(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_SHL(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_SHR(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_SAR(
    struct AqvmMemory_Memory* result,
    struct AqvmMemory_Memory* operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_IF();
int AqvmInterpreterOperator_AND(bool result, bool operand1,                              bool operand2);
int AqvmInterpreterOperator_OR(bool result, bool operand1, bool operand2);
int AqvmInterpreterOperator_XOR(bool result, bool operand1,                              bool operand2);
int AqvmInterpreterOperator_CMP(
    bool result, int opcode, struct AqvmMemory_Memory * operand1,
    struct AqvmMemory_Memory* operand2);
int AqvmInterpreterOperator_INVOKE();
int AqvmInterpreterOperator_RETURN();
int AqvmInterpreterOperator_GOTO();
int AqvmInterpreterOperator_THROW();

// TODO(WIDE): Update this function if an extended instruction set is required.
int AqvmInterpreterOperator_WIDE();
*/

#endif
