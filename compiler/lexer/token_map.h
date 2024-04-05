// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_TOKEN_MAP_H_
#define AQ_COMPILER_LEXER_TOKEN_MAP_H_

#include "compiler/compiler.h"
#include "compiler/hash_map/hash_map.h"
#include "compiler/token/token.h"

namespace Aq {
/// TODO: Should be improved
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
  HashMap<Token::Keyword> keyword_map_;
  HashMap<Token::Operator> operator_map_;
};
}  // namespace Aq

#endif