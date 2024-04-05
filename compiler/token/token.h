// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKEN_H_
#define AQ_COMPILER_TOKEN_TOKEN_H_

#include <cstddef>

#include "compiler/compiler.h"

namespace Aq {
class Compiler::Token {
 public:
  Token();
  ~Token();

  Token(const Token&) = default;
  Token(Token&&) noexcept = default;
  Token& operator=(const Token&) = default;
  Token& operator=(Token&&) noexcept = default;

  enum class Kind;

  void SetKind(Kind kind);
  Kind GetKind() const;
  void SetDataPtr(void* data_ptr);
  void* GetDataPtr() const;

 private:
  Kind kind_;
  void* data_ptr_;
};
}  // namespace Aq
#endif