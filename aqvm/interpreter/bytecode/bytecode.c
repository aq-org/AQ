// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/interpreter/bytecode/bytecode.h"

#include <stddef.h>

#include "aqvm/interpreter/register/register.h"

int AqvmInterpreterBytecode_NOP() { return 0; }

int AqvmInterpreterBytecode_LDC(void* value,
                                AqvmInterpreterRegister_Register* register) {
  switch (register->type) {
    case AqvmInterpreterRegisterValueType_INT:
      register->value.int_value = *(int*)value;
      break;
    case AqvmInterpreterRegisterValueType_FLOAT:
      register->value.float_value = *(float*)value;
      break;
    case AqvmInterpreterRegisterValueType_DOUBLE:
      register->value.double_value = *(double*)value;
      break;
    case AqvmInterpreterRegisterValueType_LONG:
      register->value.long_value = *(long*)value;
      break;
    case AqvmInterpreterRegisterValueType_CHARACTER:
      register->value.character_value = *(char*)value;
      break;
    case AqvmInterpreterRegisterValueType_BOOLEAN:
      register->value.boolean_value = *(bool*)value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }

  return 0;
}

int AqvmInterpreterBytecode_LOAD(AqvmInterpreterRegister_Register* ptr,
                                 AqvmInterpreterRegister_Register* register) {
  *register = *ptr;
  return 0;
}

int AqvmInterpreterBytecode_STORE(AqvmInterpreterRegister_Register** ptr,
                                  AqvmInterpreterRegister_Register* register) {
  *ptr = register;
  return 0;
}

int AqvmInterpreterBytecode_NEW(void** ptr, size_t size) {
  *ptr = malloc(size);
  if (*ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory allocation.
    return -1;
  }
  return 0;
}

int AqvmInterpreterBytecode_FREE(void* ptr, size_t size) {
  free(ptr);
  if (ptr == NULL) {
    // TODO(WARNING): Handle the warning of memory deallocation.
    return -1;
  }
  return 0;
}

// TODO(SIZE): Wait to develop this function until the form of this operator has
// been decided.
int AqvmInterpreterBytecode_SIZE() {}

int AqvmInterpreterBytecode_ADD(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {
  if (opcode1->type != opcode2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = opcode1->type;
  switch (result->type) {
    case AqvmInterpreterRegisterValueType_INT:
      result->value.int_value =
          opcode1->value.int_value + opcode2->value.int_value;
      break;
    case AqvmInterpreterRegisterValueType_FLOAT:
      result->value.float_value =
          opcode1->value.float_value + opcode2->value.float_value;
      break;
    case AqvmInterpreterRegisterValueType_DOUBLE:
      result->value.double_value =
          opcode1->value.double_value + opcode2->value.double_value;
      break;
    case AqvmInterpreterRegisterValueType_LONG:
      result->value.long_value =
          opcode1->value.long_value + opcode2->value.long_value;
      break;

    default:
      // TODO(ERROR): Handle the error of the register value type.
      return -1;
  }
  return 0;
}
int AqvmInterpreterBytecode_SUB(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {
  if (opcode1->type != opcode2->type) {
    // TODO(ERROR): Handle the error of the register value type.
    return -1;
  }
  result->type = opcode1->type;
  switch (result->type) {
    case AqvmInterpreterRegisterValueType_INT:
      result->value.int_value =
          opcode1->value.int_value - opcode2->value.int_value;
      break;
    case AqvmInterpreterRegisterValueType_FLOAT:
      result->value.float_value =
          opcode1->value.float_value - opcode2->value.float_value;
      break;
    case AqvmInterpreterRegisterValueType_DOUBLE:
      result->value.double_value =
          opcode1->value.double_value - opcode2->value.double_value;
      break;
    case AqvmInterpreterRegisterValueType_LONG:
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
int AqvmInterpreterBytecode_MUL(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_DIV(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_MOD(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_NEG(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SHL(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SHR(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_SAR(AqvmInterpreterRegister_Register* result,
                                AqvmInterpreterRegister_Register* opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_IF() {}
int AqvmInterpreterBytecode_WHILE() {}
int AqvmInterpreterBytecode_NOT(bool result, bool opcode) {}
int AqvmInterpreterBytecode_AND(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_OR(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_XOR(bool result, bool opcode1, bool opcode2) {}
int AqvmInterpreterBytecode_CMP(bool result, int operator,
                                AqvmInterpreterRegister_Register * opcode1,
                                AqvmInterpreterRegister_Register* opcode2) {}
int AqvmInterpreterBytecode_INVOKE() {}
int AqvmInterpreterBytecode_RETURN() {}
int AqvmInterpreterBytecode_GOTO() {}
int AqvmInterpreterBytecode_THROW() {}

// TODO(WIDE): Update this function if an extended instruction set is
// required.
int AqvmInterpreterBytecode_WIDE() {}