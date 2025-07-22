// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_LEXER_LEXER_H_
#define AQ_LEXER_LEXER_H_

#include <cstddef>
#include <cstring>

#include "token/token.h"
#include "token/tokenmap.h"

namespace Aq {
class Lexer {
 public:
  // Initialize the Lexer class and store |source_code| to |code_ptr_|.
  Lexer(char* source_code, std::size_t length)
      : code_ptr_(source_code), code_end_(source_code + length - 1){};
  ~Lexer() = default;

  Lexer(const Lexer&) = default;
  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(const Lexer&) = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  // Lexical analysis |code_ptr_|, and store the analyzed token into
  // |return_token|.
  int LexToken(Token last_token, Token& return_token);

  // Return true if the lexer is at the end of |code_ptr_|.
  bool IsReadEnd() const;

 private:
  char* code_ptr_;
  char* code_end_;
  TokenMap token_map_;

  // Performs lexical analysis on brackets. Returns true if the brackets are
  // part of the token.
  bool LexBrackets(Token& token);

  // Performs lexical analysis on general operators. Returns true if the general
  // operators are part of the token.
  bool LexGeneralOperators(Token& token);

  // Performs lexical analysis on string. Returns true if the string is part of
  // the token.
  bool LexString(Token& token);

  // Performs lexical analysis on character. Returns true if the character is
  // part of the token.
  bool LexCharacter(Token& token);

  // Performs lexical analysis on escape character. Returns true if the escape
  // character is part of the token.
  bool LexEscapeCharacter(Token& token, char*& current_location);

  bool IsHexEscapeCharacter(char* current_location);

  // Returns the length of hex escape character.
  std::size_t GetHexEscapeCharacterLength(char* current_location);

  // Performs lexical analysis on hex escape character.
  void LexHexEscapeCharacter(Token& token, char*& current_location);

  bool IsOctEscapeCharacter(char* current_location);

  // Returns the length of oct escape character.
  std::size_t GetOctEscapeCharacterLength(char* current_location);

  // Performs lexical analysis on oct escape character.
  void LexOctEscapeCharacter(Token& token, char*& current_location);

  // Performs lexical analysis on general escape character.
  void LexGeneralEscapeCharacter(Token& token, char*& current_location);

  void LexSignedNumbersAtTheStart(Token& token, char*& current_location,
                                  Token& last_token);

  bool LexScientificNotation(char*& current_location);

  // Performs lexical analysis on signed numbers. Returns true if the signed
  // numbers.
  bool LexSignedNumbers(Token& token, char*& current_location,
                        Token& last_token);

  // Performs lexical analysis on decimal point. Returns true if the decimal
  // point is part of the token.
  bool LexDecimalPoint(Token& token);

  void LexCommentAtTheStart(Token& token, char*& current_location);

  void LexCommentAtTheEnd(Token& token, char*& current_location);

  // Performs lexical analysis on comments. Returns true if the comment is part
  // of the token.
  bool LexComment(Token& token, char*& current_location);

  // Performs lexical analysis on numbers. Returns true if the numbers are part
  // of the token.
  bool LexNumbers(Token& token);

  // Performs lexical analysis on identifiers. Returns true if the identifier is
  // part of the token.
  bool LexWhitespace(Token& token);

  // Performs lexical analysis on whitespace. Returns true if the whitespace is
  // part of the token.
  bool LexNewlines(Token& token, char*& current_location);

  // Performs lexical analysis on separators. Returns true if the separator is
  // part of the token.
  bool LexSeparator(Token& token, char*& current_location);

  // Performs lexical analysis on keywords. Returns true if the keyword is part
  // of the token.
  bool LexDefault(Token& token);

  // Processes the token after lexical analysis is completed.
  void HandleFinalToken(Token& token, char*& current_location);
};
}  // namespace Aq

#endif