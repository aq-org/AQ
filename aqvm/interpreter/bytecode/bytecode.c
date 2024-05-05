// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/interpreter/bytecode/bytecode.h"

#include <stdbool.h>
#include <stddef.h>

#include "aqvm/memory/memory.h"
#include "aqvm/memory/register.h"

int AqvmInterpreterBytecode_NOP() { return 0; }

int AqvmInterpreterBytecode_LDC(
    void* value, struct AqvmMemoryRegister_Register* opcode) {
  switch (opcode->type) {
    case AqvmMemoryRegisterValueType_INT:
      opcode->value.int_value = *(int*)value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      opcode->value.float_value = *(float*)value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      opcode->value.double_value = *(double*)value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      opcode->value.long_value = *(long*)value;
      break;
    case AqvmMemoryRegisterValueType_CHARACTER:
      opcode->value.character_value = *(char*)value;
      break;
    case AqvmMemoryRegisterValueType_BOOLEAN:
      opcode->value.boolean_value = *(bool*)value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }

  return 0;
}

int AqvmInterpreterBytecode_LOAD(
    struct AqvmMemoryRegister_Register* ptr,
    struct AqvmMemoryRegister_Register* opcode) {
  *opcode = *ptr;
  return 0;
}

int AqvmInterpreterBytecode_STORE(
    struct AqvmMemoryRegister_Register** ptr,
    struct AqvmMemoryRegister_Register* opcode) {
  *ptr = opcode;
  return 0;
}

int AqvmInterpreterBytecode_NEW(void** ptr, size_t size) {
  *ptr = AqvmMemory_MemoryAllocation(size);
  if (*ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory allocation.
    return -1;
  }
  return 0;
}

int AqvmInterpreterBytecode_FREE(void* ptr, size_t size) {
  AqvmMemory_FreeMemory(ptr);
  if (ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory deallocation.
    return -1;
  }
  return 0;
}

// TODO(SIZE): Wait to develop this function until the form of this operator has
// been decided.
int AqvmInterpreterBytecode_SIZE() {}

int AqvmInterpreterBytecode_ADD(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {
  if (opcode1->type != opcode2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = opcode1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          opcode1->value.int_value + opcode2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          opcode1->value.float_value + opcode2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          opcode1->value.double_value + opcode2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          opcode1->value.long_value + opcode2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}
int AqvmInterpreterBytecode_SUB(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {
  if (opcode1->type != opcode2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = opcode1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          opcode1->value.int_value - opcode2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          opcode1->value.float_value - opcode2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          opcode1->value.double_value - opcode2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          opcode1->value.long_value - opcode2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

// TODO(Bytecode): Waiting for development.
int AqvmInterpreterBytecode_MUL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_DIV(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_MOD(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_NEG(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SHL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SHR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SAR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_IF() {}
int AqvmInterpreterBytecode_WHILE() {}
int AqvmInterpreterBytecode_NOT(bool result, bool opcode) {}
int AqvmInterpreterBytecode_AND(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_OR(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_XOR(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_CMP(
    bool result, int operator,
    struct AqvmMemoryRegister_Register * opcode1,
    struct AqvmMemoryRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_INVOKE() {}
int AqvmInterpreterBytecode_RETURN() {}
int AqvmInterpreterBytecode_GOTO() {}
int AqvmInterpreterBytecode_THROW() {}

// TODO(WIDE): Update this function if an extended instruction set is
// required.
int AqvmInterpreterBytecode_WIDE() {}