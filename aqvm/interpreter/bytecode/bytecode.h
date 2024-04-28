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

// TODO(Bytecode): Change these functions after completing bytecode development.
int AqvmInterpreterBytecode_NOP();
int AqvmInterpreterBytecode_LDC();
int AqvmInterpreterBytecode_LOAD();
int AqvmInterpreterBytecode_STORE();
int AqvmInterpreterBytecode_NEW();
int AqvmInterpreterBytecode_FREE();
int AqvmInterpreterBytecode_SIZE();
int AqvmInterpreterBytecode_ADD();
int AqvmInterpreterBytecode_SUB();
int AqvmInterpreterBytecode_MUL();
int AqvmInterpreterBytecode_DIV();
int AqvmInterpreterBytecode_MOD();
int AqvmInterpreterBytecode_NEG();
int AqvmInterpreterBytecode_SHL();
int AqvmInterpreterBytecode_SHR();
int AqvmInterpreterBytecode_SAR();
int AqvmInterpreterBytecode_IF();
int AqvmInterpreterBytecode_WHILE();
int AqvmInterpreterBytecode_NOT();
int AqvmInterpreterBytecode_AND();
int AqvmInterpreterBytecode_OR();
int AqvmInterpreterBytecode_XOR();
int AqvmInterpreterBytecode_CMP();
int AqvmInterpreterBytecode_INVOKE();
int AqvmInterpreterBytecode_RETURN();
int AqvmInterpreterBytecode_GOTO();
int AqvmInterpreterBytecode_THROW();
int AqvmInterpreterBytecode_WIDE();


#endif
