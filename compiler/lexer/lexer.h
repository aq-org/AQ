// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEXER_H_
#define AQ_COMPILER_LEXER_LEXER_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/lexer/token_map.h"
#include "compiler/token/token.h"

namespace Aq {
class Compiler::Lexer {
 public:
  // Initialize the Lexer class and store |source_code| to |buffer_ptr_|.
  Lexer(char* source_code, size_t length)
      : buffer_ptr_(source_code), buffer_end_(source_code + length - 1){};
  ~Lexer() = default;

  Lexer(const Lexer&) = default;
  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(const Lexer&) = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  // Lexical analysis |buffer_ptr_|, and store the analyzed token into
  // |return_token|. Returns one token at a time. Returns 0 for a normal read,
  // -1 for a read error.
  int LexToken(Token& return_token);

  // Returns whether the source code has finished reading.
  bool IsReadEnd() const;

 private:
  char* buffer_ptr_;
  char* buffer_end_;
  TokenMap token_map_;

  // Process the token being lexically analyzed.
  // If |token| belongs to the Token::Type::START type, the type of |token| will
  // be modified to |start_type| and true will be returned; otherwise, the
  // return value depends on whether the type of |token| matches |next_type|.
  // This function only applies to the general case, special cases require
  // additional judgment logic. In general, returning true means that |token| is
  // not yet complete, and vice versa.
  bool ProcessToken(Token& token, Token::Type start_type,
                    Token::Type next_type[], int next_type_size);
};
}  // namespace Aq

#endif