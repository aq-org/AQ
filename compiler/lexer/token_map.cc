/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#include "compiler/lexer/token_map.h"

#include "compiler/compiler.h"
#include "compiler/token/keyword.h"
#include "compiler/token/operator.h"
#include "compiler/token/token.h"
#include "debugger/debugger.h"

namespace Aq {
Compiler::TokenMap::TokenMap() {
  /// TODO: Should be improved
  keyword_map_.Insert("auto", Token::Keyword::Auto);
  keyword_map_.Insert("and", Token::Keyword::And);
  keyword_map_.Insert("bitand", Token::Keyword::Bitand);
  keyword_map_.Insert("bitor", Token::Keyword::Bitor);
  keyword_map_.Insert("bool", Token::Keyword::Bool);
  keyword_map_.Insert("break", Token::Keyword::Break);
  keyword_map_.Insert("case", Token::Keyword::Case);
  keyword_map_.Insert("catch", Token::Keyword::Catch);
  keyword_map_.Insert("char", Token::Keyword::Char);
  keyword_map_.Insert("class", Token::Keyword::Class);
  keyword_map_.Insert("const", Token::Keyword::Const);
  keyword_map_.Insert("continue", Token::Keyword::Continue);
  keyword_map_.Insert("default", Token::Keyword::Default);
  keyword_map_.Insert("do", Token::Keyword::Do);
  keyword_map_.Insert("double", Token::Keyword::Double);
  keyword_map_.Insert("else", Token::Keyword::Else);
  keyword_map_.Insert("enum", Token::Keyword::Enum);
  keyword_map_.Insert("export", Token::Keyword::Export);
  keyword_map_.Insert("extern", Token::Keyword::Extern);
  keyword_map_.Insert("false", Token::Keyword::False);
  keyword_map_.Insert("float", Token::Keyword::Float);
  keyword_map_.Insert("for", Token::Keyword::For);
  keyword_map_.Insert("friend", Token::Keyword::Friend);
  keyword_map_.Insert("goto", Token::Keyword::Goto);
  keyword_map_.Insert("import", Token::Keyword::Import);
  keyword_map_.Insert("inline", Token::Keyword::Inline);
  keyword_map_.Insert("int", Token::Keyword::Int);
  keyword_map_.Insert("long", Token::Keyword::Long);
  keyword_map_.Insert("namespace", Token::Keyword::Namespace);
  keyword_map_.Insert("new", Token::Keyword::New);
  keyword_map_.Insert("not", Token::Keyword::Not);
  keyword_map_.Insert("number", Token::Keyword::Number);
  keyword_map_.Insert("operator", Token::Keyword::Operator);
  keyword_map_.Insert("or", Token::Keyword::Or);
  keyword_map_.Insert("private", Token::Keyword::Private);
  keyword_map_.Insert("protected", Token::Keyword::Protected);
  keyword_map_.Insert("public", Token::Keyword::Public);
  keyword_map_.Insert("return", Token::Keyword::Return);
  keyword_map_.Insert("short", Token::Keyword::Short);
  keyword_map_.Insert("signed", Token::Keyword::Signed);
  keyword_map_.Insert("sizeof", Token::Keyword::Sizeof);
  keyword_map_.Insert("static", Token::Keyword::Static);
  keyword_map_.Insert("string", Token::Keyword::String);
  keyword_map_.Insert("struct", Token::Keyword::Struct);
  keyword_map_.Insert("switch", Token::Keyword::Switch);
  keyword_map_.Insert("template", Token::Keyword::Template);
  keyword_map_.Insert("this", Token::Keyword::This);
  keyword_map_.Insert("thread", Token::Keyword::Thread);
  keyword_map_.Insert("true", Token::Keyword::True);
  keyword_map_.Insert("try", Token::Keyword::Try);
  keyword_map_.Insert("typedef", Token::Keyword::Typedef);
  keyword_map_.Insert("typeid", Token::Keyword::Typeid);
  keyword_map_.Insert("typename", Token::Keyword::Typename);
  keyword_map_.Insert("union", Token::Keyword::Union);
  keyword_map_.Insert("unsigned", Token::Keyword::Unsigned);
  keyword_map_.Insert("using", Token::Keyword::Using);
  keyword_map_.Insert("virtual", Token::Keyword::Virtual);
  keyword_map_.Insert("void", Token::Keyword::Void);
  keyword_map_.Insert("wchar_t", Token::Keyword::Wchar_t);
  keyword_map_.Insert("while", Token::Keyword::While);
  keyword_map_.Insert("xor", Token::Keyword::Xor);

  operator_map_.Insert("[", Token::Operator::l_square);
  operator_map_.Insert("]", Token::Operator::r_square);
  operator_map_.Insert("(", Token::Operator::l_paren);
  operator_map_.Insert(")", Token::Operator::r_paren);
  operator_map_.Insert("{", Token::Operator::l_brace);
  operator_map_.Insert("}", Token::Operator::r_brace);
  operator_map_.Insert(".", Token::Operator::period);
  operator_map_.Insert("...", Token::Operator::ellipsis);
  operator_map_.Insert("&", Token::Operator::amp);
  operator_map_.Insert("&&", Token::Operator::ampamp);
  operator_map_.Insert("&=", Token::Operator::ampequal);
  operator_map_.Insert("*", Token::Operator::star);
  operator_map_.Insert("*=", Token::Operator::starequal);
  operator_map_.Insert("+", Token::Operator::plus);
  operator_map_.Insert("++", Token::Operator::plusplus);
  operator_map_.Insert("+=", Token::Operator::plusequal);
  operator_map_.Insert("-", Token::Operator::minus);
  operator_map_.Insert("->", Token::Operator::arrow);
  operator_map_.Insert("--", Token::Operator::minusminus);
  operator_map_.Insert("-=", Token::Operator::minusequal);
  operator_map_.Insert("~", Token::Operator::tilde);
  operator_map_.Insert("!", Token::Operator::exclaim);
  operator_map_.Insert("!=", Token::Operator::exclaimequal);
  operator_map_.Insert("/", Token::Operator::slash);
  operator_map_.Insert("/=", Token::Operator::slashequal);
  operator_map_.Insert("%", Token::Operator::percent);
  operator_map_.Insert("%=", Token::Operator::percentequal);
  operator_map_.Insert("<", Token::Operator::less);
  operator_map_.Insert("<<", Token::Operator::lessless);
  operator_map_.Insert("<=", Token::Operator::lessequal);
  operator_map_.Insert("<<=", Token::Operator::lesslessequal);
  operator_map_.Insert("<=>", Token::Operator::spaceship);
  operator_map_.Insert(">", Token::Operator::greater);
  operator_map_.Insert(">>", Token::Operator::greatergreater);
  operator_map_.Insert(">=", Token::Operator::greaterequal);
  operator_map_.Insert(">>=", Token::Operator::greatergreaterequal);
  operator_map_.Insert("^", Token::Operator::caret);
  operator_map_.Insert("^=", Token::Operator::caretequal);
  operator_map_.Insert("|", Token::Operator::pipe);
  operator_map_.Insert("||", Token::Operator::pipepipe);
  operator_map_.Insert("|=", Token::Operator::pipeequal);
  operator_map_.Insert("?", Token::Operator::question);
  operator_map_.Insert(":", Token::Operator::colon);
  operator_map_.Insert(";", Token::Operator::semi);
  operator_map_.Insert("=", Token::Operator::equal);
  operator_map_.Insert("==", Token::Operator::equalequal);
  operator_map_.Insert(",", Token::Operator::comma);
  operator_map_.Insert("#", Token::Operator::hash);
  operator_map_.Insert("##", Token::Operator::hashhash);
  operator_map_.Insert("#@", Token::Operator::hashat);
  operator_map_.Insert(".*", Token::Operator::periodstar);
  operator_map_.Insert("->*", Token::Operator::arrowstar);
  operator_map_.Insert("::", Token::Operator::coloncolon);
  operator_map_.Insert("@", Token::Operator::at);
  operator_map_.Insert("<<<", Token::Operator::lesslessless);
  operator_map_.Insert(">>>", Token::Operator::greatergreatergreater);
  operator_map_.Insert("^^", Token::Operator::caretcaret);
}
Compiler::TokenMap::~TokenMap() = default;

Compiler::Token::Keyword Compiler::TokenMap::GetKeywordValue(
    std::string keyword) {
  /// TODO: Should be improved
  /// return keyword_map_.Find(keyword);
}
Compiler::Token::Operator Compiler::TokenMap::GetOperatorValue(
    std::string _operator) {
  /// TODO: Should be improved
  /// return operator_map_.Find(_operator);
}
}  // namespace Aq