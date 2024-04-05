/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#ifndef AQ_COMPILER_LEXER_LEXER_H_
#define AQ_COMPILER_LEXER_LEXER_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/lexer/token_map.h"
#include "compiler/token/token.h"

namespace Aq {
class Compiler::Lexer {
 public:
  /// Initialize the Lexer class and store |source_code| to |buffer_ptr_|.
  Lexer(char* source_code, size_t length);
  ~Lexer();

  Lexer(const Lexer&) = default;
  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(const Lexer&) = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  /// Lexically analyze |buffer_ptr_| and store the analyzed token to
  /// |return_token|. Reads one character at a time and analyzes the token for
  /// completeness except for that character, and repeatedly reads the characters
  /// of |buffer_ptr_| until the token is complete, then returns a token. a
  /// normal read returns 0, and a read error returns -1.
  int LexToken(Token& return_token);

  /// Returns whether the source code has finished reading.
  bool IsReadEnd() const;

 private:
  char* buffer_ptr_;
  char* buffer_end_;
  TokenMap token_map_;

  /// Process the token being lexically analyzed and determine if the token is
  /// complete in the general case.
  /// This function is only applicable to the general case, special cases require
  /// additional judgment logic. In general, if |token| belongs to the
  /// Token::Type::START type, the type of |token| will be modified to
  /// |start_type| and true will be returned. if |token| matches one of the
  /// variable parameters (i.e., |next_type_list|), then true will be returned,
  /// otherwise false will be returned.
  /// In general, returning true means that |token| is incomplete except for that
  /// character, and returning false means that |token| is complete except for
  /// that character.
  bool ProcessToken(Token& token, Token::Type start_type, int next_type_size,
                    ...) const;
};
}  // namespace Aq

#endif