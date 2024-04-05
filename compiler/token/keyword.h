/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#ifndef AQ_COMPILER_TOKEN_KEYWORD_H_
#define AQ_COMPILER_TOKEN_KEYWORD_H_

#include "compiler/compiler.h"
#include "compiler/token/token.h"

namespace Aq {
enum class Compiler::Token::Keyword {
  NONE = 0,
  Auto,
  And,
  Bitand,
  Bitor,
  Bool,
  Break,
  Case,
  Catch,
  Char,
  Class,
  Const,
  Continue,
  Default,
  Do,
  Double,
  Else,
  Enum,
  Export,
  Extern,
  False,
  Float,
  For,
  Friend,
  Goto,
  Import,
  Inline,
  Int,
  Long,
  Namespace,
  New,
  Not,
  Number,
  Operator,
  Or,
  Private,
  Protected,
  Public,
  Return,
  Short,
  Signed,
  Sizeof,
  Static,
  String,
  Struct,
  Switch,
  Template,
  This,
  Thread,
  True,
  Try,
  Typedef,
  Typeid,
  Typename,
  Union,
  Unsigned,
  Using,
  Virtual,
  Void,
  Wchar_t,
  While,
  Xor,
};
}

#endif