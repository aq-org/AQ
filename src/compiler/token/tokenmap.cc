// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/token/tokenmap.h"

#include <string>
#include <unordered_map>

#include "compiler/logging/logging.h"
#include "compiler/token/token.h"


namespace Aq {
namespace Compiler {
TokenMap::TokenMap() {
  keyword_map.insert({"as", Token::KeywordType::As});
  keyword_map.insert({"auto", Token::KeywordType::Auto});
  keyword_map.insert({"and", Token::KeywordType::And});
  keyword_map.insert({"bitand", Token::KeywordType::Bitand});
  keyword_map.insert({"bitor", Token::KeywordType::Bitor});
  keyword_map.insert({"bool", Token::KeywordType::Bool});
  keyword_map.insert({"break", Token::KeywordType::Break});
  keyword_map.insert({"case", Token::KeywordType::Case});
  keyword_map.insert({"catch", Token::KeywordType::Catch});
  keyword_map.insert({"char", Token::KeywordType::Char});
  keyword_map.insert({"class", Token::KeywordType::Class});
  keyword_map.insert({"const", Token::KeywordType::Const});
  keyword_map.insert({"continue", Token::KeywordType::Continue});
  keyword_map.insert({"default", Token::KeywordType::Default});
  keyword_map.insert({"do", Token::KeywordType::Do});
  keyword_map.insert({"double", Token::KeywordType::Double});
  keyword_map.insert({"else", Token::KeywordType::Else});
  keyword_map.insert({"enum", Token::KeywordType::Enum});
  keyword_map.insert({"export", Token::KeywordType::Export});
  keyword_map.insert({"extern", Token::KeywordType::Extern});
  keyword_map.insert({"false", Token::KeywordType::False});
  keyword_map.insert({"func", Token::KeywordType::Func});
  keyword_map.insert({"float", Token::KeywordType::Float});
  keyword_map.insert({"for", Token::KeywordType::For});
  keyword_map.insert({"friend", Token::KeywordType::Friend});
  keyword_map.insert({"from", Token::KeywordType::From});
  keyword_map.insert({"goto", Token::KeywordType::Goto});
  keyword_map.insert({"if", Token::KeywordType::If});
  keyword_map.insert({"import", Token::KeywordType::Import});
  keyword_map.insert({"inline", Token::KeywordType::Inline});
  keyword_map.insert({"int", Token::KeywordType::Int});
  keyword_map.insert({"long", Token::KeywordType::Long});
  keyword_map.insert({"namespace", Token::KeywordType::Namespace});
  keyword_map.insert({"new", Token::KeywordType::New});
  keyword_map.insert({"not", Token::KeywordType::Not});
  keyword_map.insert({"number", Token::KeywordType::Number});
  keyword_map.insert({"operator", Token::KeywordType::Operator});
  keyword_map.insert({"or", Token::KeywordType::Or});
  keyword_map.insert({"private", Token::KeywordType::Private});
  keyword_map.insert({"protected", Token::KeywordType::Protected});
  keyword_map.insert({"public", Token::KeywordType::Public});
  keyword_map.insert({"return", Token::KeywordType::Return});
  keyword_map.insert({"short", Token::KeywordType::Short});
  keyword_map.insert({"signed", Token::KeywordType::Signed});
  keyword_map.insert({"sizeof", Token::KeywordType::Sizeof});
  keyword_map.insert({"static", Token::KeywordType::Static});
  keyword_map.insert({"string", Token::KeywordType::String});
  keyword_map.insert({"struct", Token::KeywordType::Struct});
  keyword_map.insert({"switch", Token::KeywordType::Switch});
  keyword_map.insert({"template", Token::KeywordType::Template});
  keyword_map.insert({"this", Token::KeywordType::This});
  keyword_map.insert({"thread", Token::KeywordType::Thread});
  keyword_map.insert({"true", Token::KeywordType::True});
  keyword_map.insert({"try", Token::KeywordType::Try});
  keyword_map.insert({"typedef", Token::KeywordType::Typedef});
  keyword_map.insert({"typeid", Token::KeywordType::Typeid});
  keyword_map.insert({"typename", Token::KeywordType::Typename});
  keyword_map.insert({"union", Token::KeywordType::Union});
  keyword_map.insert({"unsigned", Token::KeywordType::Unsigned});
  keyword_map.insert({"using", Token::KeywordType::Using});
  keyword_map.insert({"virtual", Token::KeywordType::Virtual});
  keyword_map.insert({"var", Token::KeywordType::Var});
  keyword_map.insert({"void", Token::KeywordType::Void});
  keyword_map.insert({"wchar_t", Token::KeywordType::Wchar_t});
  keyword_map.insert({"while", Token::KeywordType::While});
  keyword_map.insert({"xor", Token::KeywordType::Xor});

  operator_map.insert({"[", Token::OperatorType::l_square});
  operator_map.insert({"]", Token::OperatorType::r_square});
  operator_map.insert({"(", Token::OperatorType::l_paren});
  operator_map.insert({")", Token::OperatorType::r_paren});
  operator_map.insert({"{", Token::OperatorType::l_brace});
  operator_map.insert({"}", Token::OperatorType::r_brace});
  operator_map.insert({".", Token::OperatorType::period});
  operator_map.insert({"...", Token::OperatorType::ellipsis});
  operator_map.insert({"&", Token::OperatorType::amp});
  operator_map.insert({"&&", Token::OperatorType::ampamp});
  operator_map.insert({"&=", Token::OperatorType::ampequal});
  operator_map.insert({"*", Token::OperatorType::star});
  operator_map.insert({"*=", Token::OperatorType::starequal});
  operator_map.insert({"+", Token::OperatorType::plus});
  operator_map.insert({"++", Token::OperatorType::plusplus});
  operator_map.insert({"+=", Token::OperatorType::plusequal});
  operator_map.insert({"-", Token::OperatorType::minus});
  operator_map.insert({"--", Token::OperatorType::minusminus});
  operator_map.insert({"-=", Token::OperatorType::minusequal});
  operator_map.insert({"~", Token::OperatorType::tilde});
  operator_map.insert({"!", Token::OperatorType::exclaim});
  operator_map.insert({"!=", Token::OperatorType::exclaimequal});
  operator_map.insert({"/", Token::OperatorType::slash});
  operator_map.insert({"/=", Token::OperatorType::slashequal});
  operator_map.insert({"%", Token::OperatorType::percent});
  operator_map.insert({"%=", Token::OperatorType::percentequal});
  operator_map.insert({"<", Token::OperatorType::less});
  operator_map.insert({"<<", Token::OperatorType::lessless});
  operator_map.insert({"<=", Token::OperatorType::lessequal});
  operator_map.insert({"<<=", Token::OperatorType::lesslessequal});
  operator_map.insert({"<=>", Token::OperatorType::spaceship});
  operator_map.insert({">", Token::OperatorType::greater});
  operator_map.insert({">>", Token::OperatorType::greatergreater});
  operator_map.insert({">=", Token::OperatorType::greaterequal});
  operator_map.insert({">>=", Token::OperatorType::greatergreaterequal});
  operator_map.insert({"^", Token::OperatorType::caret});
  operator_map.insert({"^=", Token::OperatorType::caretequal});
  operator_map.insert({"|", Token::OperatorType::pipe});
  operator_map.insert({"||", Token::OperatorType::pipepipe});
  operator_map.insert({"|=", Token::OperatorType::pipeequal});
  operator_map.insert({"?", Token::OperatorType::question});
  operator_map.insert({":", Token::OperatorType::colon});
  operator_map.insert({";", Token::OperatorType::semi});
  operator_map.insert({"=", Token::OperatorType::equal});
  operator_map.insert({"==", Token::OperatorType::equalequal});
  operator_map.insert({",", Token::OperatorType::comma});
  operator_map.insert({"<<<", Token::OperatorType::lesslessless});
  operator_map.insert({">>>", Token::OperatorType::greatergreatergreater});
  operator_map.insert({"^^", Token::OperatorType::caretcaret});
}

Token::KeywordType TokenMap::GetKeywordValue(std::string keyword) {
  auto it = keyword_map.find(keyword);
  if (it != keyword_map.end()) {
    return it->second;
  } else {
    Logging::ERROR(__FUNCTION__, "Unknown keyword: " + keyword);
    return Token::KeywordType::None;
  }
}
Token::OperatorType TokenMap::GetOperatorValue(std::string oper) {
  auto it = operator_map.find(keyword);
  if (it != operator_map.end()) {
    return it->second;
  } else {
    Logging::ERROR(__FUNCTION__, "Unknown keyword: " + oper);
    return Token::OperatorType::None;
  }
}

}  // namespace Compiler
}  // namespace Aq