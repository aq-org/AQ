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
  /// \fn Lexer
  /// \brief Creates and initialize a Lexer.
  /// \details Initialize the Lexer class and store `source_code` to
  /// `buffer_ptr_`.
  Lexer(char* source_code, size_t length);
  ~Lexer();

  Lexer(const Lexer&) = delete;
  Lexer(Lexer&&) noexcept = delete;
  Lexer& operator=(const Lexer&) = delete;
  Lexer& operator=(Lexer&&) noexcept = delete;

  /// \fn LexToken
  /// \brief Lexically analyze `buffer_ptr_` and store the analyzed token to
  /// `return_token`.
  /// \details Reads one character at a time and analyzes the token for
  /// completeness except for that character, and repeatedly reads the
  /// characters of `buffer_ptr_` until the token is complete, then returns a
  /// token.
  /// \param return_token Token& Type.
  /// \return A normal read returns `0`, and a read error returns `-1`.
  int LexToken(Token& return_token);

  /// \fn IsReadEnd
  /// \brief Returns whether the source code has finished reading.
  /// \return true if the source code has finished reading, false otherwise.
  bool IsReadEnd() const;

 private:
  /// @brief The source code to be analyzed.
  char* buffer_ptr_;

  /// @brief The end of the source code to be analyzed.
  char* buffer_end_;
  
  /// @brief A token map to get the analyzed tokens' kinds.
  TokenMap token_map_;

  /// \fn ProcessToken
  /// \brief Process the token being lexically analyzed and determine if the token is
  /// complete in the general case.
  /// \details This function is only applicable to the general case, special cases
  /// require additional judgment logic. In general, if `token` belongs to the
  /// Token::Kind::UNKNOWN type, the type of `token` will be modified to
  /// `start_kind` and true will be returned. if `token` matches one of the
  /// variable parameters (i.e., `next_kind_list`), then true will be returned,
  /// otherwise false will be returned.
  /// \param token Token& Type
  /// \param start_kind Token::Kind Type
  /// \param next_kind_size int Type
  /// \param ... Token::Kind Type
  /// \return In general, returning true means that `token` is incomplete except for
  /// that character, and returning false means that `token` is complete except
  /// for that character.
  bool ProcessToken(Token& token, Token::Kind start_kind, int next_kind_size,
                    ...) const;
};
}  // namespace Aq

#endif