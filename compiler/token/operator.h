// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_OPERATOR_H_
#define AQ_COMPILER_TOKEN_OPERATOR_H_

#include "compiler/compiler.h"
#include "compiler/token/token.h"

namespace Aq{
enum class Compiler::Token::Operator {
  NONE = 0,
  l_square,
  r_square,
  l_paren,
  r_paren,
  l_brace,
  r_brace,
  period,
  ellipsis,
  amp,
  ampamp,
  ampequal,
  star,
  starequal,
  plus,
  plusplus,
  plusequal,
  minus,
  arrow,
  minusminus,
  minusequal,
  tilde,
  exclaim,
  exclaimequal,
  slash,
  slashequal,
  percent,
  percentequal,
  less,
  lessless,
  lessequal,
  lesslessequal,
  spaceship,
  greater,
  greatergreater,
  greaterequal,
  greatergreaterequal,
  caret,
  caretequal,
  pipe,
  pipepipe,
  pipeequal,
  question,
  colon,
  semi,
  equal,
  equalequal,
  comma,
  hash,
  hashhash,
  hashat,
  periodstar,
  arrowstar,
  coloncolon,
  at,
  lesslessless,
  greatergreatergreater,
  caretcaret,
};
}

#endif