// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include <string>

#include "compiler/compiler.h"
#include "compiler/token/keyword.h"
#include "compiler/token/operator.h"
#include "compiler/token/token.h"
#include "debugger/debugger.h"

namespace Aq {
int Compiler::Lexer::LexToken(Token& return_token) {
  // Set the return token type to start.
  return_token.type = Token::Type::START;

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
      Token::Type gen_op_next_type[4] = {Token::Type::OPERATOR, Token::Type::CHARACTER,
                                  Token::Type::STRING, Token::Type::COMMENT};
      if (ProcessToken(return_token, Token::Type::OPERATOR, gen_op_next_type, 4)) {
        read_ptr++;
        goto LexStart;
      }
      goto LexEnd;

    // The string flag.
    case '"':
      Token::Type str_next_type[2] = {Token::Type::STRING, Token::Type::COMMENT};
      if (ProcessToken(return_token, Token::Type::STRING, str_next_type, 2)) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::STRING) {
        read_ptr++;
      }
      goto LexEnd;

    // The character flag.
    case '\'':
      Token::Type char_next_type[2] = {Token::Type::STRING, Token::Type::COMMENT};
      if (ProcessToken(return_token, Token::Type::CHARACTER, char_next_type, 2)) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER) {
        read_ptr++;
      }
      goto LexEnd;

    // Escape character.
    case '\\':
      Token::Type escape_next_type[2] = {Token::Type::OPERATOR, Token::Type::COMMENT};
      if (return_token.type == Token::Type::CHARACTER ||
          return_token.type == Token::Type::STRING) {
        // Skip escape characters.
        if (read_ptr + 2 <= buffer_end_) {
          read_ptr += 2;
        } else {
          read_ptr++;
        }
        goto LexStart;
      } else if (ProcessToken(return_token, Token::Type::OPERATOR, escape_next_type,
                              2)) {
        read_ptr++;
        goto LexStart;
      }
      goto LexEnd;

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
        goto LexStart;
      } else if (return_token.type == Token::Type::NUMBER) {
        // Dealing with scientific notation.
        if (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e') {
          read_ptr++;
          goto LexStart;
        } else {
          goto LexEnd;
        }
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Decimal point.
    case '.':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
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
            goto LexStart;
          } else {
            read_ptr++;
            goto LexStart;
          }
        } else {
          return_token.type = Token::Type::OPERATOR;
          read_ptr++;
        }
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::OPERATOR) {
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          read_ptr++;
          goto LexStart;
        }
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          if (*(read_ptr - 1) == '*') {
            // /**/ style comments, skip all comments.
            buffer_ptr_ = ++read_ptr;
            return_token.type = Token::Type::START;
            goto LexStart;
          } else {
            // Non-end comment mark, continue reading until the end mark of the
            // comment.
            read_ptr++;
            goto LexStart;
          }
        } else {
          // // style comments, continue reading until newlines are skipped.
          read_ptr++;
          goto LexStart;
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
        goto LexStart;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
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
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Newlines.
    case '\n':
      if (return_token.type == Token::Type::START) {
        // Skip newlines.
        read_ptr++;
        buffer_ptr_++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          // /**/ style comments, continue reading until the end mark of the
          // comment.
          read_ptr++;
          goto LexStart;
        } else {
          // // style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          return_token.type = Token::Type::START;
          goto LexStart;
        }
      } else {
        goto LexEnd;
      }

    // EOF.
    case '\0':
      goto LexEnd;

    // Separator flag.
    case ',':
    case ';':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexEnd;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    default:
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::IDENTIFIER;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }
  }

LexEnd:
  // Meaningless token.
  if (return_token.type == Token::Type::START ||
      return_token.type == Token::Type::COMMENT) {
    return_token.type = Token::Type::NONE;
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
      case Token::Type::IDENTIFIER:
        return_token.value.keyword =
            token_map_.GetKeywordValue(std::string(location, length));
        if (return_token.value.keyword == Token::Keyword::NONE) {
          return_token.value.identifier = value;
          break;
        }
        return_token.type = Token::Type::KEYWORD;
        break;

      case Token::Type::CHARACTER:
        return_token.value.character = value;
        break;

      case Token::Type::STRING:
        return_token.value.string = value;
        break;

      case Token::Type::OPERATOR:
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

      case Token::Type::NUMBER:
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
  } else {
    return false;
  }
}

bool Compiler::Lexer::ProcessToken(Token& token, Token::Type start_type,
                                   Token::Type next_type[], int next_type_size) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    return true;
  }
  for (int i = 0; i < next_type_size; i++) {
    if (token.type == next_type[i]) {
      return true;
    }
  }
  return false;
}
}  // namespace Aq