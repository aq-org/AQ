// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include <iostream>

#include "compiler/lexer/token.h"

namespace Aq {
namespace Compiler {
Lexer::Lexer(char* source_code, size_t length) {
  buffer_ptr_ = source_code;
  buffer_end_ = source_code + length;
}

bool Lexer::IsReadEnd() {
  if (buffer_ptr_ == nullptr || buffer_end_ == nullptr) {
    return true;
  }
  if (buffer_ptr_ >= buffer_end_) {
    return true;
  } else {
    return false;
  }
}

int Lexer::LexToken(Token& return_token) {
LexStart:
  // Initialize a basic token and store the token for preliminary analysis. Then
  // set the initial state.
  BaseToken token_buffer;
  token_buffer.type = BaseToken::Type::START;

  // Set the reading position pointer equal to the buffer pointer.
  char* read_ptr = buffer_ptr_;

LexToken:
  // If the end of the buffer is reached, return.
  if (read_ptr == buffer_end_) {
    return 0;
  } else if (read_ptr > buffer_end_) {
    // Memory out of bounds occurred. Return an error.
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
    case ',':
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
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::OPERATOR;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::OPERATOR ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // The string flag.
    case '"':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::STRING;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::STRING) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // The character flag.
    case '\'':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::CHARACTER;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // Escape character.
    case '\\':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::OPERATOR;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING) {
        // Skip escape characters.
        read_ptr += 2;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::OPERATOR ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Positive and negative numbers.
    case '+':
    case '-':
      if (token_buffer.type == BaseToken::Type::START) {
        if (*(read_ptr + 1) == '0' || *(read_ptr + 1) == '1' ||
            *(read_ptr + 1) == '2' || *(read_ptr + 1) == '3' ||
            *(read_ptr + 1) == '4' || *(read_ptr + 1) == '5' ||
            *(read_ptr + 1) == '6' || *(read_ptr + 1) == '7' ||
            *(read_ptr + 1) == '8' || *(read_ptr + 1) == '9') {
          token_buffer.type = BaseToken::Type::NUMBER;
        } else {
          token_buffer.type = BaseToken::Type::OPERATOR;
        }
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::NUMBER) {
        // Dealing with scientific notation.
        if (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e') {
          read_ptr++;
          goto LexToken;
        } else {
          goto LexEnd;
        }
      } else if (token_buffer.type == BaseToken::Type::OPERATOR ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Decimal point.
    case '.':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::OPERATOR;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::OPERATOR ||
                 token_buffer.type == BaseToken::Type::NUMBER ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // The comment flag.
    case '/':
      if (token_buffer.type == BaseToken::Type::START) {
        if (*(buffer_ptr_ + 1) == '/' || *(buffer_ptr_ + 1) == '*') {
          token_buffer.type = BaseToken::Type::COMMENT;
          read_ptr += 2;
        } else {
          token_buffer.type = BaseToken::Type::OPERATOR;
          read_ptr++;
        }
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::OPERATOR) {
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          read_ptr++;
          goto LexToken;
        }
      } else if (token_buffer.type == BaseToken::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          if (*(read_ptr - 1) == '*') {
            // /**/ style comments, skip all comments.
            buffer_ptr_ = ++read_ptr;
            token_buffer.type = BaseToken::Type::START;
            goto LexToken;
          } else {
            // Non-end comment mark, continue reading until the end mark of the
            // comment.
            read_ptr++;
            goto LexToken;
          }
        } else {
          // // style comments, continue reading until newlines are skipped.
          read_ptr++;
          goto LexToken;
        }
      } else {
        goto LexEnd;
      }

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
      if (token_buffer.type == BaseToken::Type::START) {
        read_ptr++;
        token_buffer.type = BaseToken::Type::NUMBER;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::IDENTIFIER ||
                 token_buffer.type == BaseToken::Type::NUMBER ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Whitespace characters.
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      if (token_buffer.type == BaseToken::Type::START) {
        // Skip whitespace characters.
        read_ptr++;
        buffer_ptr_++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Newlines.
    case '\n':
    case '\0':
      if (token_buffer.type == BaseToken::Type::START) {
        // Skip newlines.
        read_ptr++;
        buffer_ptr_++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          // /**/ style comments, continue reading until the end mark of the
          // comment.
          read_ptr++;
          goto LexToken;
        } else {
          // // style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          token_buffer.type = BaseToken::Type::START;
          goto LexToken;
        }
      } else {
        goto LexEnd;
      }

    // End of code flag.
    case ';':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::SEPARATOR;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    default:
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::IDENTIFIER;
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::IDENTIFIER ||
                 token_buffer.type == BaseToken::Type::NUMBER ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }
  }

LexEnd:
  token_buffer.location = buffer_ptr_;
  token_buffer.length = read_ptr - buffer_ptr_;
  buffer_ptr_ = read_ptr;
  if (token_buffer.type == BaseToken::Type::OPERATOR) {
    // TODO: Handle operator
  }
  // TODO: Handle other token types

  // test
  return_token.location = token_buffer.location;
  return_token.length = token_buffer.length;
  // test
  return 0;
}
}  // namespace Compiler
}  // namespace Aq
