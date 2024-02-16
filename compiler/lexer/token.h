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
    NUMBER,
    CHARACTER,
    STRING,
    COMMENT
  };
  enum KeywordType {
    NONKEYWORD = 0,
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
    String,
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
  };
  enum OperatorType {
    // TODO: Add more operators.
    NONOPERATOR = 0,
    l_square,
    r_square,
    l_paren,
    r_paren,
    l_brace,
    r_brace,
    period,
    ellipsis,
    amp,
    ampamp,
    ampequal,
    star,
    starequal,
    plus,
    plusplus,
    plusequal,
    minus,
    arrow,
    minusminus,
    minusequal,
    tilde,
    exclaim,
    exclaimequal,
    slash,
    slashequal,
    percent,
    percentequal,
    less,
    lessless,
    lessequal,
    lesslessequal,
    spaceship,
    greater,
    greatergreater,
    greaterequal,
    greatergreaterequal,
    caret,
    caretequal,
    pipe,
    pipepipe,
    pipeequal,
    question,
    colon,
    semi,
    equal,
    equalequal,
    comma,
    hash,
    hashhash,
    hashat,
    periodstar,
    arrowstar,
    coloncolon,
    at,
    lesslessless,
    greatergreatergreater,
    caretcaret,
  };
  union Value {
    int Number;
    KeywordType Keyword;
    char* Identifier;
    OperatorType Operator;
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

 private:
  LexMap<Token::KeywordType> keyword_map;
  LexMap<Token::OperatorType> operator_map;
};

}  // namespace Compiler
}  // namespace Aq
#endif