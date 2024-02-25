// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_TOKEN_MAP_H_
#define AQ_COMPILER_LEXER_TOKEN_MAP_H_

#include "compiler/compiler.h"
#include "compiler/token/token.h"
#include "compiler/lexer/lex_map.h"

namespace Aq {
class Compiler::TokenMap {
 public:
  TokenMap();
  ~TokenMap();

  Token::Keyword GetKeywordValue(std::string keyword);
  Token::Operator GetOperatorValue(std::string _operator);

  TokenMap(const TokenMap&) = default;
  TokenMap(TokenMap&&) noexcept = default;
  TokenMap& operator=(const TokenMap&) = default;
  TokenMap& operator=(TokenMap&&) noexcept = default;

 private:
  LexMap<Token::Keyword> keyword_map_;
  LexMap<Token::Operator> operator_map_;
};
}  // namespace Aq

#endif