// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKEN_H_
#define AQ_COMPILER_TOKEN_TOKEN_H_

#include <cstddef>

#include "compiler/compiler.h"

namespace Aq {
/// TODO: Should be improved
class Compiler::Token {
 public:
  Token();
  ~Token();

  Token(const Token&) = default;
  Token(Token&&) noexcept = default;
  Token& operator=(const Token&) = default;
  Token& operator=(Token&&) noexcept = default;

  enum class Kind;

 private:
  Kind kind_;
  void* data_ptr_;
};
}  // namespace Aq
#endif