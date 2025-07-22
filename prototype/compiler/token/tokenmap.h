// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKENMAP_H_
#define AQ_COMPILER_TOKEN_TOKENMAP_H_

#include <string>
#include <unordered_map>

#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {
class TokenMap {
 public:
  TokenMap();
  ~TokenMap() = default;

  Token::KeywordType GetKeywordValue(std::string keyword);
  Token::OperatorType GetOperatorValue(std::string oper);

  TokenMap(const TokenMap&) = default;
  TokenMap(TokenMap&&) noexcept = default;
  TokenMap& operator=(const TokenMap&) = default;
  TokenMap& operator=(TokenMap&&) noexcept = default;

 private:
  std::unordered_map<std::string, Token::KeywordType> keyword_map;
  std::unordered_map<std::string, Token::OperatorType> operator_map;
};
}  // namespace Compiler
}  // namespace Aq
#endif