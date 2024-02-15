// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_TOKEN_H_
#define AQ_COMPILER_LEXER_TOKEN_H_

#include "compiler/lexer/lex_map.h"

namespace Aq {
namespace Compiler {
struct Token {
  enum Type {
    START,
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    SEPARATOR,
    NUMBER,
    CHARACTER,
    STRING,
    COMMENT
  };
  enum KeywordType {
    // TODO: Add more keywords.
  };
  enum OperatorType {
    // TODO: Add more operators.
  };
  enum SeparatorType {
    // TODO: Add more separators.
  };
  union Value {
    int Number;
    KeywordType Keyword;
    char* Identifier;
    OperatorType Operator;
    SeparatorType Separator;
    char* Character;
    char* String;
  };

  Type type;
  char* location;
  int length;
  Value value;
};

class TokenMap {
 public:
  TokenMap();
  ~TokenMap();

  Token::KeywordType GetKeywordValue(char* keyword);
  Token::OperatorType GetOperatorValue(char* _operator);
  Token::SeparatorType GetSeparatorValue(char* separator);

 private:
  LexMap<Token::KeywordType> keyword_map;
  LexMap<Token::OperatorType> operator_map;
  LexMap<Token::SeparatorType> separator_map;
};

}  // namespace Compiler
}  // namespace Aq
#endif