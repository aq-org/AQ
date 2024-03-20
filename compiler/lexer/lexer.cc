// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include <cstdarg>
#include <string>

#include "compiler/compiler.h"
#include "compiler/token/keyword.h"
#include "compiler/token/operator.h"
#include "compiler/token/token.h"
#include "debugger/debugger.h"

namespace Aq {
Compiler::Lexer::Lexer(char* source_code, size_t length)
    : buffer_ptr_(source_code), buffer_end_(source_code + length - 1){};
Compiler::Lexer::~Lexer() = default;

int Compiler::Lexer::LexToken(Token& return_token) {
  using Tok = Token::Type;

  // Set the return token type to start.
  return_token.type = Tok::START;

  // Set the reading position pointer equal to the buffer pointer.
  char* read_ptr = buffer_ptr_;

LexStart:
  // Memory out of bounds occurred. Return an error.
  if (read_ptr > buffer_end_) {
    buffer_ptr_ = read_ptr;
    Debugger error_info(Debugger::Level::ERROR, "Aq::Compiler::Lexer::LexToken",
                        "LexToken_ReadError", "Memory out of bounds occurred.",
                        nullptr);
    return -1;
  }

  // Start lexical analysis.
  switch (*read_ptr) {
    // General operators.
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '(':
    case ')':
    case '*':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case ']':
    case '^':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
      if (ProcessToken(return_token, Tok::OPERATOR, 4, Tok::OPERATOR,
                       Tok::CHARACTER, Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    // The string flag.
    case '"':
      if (ProcessToken(return_token, Tok::STRING, 2, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      // End of string.
      if (return_token.type == Tok::STRING) {
        read_ptr++;
      }
      goto LexEnd;

    // The character flag.
    case '\'':
      if (ProcessToken(return_token, Tok::CHARACTER, 2, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      // End of character.
      if (return_token.type == Tok::CHARACTER) {
        read_ptr++;
      }
      goto LexEnd;

    // Escape character.
    case '\\':
      if (ProcessToken(return_token, Tok::OPERATOR, 2, Tok::OPERATOR,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

      // Skip escape characters.
      if (return_token.type == Tok::CHARACTER ||
          return_token.type == Tok::STRING) {
        if (read_ptr + 2 <= buffer_end_) {
          read_ptr++;
        }
        goto LexNext;
      }

    // Positive and negative numbers.
    case '+':
    case '-':
      // Signed numbers.
      if (return_token.type == Tok::START && *(read_ptr + 1) == '0' ||
          *(read_ptr + 1) == '1' || *(read_ptr + 1) == '2' ||
          *(read_ptr + 1) == '3' || *(read_ptr + 1) == '4' ||
          *(read_ptr + 1) == '5' || *(read_ptr + 1) == '6' ||
          *(read_ptr + 1) == '7' || *(read_ptr + 1) == '8' ||
          *(read_ptr + 1) == '9') {
        return_token.type = Tok::NUMBER;
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 4, Tok::OPERATOR,
                       Tok::CHARACTER, Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      // Dealing with scientific notation.
      if (return_token.type == Tok::NUMBER &&
          (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e')) {
        goto LexNext;
      }
      goto LexEnd;

    // Decimal point.
    case '.':
      if (ProcessToken(return_token, Tok::OPERATOR, 5, Tok::OPERATOR,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    // The comment flag.
    case '/':
      // Comment start.
      if (return_token.type == Tok::START && *(buffer_ptr_ + 1) == '/' ||
          *(buffer_ptr_ + 1) == '*') {
        return_token.type = Tok::COMMENT;
        if (read_ptr + 2 <= buffer_end_) {
          read_ptr++;
        }
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 2, Tok::CHARACTER,
                       Tok::STRING)) {
        goto LexNext;
      }

      if (return_token.type == Tok::OPERATOR) {
        // Comment.
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          goto LexNext;
        }
      }

      if (return_token.type == Tok::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*' && *(read_ptr - 1) == '*') {
          // /**/ style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          return_token.type = Tok::START;
          goto LexStart;
        } else {
          // // style comments or Non-end comment mark, continue reading until
          // the end mark of the comment.
          goto LexNext;
        }
      }
      goto LexEnd;

    // Numbers.
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (ProcessToken(return_token, Tok::NUMBER, 5, Tok::IDENTIFIER,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    // Whitespace characters.
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      if (return_token.type == Tok::START) {
        // Skip whitespace characters.
        buffer_ptr_++;
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::START, 3, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    // Newlines.
    case '\n':
      if (return_token.type == Tok::START) {
        // Skip newlines.
        buffer_ptr_++;
        goto LexNext;
      }

      if (return_token.type == Tok::COMMENT && *(buffer_ptr_ + 1) == '/') {
        // // style comments, skip all comments.
        buffer_ptr_ = ++read_ptr;
        return_token.type = Tok::START;
        goto LexStart;
      }

      if (ProcessToken(return_token, Tok::START, 3, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    // EOF.
    case '\0':
      goto LexEnd;

    // Separator flag.
    case ',':
    case ';':
      if (return_token.type == Tok::START) {
        return_token.type = Tok::OPERATOR;
        read_ptr++;
        goto LexEnd;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 3, Tok::CHARACTER,
                       Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;

    default:
      if (ProcessToken(return_token, Tok::IDENTIFIER, 5, Tok::IDENTIFIER,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }
      goto LexEnd;
  }

LexNext:
  read_ptr++;
  goto LexStart;

LexEnd:
  // Meaningless token.
  if (return_token.type == Tok::START || return_token.type == Tok::COMMENT) {
    return_token.type = Tok::NONE;
    buffer_ptr_ = read_ptr;
    return 0;
  } else {
    // Meaningful token. Determine the specific token information.
    char* location = buffer_ptr_;
    size_t length = read_ptr - buffer_ptr_;
    buffer_ptr_ = read_ptr;

    // Handle the detailed information of tokens.
    Token::ValueStr value;
    value.location = location;
    value.length = length;
    switch (return_token.type) {
      case Tok::IDENTIFIER:
        return_token.value.keyword =
            token_map_.GetKeywordValue(std::string(location, length));
        if (return_token.value.keyword == Token::Keyword::NONE) {
          return_token.value.identifier = value;
          break;
        }
        return_token.type = Tok::KEYWORD;
        break;

      case Tok::CHARACTER:
        return_token.value.character = value;
        break;

      case Tok::STRING:
        return_token.value.string = value;
        break;

      case Tok::OPERATOR:
        return_token.value._operator =
            token_map_.GetOperatorValue(std::string(location, length));
        while (return_token.value._operator == Token::Operator::NONE &&
               length > 1) {
          length--;
          buffer_ptr_--;
          return_token.value._operator =
              token_map_.GetOperatorValue(std::string(location, length));
        }
        break;

      case Tok::NUMBER:
        return_token.value.number = value;
        break;

      default:
        Debugger error_info(Debugger::Level::ERROR,
                            "Aq::Compiler::Lexer::LexToken",
                            "Lextoken_UnexpectedSituations",
                            "Encountered a situation where the token value "
                            "should not exist while processing.",
                            nullptr);
        return -1;
    }
  }
  return 0;
}

bool Compiler::Lexer::IsReadEnd() const {
  if (buffer_ptr_ >= buffer_end_) {
    return true;
  }
  return false;
}

bool Compiler::Lexer::ProcessToken(Token& token, Token::Type start_type,
                                   int next_type_size, ...) {
  if (token.type == Token::Type::START) {
    token.type = start_type;
    return true;
  }
  std::va_list next_type_list;
  va_start(next_type_list, next_type_size);
  for (int i = 0; i < next_type_size; i++) {
    if (token.type == va_arg(next_type_list, Token::Type)) {
      return true;
    }
  }
  return false;
}
}  // namespace Aq