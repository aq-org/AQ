// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include "compiler/lexer/token.h"

namespace Aq {
namespace Compiler {
int Lexer::LexToken(Token& return_token) {
LexStart:
  BaseToken token_buffer;
  token_buffer.type = BaseToken::Type::START;

  char* read_ptr = buffer_ptr_;

  if (read_ptr == buffer_end_) {
    goto LexEnd;
  } else if (read_ptr > buffer_end_) {
    // TODO: Handle error
    return -1;
  }
LexToken:
  switch (*read_ptr) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '(':
    case ')':
    case '*':
    case ',':
    case '/':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case '\\':
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
      } else if (token_buffer.type == BaseToken::Type::OPERATOR ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    case '"':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::STRING;
        read_ptr++;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::STRING) {
        if (*(read_ptr - 1) == '\\') {
          read_ptr++;
          goto LexToken;
        } else {
          read_ptr++;
          goto LexEnd;
        }
      } else {
        goto LexEnd;
      }

    case '\'':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::CHARACTER;
        read_ptr++;
      } else if (token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else if (token_buffer.type == BaseToken::Type::CHARACTER) {
        if (*(read_ptr - 1) == '\\') {
          read_ptr++;
          goto LexToken;
        } else {
          read_ptr++;
          goto LexEnd;
        }
      } else {
        goto LexEnd;
      }

    case '+':
    case '-':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::OPERATOR;
        read_ptr++;
      } else if (token_buffer.type == BaseToken::Type::NUMBER) {
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

    case '.':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::OPERATOR;
        read_ptr++;
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
        token_buffer.type = BaseToken::Type::NUMBER;
      } else if (token_buffer.type == BaseToken::Type::NUMBER ||
                 token_buffer.type == BaseToken::Type::CHARACTER ||
                 token_buffer.type == BaseToken::Type::STRING ||
                 token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        goto LexEnd;
      }

    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      if (token_buffer.type == BaseToken::Type::CHARACTER ||
          token_buffer.type == BaseToken::Type::STRING ||
          token_buffer.type == BaseToken::Type::COMMENT) {
        read_ptr++;
        goto LexToken;
      } else {
        read_ptr++;
        buffer_ptr_++;
        goto LexToken;
      }

    case ';':
      if (token_buffer.type == BaseToken::Type::START) {
        token_buffer.type = BaseToken::Type::SEPARATOR;
        read_ptr++;
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
    } else if (token_buffer.type == BaseToken::Type::NUMBER ||
               token_buffer.type == BaseToken::Type::CHARACTER ||
               token_buffer.type == BaseToken::Type::STRING ||
               token_buffer.type == BaseToken::Type::COMMENT) {
      read_ptr++;
      goto LexToken;
    } else {
      goto LexEnd;
    }
}
LexEnd : if (token_buffer.type == BaseToken::Type::OPERATOR) {
  // TODO: Handle operator
}
// TODO: Handle other token types
return 0;
}
}  // namespace Compiler
}  // namespace Aq
