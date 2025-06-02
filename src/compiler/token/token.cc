// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/token/token.h"

#include <iostream>
#include <string>

#include "compiler/logging/logging.h"

namespace Aq {
namespace Compiler {

std::string Token::GetTokenTypeString(Token::Type type) {
  switch (type) {
    case Token::Type::NONE:
      return "NONE";
    case Token::Type::START:
      return "START";
    case Token::Type::KEYWORD:
      return "KEYWORD";
    case Token::Type::IDENTIFIER:
      return "IDENTIFIER";
    case Token::Type::OPERATOR:
      return "OPERATOR";
    case Token::Type::NUMBER:
      return "NUMBER";
    case Token::Type::CHARACTER:
      return "CHARACTER";
    case Token::Type::STRING:
      return "STRING";
    case Token::Type::COMMENT:
      return "COMMENT";
    default:
      LOGGING_WARNING("Unknown token type: " +
                      std::to_string(static_cast<int>(type)));
      return "UNKNOWN";
  }
}

std::string Token::GetKeywordTypeString(Token::KeywordType keyword) {
  switch (keyword) {
    case Token::KeywordType::Auto:
      return "Auto";
    case Token::KeywordType::And:
      return "And";
    case Token::KeywordType::Bitand:
      return "Bitand";
    case Token::KeywordType::Bitor:
      return "Bitor";
    case Token::KeywordType::Bool:
      return "Bool";
    case Token::KeywordType::Break:
      return "Break";
    case Token::KeywordType::Case:
      return "Case";
    case Token::KeywordType::Catch:
      return "Catch";
    case Token::KeywordType::Char:
      return "Char";
    case Token::KeywordType::Class:
      return "Class";
    case Token::KeywordType::Const:
      return "Const";
    case Token::KeywordType::Continue:
      return "Continue";
    case Token::KeywordType::Default:
      return "Default";
    case Token::KeywordType::Do:
      return "Do";
    case Token::KeywordType::Double:
      return "Double";
    case Token::KeywordType::Else:
      return "Else";
    case Token::KeywordType::Enum:
      return "Enum";
    case Token::KeywordType::Export:
      return "Export";
    case Token::KeywordType::Extern:
      return "Extern";
    case Token::KeywordType::False:
      return "False";
    case Token::KeywordType::Func:
      return "Func";
    case Token::KeywordType::Float:
      return "Float";
    case Token::KeywordType::For:
      return "For";
    case Token::KeywordType::Friend:
      return "Friend";
    case Token::KeywordType::Goto:
      return "Goto";
    case Token::KeywordType::If:
      return "If";
    case Token::KeywordType::Import:
      return "Import";
    case Token::KeywordType::Inline:
      return "Inline";
    case Token::KeywordType::Int:
      return "Int";
    case Token::KeywordType::Long:
      return "Long";
    case Token::KeywordType::Namespace:
      return "Namespace";
    case Token::KeywordType::New:
      return "New";
    case Token::KeywordType::Not:
      return "Not";
    case Token::KeywordType::Number:
      return "Number";
    case Token::KeywordType::Operator:
      return "Operator";
    case Token::KeywordType::Or:
      return "Or";
    case Token::KeywordType::Private:
      return "Private";
    case Token::KeywordType::Protected:
      return "Protected";
    case Token::KeywordType::Public:
      return "Public";
    case Token::KeywordType::Return:
      return "Return";
    case Token::KeywordType::Short:
      return "Short";
    case Token::KeywordType::Signed:
      return "Signed";
    case Token::KeywordType::Sizeof:
      return "Sizeof";
    case Token::KeywordType::Static:
      return "Static";
    case Token::KeywordType::String:
      return "String";
    case Token::KeywordType::Struct:
      return "Struct";
    case Token::KeywordType::Switch:
      return "Switch";
    case Token::KeywordType::Template:
      return "Template";
    case Token::KeywordType::This:
      return "This";
    case Token::KeywordType::Thread:
      return "Thread";
    case Token::KeywordType::True:
      return "True";
    case Token::KeywordType::Try:
      return "Try";
    case Token::KeywordType::Typedef:
      return "Typedef";
    case Token::KeywordType::Typeid:
      return "Typeid";
    case Token::KeywordType::Typename:
      return "Typename";
    case Token::KeywordType::Union:
      return "Union";
    case Token::KeywordType::Unsigned:
      return "Unsigned";
    case Token::KeywordType::Using:
      return "Using";
    case Token::KeywordType::Var:
      return "Var";
    case Token::KeywordType::Virtual:
      return "Virtual";
    case Token::KeywordType::Void:
      return "Void";
    case Token::KeywordType::Wchar_t:
      return "Wchar_t";
    case Token::KeywordType::While:
      return "While";
    case Token::KeywordType::Xor:
      return "Xor";
    default:
      LOGGING_WARNING("Unknown keyword type: " +
                      std::to_string(static_cast<int>(keyword)));
      return "UNKNOWN_KEYWORD";
  }
}

std::string Token::GetOperatorTypeString(Token::OperatorType op) {
  switch (op) {
    case Token::OperatorType::NONE:
      return "NONE";
    case Token::OperatorType::l_square:
      return "[";
    case Token::OperatorType::r_square:
      return "]";
    case Token::OperatorType::l_paren:
      return "(";
    case Token::OperatorType::r_paren:
      return ")";
    case Token::OperatorType::l_brace:
      return "{";
    case Token::OperatorType::r_brace:
      return "}";
    case Token::OperatorType::period:
      return ".";
    case Token::OperatorType::ellipsis:
      return "...";
    case Token::OperatorType::amp:
      return "&";
    case Token::OperatorType::ampamp:
      return "&&";
    case Token::OperatorType::ampequal:
      return "&=";
    case Token::OperatorType::star:
      return "*";
    case Token::OperatorType::starequal:
      return "*=";
    case Token::OperatorType::plus:
      return "+";
    case Token::OperatorType::plusplus:
      return "++";
    case Token::OperatorType::plusequal:
      return "+=";
    case Token::OperatorType::minus:
      return "-";
    case Token::OperatorType::minusminus:
      return "--";
    case Token::OperatorType::minusequal:
      return "-=";
    case Token::OperatorType::tilde:
      return "~";
    case Token::OperatorType::exclaim:
      return "!";
    case Token::OperatorType::exclaimequal:
      return "!=";
    case Token::OperatorType::slash:
      return "/";
    case Token::OperatorType::slashequal:
      return "/=";
    case Token::OperatorType::percent:
      return "%";
    case Token::OperatorType::percentequal:
      return "%=";
    case Token::OperatorType::less:
      return "<";
    case Token::OperatorType::lessless:
      return "<<";
    case Token::OperatorType::lessequal:
      return "<=";
    case Token::OperatorType::lesslessequal:
      return "<<=";
    case Token::OperatorType::spaceship:
      return "<=>";
    case Token::OperatorType::greater:
      return ">";
    case Token::OperatorType::greatergreater:
      return ">>";
    case Token::OperatorType::greaterequal:
      return ">=";
    case Token::OperatorType::greatergreaterequal:
      return ">>=";
    case Token::OperatorType::caret:
      return "^";
    case Token::OperatorType::caretequal:
      return "^=";
    case Token::OperatorType::pipe:
      return "|";
    case Token::OperatorType::pipepipe:
      return "||";
    case Token::OperatorType::pipeequal:
      return "|=";
    case Token::OperatorType::question:
      return "?";
    case Token::OperatorType::colon:
      return ":";
    case Token::OperatorType::semi:
      return ";";
    case Token::OperatorType::equal:
      return "=";
    case Token::OperatorType::equalequal:
      return "==";
    case Token::OperatorType::comma:
      return ",";
    case Token::OperatorType::lesslessless:
      return "<<<";
    case Token::OperatorType::greatergreatergreater:
      return ">>>";
    case Token::OperatorType::caretcaret:
      return "^^";
    default:
      LOGGING_WARNING("Unknown operator type: " +
                      std::to_string(static_cast<int>(op)));
      return "UNKNOWN_OPERATOR";
  }
}

std::ostream& Token::operator<<(std::ostream& os, Token& token) {
  os << "Type: " << token.GetTokenTypeString(token.type) << ", Value: ";
  switch (token.type) {
    case Token::Type::KEYWORD:
      os << token.GetKeywordTypeString(token.value.keyword);
      break;
    case Token::Type::IDENTIFIER:
      os << std::string(token.value.identifier.location,
                        token.value.identifier.length);
      break;
    case Token::Type::OPERATOR:
      os << token.GetOperatorTypeString(token.value._operator);
      break;
    case Token::Type::NUMBER:
      os << std::string(token.value.number.location, token.value.number.length);
      break;
    case Token::Type::CHARACTER:
      os << token.value.character;
      break;
    case Token::Type::STRING:
      os << token.value.string;
      break;
    default:
      LOGGING_WARNING("Unknown token type: " +
                      std::to_string(static_cast<int>(token.type)));
      os << "N/A";
      break;
  }
  return os;
}
}  // namespace Compiler
}  // namespace Aq