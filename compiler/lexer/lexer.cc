// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lexer.h"

#include <cstring>
#include <vector>

#include "compiler/lexer/token.h"

namespace Aq {
namespace Compiler {
Lexer::Lexer(char* source_code) {
LexInit:
  int read_index = 0;
  std::vector<Token> token;
  BaseToken token_buffer;
LexStart:
  if (read_index + 1 >= std::strlen(source_code)) {
    goto LexEnd;
  }
LexToken:
  switch (source_code[read_index]) {
    case '!':
      if (token_buffer.type == BaseToken::Type::OPERATOR ||
          token_buffer.type == BaseToken::Type::CHARACTER ||
          token_buffer.type == BaseToken::Type::STRING ||
          token_buffer.type == BaseToken::Type::COMMENT) {
        token_buffer.token = strcat(token_buffer.token, "!");
        read_index++;
        goto LexStart;
      } else {
        goto LexEnd;
      }
    case '"':
    case '#':
    case '$':
    case '%':
    case '&':
    case '\'':
    case '(':
    case ')':
    case '*':
    case '+':
    case ',':
    case '-':
    case '.':
    case '/':
    case ':':
    case ';':
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
      break;
    default:
      break;
  }
LexEnd:
  if (token_buffer.type == BaseToken::Type::OPERATOR) {
    // LexOperator(token_buffer.token);
    // Token token;
    Token token_arr[100];
    Token* test_tok = token_arr;
  }
  if (read_index < std::strlen(source_code)) {
    goto LexStart;
  }
}
}  // namespace Compiler
}  // namespace Aq
