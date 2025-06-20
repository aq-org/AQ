// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_VM_OPERATOR_OPERATOR_H_
#define AQ_VM_OPERATOR_OPERATOR_H_

namespace Aq {
namespace Vm {
namespace Operator {
enum class Operator {
  NOP = 0x00,
  LOAD,
  STORE,
  NEW,
  ARRAY,
  PTR,
  ADD,
  SUB,
  MUL,
  DIV,
  REM,
  NEG,
  SHL,
  SHR,
  REFER,
  IF,
  AND,
  OR,
  XOR,
  CMP,
  INVOKE,
  EQUAL,
  GOTO,
  LOAD_CONST,
  CONVERT,
  CONST,
  INVOKE_METHOD,
  LOAD_MEMBER,
  WIDE = 0xFF
};



}
}  // namespace Vm
}  // namespace Aq

#endif