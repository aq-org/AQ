// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include "compiler/lexer/token.h"

namespace Aq {
namespace Compiler {
Lexer::Lexer(char* source_code, std::size_t length) {
  buffer_ptr_ = source_code;
  buffer_end_ = source_code + length;
}
Lexer::~Lexer() = default;

bool Lexer::IsReadEnd() {
  if (buffer_ptr_ >= buffer_end_) {
    return true;
  } else {
    return false;
  }
}

int Lexer::LexToken(Token& return_token) {
LexStart:
  return_token.type = Token::Type::START;

  // Set the reading position pointer equal to the buffer pointer.
  char* read_ptr = buffer_ptr_;

LexToken:
  // Memory out of bounds occurred. Return an error.
  if (read_ptr > buffer_end_) {
    buffer_ptr_ = read_ptr;
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
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // The string flag.
    case '"':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::STRING;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // The character flag.
    case '\'':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::CHARACTER;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // Escape character.
    case '\\':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        // Skip escape characters.
        if (read_ptr + 2 <= buffer_end_) {
          read_ptr += 2;
          goto LexToken;
        } else {
          read_ptr++;
          goto LexToken;
        }
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Positive and negative numbers.
    case '+':
    case '-':
      if (return_token.type == Token::Type::START) {
        if (*(read_ptr + 1) == '0' || *(read_ptr + 1) == '1' ||
            *(read_ptr + 1) == '2' || *(read_ptr + 1) == '3' ||
            *(read_ptr + 1) == '4' || *(read_ptr + 1) == '5' ||
            *(read_ptr + 1) == '6' || *(read_ptr + 1) == '7' ||
            *(read_ptr + 1) == '8' || *(read_ptr + 1) == '9') {
          return_token.type = Token::Type::NUMBER;
        } else {
          return_token.type = Token::Type::OPERATOR;
        }
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::NUMBER) {
        // Dealing with scientific notation.
        if (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e') {
          read_ptr++;
          goto LexToken;
        } else {
          goto LexEnd;
        }
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    case ',':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::SEPARATOR;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Decimal point.
    case '.':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::SEPARATOR;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // The comment flag.
    case '/':
      if (return_token.type == Token::Type::START) {
        if (*(buffer_ptr_ + 1) == '/' || *(buffer_ptr_ + 1) == '*') {
          return_token.type = Token::Type::COMMENT;
          if (read_ptr + 2 <= buffer_end_) {
            read_ptr += 2;
            goto LexToken;
          } else {
            read_ptr++;
            goto LexToken;
          }
        } else {
          return_token.type = Token::Type::OPERATOR;
          read_ptr++;
        }
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::OPERATOR) {
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          read_ptr++;
          goto LexToken;
        }
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          if (*(read_ptr - 1) == '*') {
            // /**/ style comments, skip all comments.
            buffer_ptr_ = ++read_ptr;
            return_token.type = Token::Type::START;
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
      if (return_token.type == Token::Type::START) {
        read_ptr++;
        return_token.type = Token::Type::NUMBER;
        goto LexToken;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
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
      if (return_token.type == Token::Type::START) {
        // Skip whitespace characters.
        read_ptr++;
        buffer_ptr_++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    // Newlines.
    case '\n':
      if (return_token.type == Token::Type::START) {
        // Skip newlines.
        read_ptr++;
        buffer_ptr_++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          // /**/ style comments, continue reading until the end mark of the
          // comment.
          read_ptr++;
          goto LexToken;
        } else {
          // // style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          return_token.type = Token::Type::START;
          goto LexToken;
        }
      } else {
        goto LexEnd;
      }

    case '\0':
      goto LexEnd;

    // End of code flag.
    case ';':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::SEPARATOR;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    default:
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::IDENTIFIER;
        read_ptr++;
        goto LexToken;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }
  }

LexEnd:
  if (return_token.type == Token::Type::START ||
      return_token.type == Token::Type::COMMENT) {
    return_token.location = read_ptr;
    return_token.length = 0;
    buffer_ptr_ = read_ptr;
    return_token.location = return_token.location;
    return_token.length = return_token.length;
    return 0;
  } else {
    return_token.location = buffer_ptr_;
    return_token.length = read_ptr - buffer_ptr_;
    buffer_ptr_ = read_ptr;
    if (return_token.type == Token::Type::OPERATOR) {
      // TODO: Handle operator
    }
    // TODO: Handle other token types

    // test
    return_token.location = return_token.location;
    return_token.length = return_token.length;
    // test
    return 0;
  }
}
}  // namespace Compiler
}  // namespace Aq
