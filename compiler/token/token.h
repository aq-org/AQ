// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKEN_H_
#define AQ_COMPILER_TOKEN_TOKEN_H_

#include <cstddef>

#include "compiler/compiler.h"

namespace Aq {
// TODO: Should be improved
class Compiler::Token {
 public:
  enum class Kind;
  enum class Type {
    NONE,
    START,
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    NUMBER,
    CHARACTER,
    STRING,
    COMMENT
  };
  enum class Keyword;
  enum class Operator;
  struct ValueStr {
    char* location;
    size_t length;
  };
  union Value {
    ValueStr number;
    Keyword keyword;
    ValueStr identifier;
    Operator _operator;
    ValueStr character;
    ValueStr string;
  };

  Type type;
  Value value;

  Token();
  ~Token();

  Token(const Token&) = default;
  Token(Token&&) noexcept = default;
  Token& operator=(const Token&) = default;
  Token& operator=(Token&&) noexcept = default;
};
}  // namespace Aq
#endif