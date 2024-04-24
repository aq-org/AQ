// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_CODE_BYTECODE_H_
#define AQ_AQVM_CODE_BYTECODE_H_

enum AqvmInterpreterBytecode_Type {
  NOP = 0x00,
  LDC,
  LOAD,
  STORE,
  NEW,
  FREE,
  SIZE,
  ADD,
  SUB,
  MUL,
  DIV,
  MOD,
  NEG,
  SHL,
  SHR,
  SAR,
  IF,
  WHILE,
  NOT,
  AND,
  OR,
  XOR,
  CMP,
  INVOKE,
  RETURN,
  GOTO,
  THROW,
  WIDE = 0xFF
};

#endif