// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/interpreter/bytecode/opcode/opcode.h"

#include <stdbool.h>
#include <stddef.h>

#include "aqvm/memory/memory.h"
#include "aqvm/memory/register.h"

/*
int AqvmInterpreterBytecodeOpcode_NOP() { return 0; }

int AqvmInterpreterBytecodeOpcode_LDC(
    void* value, struct AqvmMemoryRegister_Register* operand) {
  switch (operand->type) {
    case AqvmMemoryRegisterValueType_INT:
      operand->value.int_value = *(int*)value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      operand->value.float_value = *(float*)value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      operand->value.double_value = *(double*)value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      operand->value.long_value = *(long*)value;
      break;
    case AqvmMemoryRegisterValueType_CHARACTER:
      operand->value.character_value = *(char*)value;
      break;
    case AqvmMemoryRegisterValueType_BOOLEAN:
      operand->value.boolean_value = *(bool*)value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }

  return 0;
}

int AqvmInterpreterBytecodeOpcode_LOAD(
    struct AqvmMemoryRegister_Register* ptr,
    struct AqvmMemoryRegister_Register* operand) {
  //*operand = *ptr;
  return 0;
}

int AqvmInterpreterBytecodeOpcode_STORE(
    struct AqvmMemoryRegister_Register** ptr,
    struct AqvmMemoryRegister_Register* operand) {
  *ptr = operand;
  return 0;
}

int AqvmInterpreterBytecodeOpcode_NEW(void** ptr, size_t size) {
  *ptr = AqvmMemory_MemoryAllocation(size);
  if (*ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory allocation.
    return -1;
  }
  return 0;
}

int AqvmInterpreterBytecodeOpcode_FREE(void* ptr, size_t size) {
  AqvmMemory_FreeMemory(ptr);
  if (ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory deallocation.
    return -1;
  }
  return 0;
}

// TODO(SIZE): Wait to develop this function until the form of this operator has
// been decided.
int AqvmInterpreterBytecodeOpcode_SIZE() {}

int AqvmInterpreterBytecodeOpcode_ADD(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {
  if (operand1->type != operand2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = operand1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          operand1->value.int_value + operand2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          operand1->value.float_value + operand2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          operand1->value.double_value + operand2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          operand1->value.long_value + operand2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

int AqvmInterpreterBytecodeOpcode_SUB(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {
  if (operand1->type != operand2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = operand1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          operand1->value.int_value - operand2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          operand1->value.float_value - operand2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          operand1->value.double_value - operand2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          operand1->value.long_value - operand2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

int AqvmInterpreterBytecodeOpcode_MUL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {
  if (operand1->type != operand2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = operand1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          operand1->value.int_value * operand2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          operand1->value.float_value * operand2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          operand1->value.double_value * operand2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          operand1->value.long_value * operand2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

int AqvmInterpreterBytecodeOpcode_DIV(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {
  if (operand1->type != operand2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = operand1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          operand1->value.int_value / operand2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_FLOAT:
      result->value.float_value =
          operand1->value.float_value / operand2->value.float_value;
      break;
    case AqvmMemoryRegisterValueType_DOUBLE:
      result->value.double_value =
          operand1->value.double_value / operand2->value.double_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          operand1->value.long_value / operand2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

int AqvmInterpreterBytecodeOpcode_REM(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {
  if (operand1->type != operand2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = operand1->type;
  switch (result->type) {
    case AqvmMemoryRegisterValueType_INT:
      result->value.int_value =
          operand1->value.int_value % operand2->value.int_value;
      break;
    case AqvmMemoryRegisterValueType_LONG:
      result->value.long_value =
          operand1->value.long_value % operand2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}

// TODO(Bytecode): Waiting for development.
int AqvmInterpreterBytecodeOpcode_NEG(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {}
int AqvmInterpreterBytecodeOpcode_SHL(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {}
int AqvmInterpreterBytecodeOpcode_SHR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {}
int AqvmInterpreterBytecodeOpcode_SAR(
    struct AqvmMemoryRegister_Register* result,
    struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {}
int AqvmInterpreterBytecodeOpcode_IF() {}
int AqvmInterpreterBytecodeOpcode_AND(bool result, bool operand1,
                                      bool operand2) {}
int AqvmInterpreterBytecodeOpcode_OR(bool result, bool operand1,
                                     bool operand2) {}
int AqvmInterpreterBytecodeOpcode_XOR(bool result, bool operand1,
                                      bool operand2) {}
int AqvmInterpreterBytecodeOpcode_CMP(
    bool result, int opcode, struct AqvmMemoryRegister_Register* operand1,
    struct AqvmMemoryRegister_Register* operand2) {}
int AqvmInterpreterBytecodeOpcode_INVOKE() {}
int AqvmInterpreterBytecodeOpcode_RETURN() {}
int AqvmInterpreterBytecodeOpcode_GOTO() {}
int AqvmInterpreterBytecodeOpcode_THROW() {}

// TODO(WIDE): Update this function if an extended instruction set is
// required.
int AqvmInterpreterBytecodeOpcode_WIDE() {}
*/