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
    Auto,
    And,
    Bitand,
    Bitor,
    Bool,
    Break,
    Case,
    Catch,
    Char,
    Class,
    Const,
    Continue,
    Default,
    Do,
    Double,
    Else,
    Enum,
    Export,
    Extern,
    False,
    Float,
    For,
    Friend,
    Goto,
    Import,
    Inline,
    Int,
    Long,
    Namespace,
    New,
    Not,
    Operator,
    Or,
    Private,
    Protected,
    Public,
    Return,
    Short,
    Signed,
    Sizeof,
    Static,
    Struct,
    Switch,
    Template,
    This,
    Thread,
    True,
    Try,
    Typedef,
    Typeid,
    Typename,
    Union,
    Unsigned,
    Using,
    Virtual,
    Void,
    Wchar_t,
    While,
    Xor,
    NONE
  };
  enum OperatorType {
    // TODO: Add more operators.
    NONE
  };
  enum SeparatorType {
    // TODO: Add more separators.
    NONE
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