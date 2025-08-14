// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "lexer/lexer.h"

#include <cstring>
#include <vector>

#include "logging/logging.h"
#include "token/token.h"
#include "token/tokenmap.h"

namespace Aq {

bool Lexer::IsReadEnd() const {
  if (code_ptr_ >= code_end_) {
    return true;
  } else {
    return false;
  }
}

int Lexer::LexToken(Token last_token, Token& return_token) {
  return_token.type = Token::Type::START;
  char* current_location = code_ptr_;

LexStart:
  if (current_location > code_end_) {
    INTERNAL_ERROR("Memory out of bounds during lexical analysis.");
    return -1;
  }

  switch (*current_location) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
      if (LexBrackets(return_token)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;

    // General operators.
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '*':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '^':
    case '`':
    case '|':
    case '~':
      if (LexGeneralOperators(return_token)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;

    // The string flag.
    case '"': {
      bool is_string = return_token.type == Token::Type::STRING;
      if (LexString(return_token)) {
        current_location++;
        if (is_string) goto LexEnd;
        goto LexStart;
      }
      goto LexEnd;
    }

    // The character flag.
    case '\'': {
      bool is_charater = return_token.type == Token::Type::CHARACTER;
      if (LexCharacter(return_token)) {
        current_location++;
        if (is_charater) goto LexEnd;
        goto LexStart;
      }
      goto LexEnd;
    }

      // Escape character.
    case '\\':
      if (LexEscapeCharacter(return_token, current_location)) goto LexStart;
      goto LexEnd;

    // Positive and negative numbers.
    case '+':
    case '-':
      if (LexSignedNumbers(return_token, current_location, last_token))
        goto LexStart;
      goto LexEnd;

    // Decimal point.
    case '.':
      if (LexDecimalPoint(return_token)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;

    // The comment flag.
    case '/':
      if (LexComment(return_token, current_location)) goto LexStart;
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
      if (LexNumbers(return_token)) {
        current_location++;
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
      if (LexWhitespace(return_token)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;

    // Newlines.
    case '\n':
      if (LexNewlines(return_token, current_location)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;

    // EOF.
    case '\0':
      goto LexEnd;

    // Separator flag.
    case ',':
    case ';':
      if (LexSeparator(return_token, current_location)) goto LexStart;
      goto LexEnd;

    default:
      if (LexDefault(return_token)) {
        current_location++;
        goto LexStart;
      }
      goto LexEnd;
  }

LexEnd:
  // Meaningless token.
  if (return_token.type == Token::Type::START ||
      return_token.type == Token::Type::COMMENT) {
    return_token.type = Token::Type::NONE;
  } else {
    // Meaningful token. Determine the specific token information.
    HandleFinalToken(return_token, current_location);
  }
  code_ptr_ = current_location;
  return 0;
}

bool Lexer::LexBrackets(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

bool Lexer::LexGeneralOperators(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    return true;
  } else if (token.type == Token::Type::OPERATOR ||
             token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

bool Lexer::LexString(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::STRING;
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::COMMENT ||
             token.type == Token::Type::STRING) {
    return true;
  }
  return false;
}
bool Lexer::LexCharacter(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::CHARACTER;
    return true;
  } else if (token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT ||
             token.type == Token::Type::CHARACTER) {
    return true;
  }
  return false;
}

bool Lexer::LexEscapeCharacter(Token& token, char*& current_location) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    current_location++;
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING) {
    if (current_location + 2 <= code_end_) {
      if (IsHexEscapeCharacter(current_location + 1)) {
        LexHexEscapeCharacter(token, current_location);
      } else if (IsOctEscapeCharacter(current_location + 1)) {
        LexOctEscapeCharacter(token, current_location);
      } else {
        LexGeneralEscapeCharacter(token, current_location);
      }
      return true;
    } else {
      current_location++;
      return true;
    }
  } else if (token.type == Token::Type::OPERATOR ||
             token.type == Token::Type::COMMENT) {
    current_location++;
    return true;
  }
  return false;
}

bool Lexer::IsHexEscapeCharacter(char* current_location) {
  bool has_hex_flag = *current_location == 'x';
  if (has_hex_flag) {
    if (current_location + 1 <= code_end_ &&
        isxdigit(*(current_location + 1))) {
      return true;
    } else {
      LOGGING_ERROR(
          "Invalid hex escape character: 'x' must be followed by a "
          "hexadecimal digit.");
    }
  }
  return false;
}

std::size_t Lexer::GetHexEscapeCharacterLength(char* current_location) {
  std::size_t length = 0;
  while (current_location <= code_end_ && isxdigit(*current_location)) {
    length++;
    current_location++;
  }
  return length;
}

void Lexer::LexHexEscapeCharacter(Token& token, char*& current_location) {
  std::size_t length = GetHexEscapeCharacterLength(current_location + 2) + 1;
  std::vector<char> hex_str(length);
  for (std::size_t i = 0; i < length; ++i) {
    hex_str[i] = *(current_location + 2 + i);
  }
  hex_str.push_back('\0');

  // Converts the hex string to a character.
  *current_location = static_cast<char>(strtol(hex_str.data(), nullptr, 16));

  // Moves the current location forward by the length of the hex escape
  // character, and adjust the code end pointer.
  current_location++;
  memmove(current_location, current_location + length,
          code_end_ - current_location - length);
  code_end_ -= length;
}

bool Lexer::IsOctEscapeCharacter(char* current_location) {
  return *current_location >= '0' && *current_location < '8';
}

std::size_t Lexer::GetOctEscapeCharacterLength(char* current_location) {
  std::size_t length = 0;
  while (current_location <= code_end_ && *current_location >= '0' &&
         *current_location < '8') {
    length++;
    current_location++;
  }
  return length;
}

void Lexer::LexOctEscapeCharacter(Token& token, char*& current_location) {
  std::size_t length = GetOctEscapeCharacterLength(current_location + 1);
  std::vector<char> oct_str(length);
  for (std::size_t i = 0; i < length; ++i) {
    oct_str[i] = *(current_location + 1 + i);
  }
  oct_str.push_back('\0');

  // Converts the octal string to a character.
  *current_location = static_cast<char>(strtol(oct_str.data(), nullptr, 8));

  // Moves the current location forward by the length of the hex escape
  // character, and adjust the code end pointer.
  current_location++;
  memmove(current_location, current_location + length,
          code_end_ - current_location - length);
  code_end_ -= length;
}

void Lexer::LexGeneralEscapeCharacter(Token& token, char*& current_location) {
  switch (*(current_location + 1)) {
    case 'n':
      *current_location = '\n';
      break;
    case 't':
      *current_location = '\t';
      break;
    case 'r':
      *current_location = '\r';
      break;
    case 'b':
      *current_location = '\b';
      break;
    case 'f':
      *current_location = '\f';
      break;
    case 'v':
      *current_location = '\v';
      break;
    case 'a':
      *current_location = '\a';
      break;
    case '\\':
      *current_location = '\\';
      break;
    case '\'':
      *current_location = '\'';
      break;
    case '\"':
      *current_location = '\"';
      break;
    case '\?':
      *current_location = '\?';
      break;
    case '0':
      *current_location = '\0';
      break;
    default:
      const char current_escape_str[3] = {'\\', *current_location, '\0'};
      LOGGING_ERROR("Unknown escape character '\\" +
                    std::string(current_escape_str) + "'.");
      break;
  }

  memmove(current_location + 1, current_location + 2,
          code_end_ - current_location - 2);
  code_end_ -= 1;
  *code_end_ = '\0';

  current_location++;
}

void Lexer::LexSignedNumbersAtTheStart(Token& token, char*& current_location,
                                       Token& last_token) {
  if (last_token.type != Token::Type::IDENTIFIER &&
      last_token.type != Token::Type::NUMBER &&
      (*(current_location + 1) == '0' || *(current_location + 1) == '1' ||
       *(current_location + 1) == '2' || *(current_location + 1) == '3' ||
       *(current_location + 1) == '4' || *(current_location + 1) == '5' ||
       *(current_location + 1) == '6' || *(current_location + 1) == '7' ||
       *(current_location + 1) == '8' || *(current_location + 1) == '9')) {
    token.type = Token::Type::NUMBER;
  } else {
    token.type = Token::Type::OPERATOR;
  }
  current_location++;
}

bool Lexer::LexScientificNotation(char*& current_location) {
  if (*(current_location - 1) == 'E' || *(current_location - 1) == 'e') {
    current_location++;
    return true;
  }
  return false;
}

bool Lexer::LexSignedNumbers(Token& token, char*& current_location,
                             Token& last_token) {
  if (token.type == Token::Type::START) {
    LexSignedNumbersAtTheStart(token, current_location, last_token);
    return true;
  } else if (token.type == Token::Type::NUMBER) {
    return LexScientificNotation(current_location);
  } else if (token.type == Token::Type::OPERATOR ||
             token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    current_location++;
    return true;
  }
  return false;
}

bool Lexer::LexDecimalPoint(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    return true;
  } else if (token.type == Token::Type::OPERATOR ||
             token.type == Token::Type::NUMBER ||
             token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

void Lexer::LexCommentAtTheStart(Token& token, char*& current_location) {
  if (*(code_ptr_ + 1) == '/' || *(code_ptr_ + 1) == '*') {
    token.type = Token::Type::COMMENT;
    if (current_location + 2 <= code_end_) {
      current_location += 2;
    } else {
      // If the comment is at the end of the code, just skip it.
      current_location++;
    }
  } else {
    token.type = Token::Type::OPERATOR;
    current_location++;
  }
}

void Lexer::LexCommentAtTheEnd(Token& token, char*& current_location) {
  if (*(code_ptr_ + 1) == '*') {
    if (*(current_location - 1) == '*') {
      // /**/ style comments, skip all comments.
      code_ptr_ = ++current_location;
      token.type = Token::Type::START;
    } else {
      // Non-end comment mark, continue reading until the end mark of the
      // comment.
      current_location++;
    }
  } else {
    // // style comments, continue reading until newlines are skipped.
    current_location++;
  }
}

bool Lexer::LexComment(Token& token, char*& current_location) {
  if (token.type == Token::Type::START) {
    LexCommentAtTheStart(token, current_location);
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING) {
    current_location++;
    return true;
  } else if (token.type == Token::Type::OPERATOR) {
    if (*(current_location + 1) == '/' || *(current_location + 1) == '*')
      // Skips combining this operator when it can form a comment with the next
      // character.
      return false;

    current_location++;
    return true;
  } else if (token.type == Token::Type::COMMENT) {
    LexCommentAtTheEnd(token, current_location);
    return true;
  }
  return false;
}

bool Lexer::LexNumbers(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::NUMBER;
    return true;
  } else if (token.type == Token::Type::IDENTIFIER ||
             token.type == Token::Type::NUMBER ||
             token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

bool Lexer::LexWhitespace(Token& token) {
  if (token.type == Token::Type::START) {
    // Skip whitespace characters.
    code_ptr_++;
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

bool Lexer::LexNewlines(Token& token, char*& current_location) {
  if (token.type == Token::Type::START) {
    // Skip newlines.
    code_ptr_++;
    return true;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING) {
    return true;
  } else if (token.type == Token::Type::COMMENT) {
    if (*(code_ptr_ + 1) == '*') {
      // /**/ style comments, continue reading until the end mark of the
      // comment.
      return true;
    } else {
      // // style comments, skip all comments.
      code_ptr_ = current_location + 1;
      token.type = Token::Type::START;
      return true;
    }
  }
  return false;
}

bool Lexer::LexSeparator(Token& token, char*& current_location) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::OPERATOR;
    current_location++;
    return false;
  } else if (token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    current_location++;
    return true;
  }
  return false;
}

bool Lexer::LexDefault(Token& token) {
  if (token.type == Token::Type::START) {
    token.type = Token::Type::IDENTIFIER;
    return true;
  } else if (token.type == Token::Type::IDENTIFIER ||
             token.type == Token::Type::NUMBER ||
             token.type == Token::Type::CHARACTER ||
             token.type == Token::Type::STRING ||
             token.type == Token::Type::COMMENT) {
    return true;
  }
  return false;
}

void Lexer::HandleFinalToken(Token& token, char*& current_location) {
  char* location = code_ptr_;
  LOGGING_INFO(location);
  std::size_t length = current_location - code_ptr_;
  code_ptr_ = current_location;

  // Handle the detailed information of tokens.
  Token::ValueStr value;
  value.location = location;
  value.length = length;

  switch (token.type) {
    case Token::Type::IDENTIFIER:
      token.value.keyword =
          token_map_.GetKeywordValue(std::string(location, length));
      if (token.value.keyword == Token::KeywordType::NONE) {
        token.value.identifier = value;
        break;
      }
      token.type = Token::Type::KEYWORD;
      break;

    case Token::Type::CHARACTER:
      token.value.character = value.location[1];
      break;

    case Token::Type::STRING:
      if (location == nullptr || length < 2)
        INTERNAL_ERROR("Unexpected location or string length.");
      token.value.string = new std::string(location + 1, length - 2);
      break;

    case Token::Type::OPERATOR:
      token.value.oper =
          token_map_.GetOperatorValue(std::string(location, length));
      while (token.value.oper == Token::OperatorType::NONE && length > 1) {
        LOGGING_INFO("Trying to shorten the operator: " +
                     std::string(location, length));
        length--;
        code_ptr_--;
        current_location--;
        token.value.oper =
            token_map_.GetOperatorValue(std::string(location, length));
      }
      break;

    case Token::Type::NUMBER:
      token.value.number = value;
      break;

    default:
      INTERNAL_ERROR("Unknown token type: " +
                     std::to_string(static_cast<int>(token.type)));
      return;
  }
}

}  // namespace Aq