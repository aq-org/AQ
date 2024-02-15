// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/token.h"

#include "compiler/lexer/lex_map.h"
#include "debug/debug.h"

namespace Aq {
namespace Compiler {
TokenMap::TokenMap() {
  // TODO: Add all tokens to the maps.
}
TokenMap::~TokenMap() = default;

Token::KeywordType TokenMap::GetKeywordValue(char* keyword) {
  return *keyword_map.Find(keyword);
}
Token::OperatorType TokenMap::GetOperatorValue(char* _operator) {
  return *operator_map.Find(_operator);
}
Token::SeparatorType TokenMap::GetSeparatorValue(char* separator) {
  return *separator_map.Find(separator);
}
}  // namespace Compiler
}  // namespace Aq