// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_CODE_BYTECODE_H_
#define AQ_AQVM_CODE_BYTECODE_H_

enum AqvmInterpreterBytecode_Type {
  LOAD,
  STORE,
  NEWARRAY,
  RETURN,
  ARRAYLENGTH,
  THROW,
  PUSH,
  ADD,
  CONST,
  DIV,
  MUL,
  NEG,
  REM,
  SUB,
  GOTO,
  AND,
  IFEQ,
  IFGE,
  IFGT,
  IFLE,
  IFLT,
  IFNE,
  INVOKE,
  OR,
  SHL,
  SHR,
  XOR,
  LDC,
  NEW,
  NOP,
  POP,
  WIDE
};

#endif