// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKEN_H_
#define AQ_COMPILER_TOKEN_TOKEN_H_

#include <cstddef>
#include <cstring>
#include <iostream>
#include <string>

namespace Aq {
namespace Compiler {
struct Token {
  enum class Type {
    NONE,
    START,
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    NUMBER,
    CHARACTER,
    STRING,
    COMMENT
  };
  enum class KeywordType {
    NONE = 0,
    As,
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
    Func,
    Float,
    For,
    Friend,
    From,
    Goto,
    If,
    Import,
    Inline,
    Int,
    Long,
    Namespace,
    New,
    Not,
    Number,
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
    Var,
    Virtual,
    Void,
    Wchar_t,
    While,
    Xor,
  };
  enum class OperatorType {
    // TODO: Add more operators.
    NONE = 0,
    l_square,               //[
    r_square,               //]
    l_paren,                //(
    r_paren,                //)
    l_brace,                //{
    r_brace,                //}
    period,                 //.
    ellipsis,               //...
    amp,                    //&
    ampamp,                 //&&
    ampequal,               //&=
    star,                   //*
    starequal,              //*=
    plus,                   //+
    plusplus,               //++
    plusequal,              //+=
    minus,                  //-
    minusminus,             //--
    minusequal,             //-=
    tilde,                  //~
    exclaim,                //!
    exclaimequal,           //!=
    slash,                  ///
    slashequal,             ///=
    percent,                //%
    percentequal,           //%=
    less,                   //<
    lessless,               //<<
    lessequal,              //<=
    lesslessequal,          //<<=
    spaceship,              //<=>
    greater,                //>
    greatergreater,         //>>
    greaterequal,           //>=
    greatergreaterequal,    //>>=
    caret,                  //^
    caretequal,             //^=
    pipe,                   //|
    pipepipe,               //||
    pipeequal,              //|=
    question,               //?
    colon,                  //:
    semi,                   //;
    equal,                  //=
    equalequal,             //==
    comma,                  //,
    lesslessless,           //<<<
    greatergreatergreater,  //>>>
    caretcaret,             //^^
  };
  struct ValueStr {
    char* location;
    std::size_t length;
  };
  union Value {
    ValueStr number;
    KeywordType keyword;
    ValueStr identifier;
    OperatorType oper;
    char character;
    std::string* string;
  };

  Type type = Type::START;
  Value value = Value();

  Token() = default;
  ~Token() = default;

  Token(const Token& other) = default;
  Token(Token&& other) noexcept = default;

  Token& operator=(const Token& other) = default;
  Token& operator=(Token&& other) noexcept = default;

  static std::string GetTokenTypeString(Token::Type type);

  static std::string GetKeywordTypeString(Token::KeywordType keyword);

  static std::string GetOperatorTypeString(Token::OperatorType op);

  // Returns true if the token is an operator and the token's value equals to
  // |oper|.
  bool operator==(const OperatorType& oper) const {
    return type == Type::OPERATOR && value.oper == oper;
  }

  // Returns true if the token is a keyword and the token's value equals to
  // |keyword|.
  bool operator==(const KeywordType& keyword) const {
    return type == Type::KEYWORD && value.keyword == keyword;
  }

  // Returns true if the token's type equals to |type|.
  bool operator==(const Type& ty) const { return type == ty; }
};
}  // namespace Compiler
}  // namespace Aq

#endif