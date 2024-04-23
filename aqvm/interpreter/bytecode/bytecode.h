// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_CODE_BYTECODE_H_
#define AQ_AQVM_CODE_BYTECODE_H_

enum AqvmInterpreterBytecode_Type {
  NOP = 0x00,
  CONST,
  LOAD,
  STORE,
  NEWARRAY,
  NEW,
  FREE,
  POINTER,
  SIZE,
  POP,
  ADD,
  SUB,
  MUL,
  DIV,
  REM,
  NEG,
  SHL,
  SHR,
  IF,
  WHILE,
  AND,
  OR,
  XOR,
  COMPARE,
  INVOKE,
  RETURN,
  GOTO,
  THROW,
  WIDE = 0xFF
};

#endif
