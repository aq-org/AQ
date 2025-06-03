// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/parser/declaration_parser.h"

#include <cstddef>
#include <string>

#include "compiler/ast/ast.h"
#include "compiler/ast/type.h"
#include "compiler/logging/logging.h"
#include "compiler/parser/expression_parser.h"

namespace Aq {
namespace Compiler {

bool Parser::DeclarationParser::IsDeclaration(Token* token, std::size_t length,
                                              std::size_t index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  return HasTypeBeforeExpression(token, length, index) ||
         HasCustomTypeBeforeExpression(token, length, index);
}

bool Parser::DeclarationParser::IsFunctionDeclaration(Token* token,
                                                      std::size_t length,
                                                      std::size_t index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  for (std::size_t i = index; i < length; i++) {
    if (token[i].type == Token::Type::IDENTIFIER) {
      if (token[i + 1] == Token::OperatorType::l_paren) return true;

      // Skip the identifier if it is followed by a period (an identifier with
      // scopes). i++ in this if skips the identifier and another i++ from the
      // for loop skips the period.
      if (token[i + 1] == Token::OperatorType::period) {
        i++;
      } else {
        return false;
      }
    }
  }
  return false;
}

bool Parser::DeclarationParser::IsClassDeclaration(Token* token,
                                                   std::size_t length,
                                                   std::size_t index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  if (token[index] == Token::KeywordType::Class ||
      token[index] == Token::KeywordType::Struct)
    return true;

  return false;
}

Ast::FunctionDeclaration* Parser::DeclarationParser::ParseFunctionDeclaration(
    Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  Ast::Type* return_type = Ast::Type::CreateType(token, length, index);
  Ast::Function* statement = dynamic_cast<Ast::Function*>(
      Parser::ExpressionParser::ParsePrimaryExpression(token, length, index));

  if (statement == nullptr) LOGGING_ERROR("Statement isn't a function.");

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (token[index] == Token::OperatorType::semi) {
    index++;
    LOGGING_WARNING("The function declaration has been deprecated.");
    return new Ast::FunctionDeclaration(return_type, statement, nullptr);
  }

  if (!(token[index] == Token::OperatorType::l_brace))
    LOGGING_ERROR("Expected '{', but not found. (Function body start).");

  Ast::Compound* body =
      dynamic_cast<Ast::Compound*>(ParseStatement(token, length, index));
  if (body == nullptr) LOGGING_ERROR("The function body has error.");

  return new Ast::FunctionDeclaration(return_type, statement, body);
}

Ast::Class* Parser::DeclarationParser::ParseClassDeclaration(
    Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");
  if ((!(token[index] == Token::KeywordType::Class) &&
       !(token[index] == Token::KeywordType::Struct)))
    LOGGING_ERROR("Class or Struct not found.");

  index++;

  Ast::Identifier* name = dynamic_cast<Ast::Identifier*>(
      Parser::ExpressionParser::ParsePrimaryExpression(token, length, index));
  if (name == nullptr) LOGGING_ERROR("name is not an identifier.");

  if (!(token[index] == Token::OperatorType::l_brace))
    LOGGING_ERROR("Expected '{', but not found. (Class body start).");

  index++;

  std::vector<Ast::Static*> static_members;
  std::vector<Ast::Variable*> members;
  std::vector<Ast::FunctionDeclaration*> methods;
  std::vector<Ast::Class*> sub_classes;

  while (index < length && !(token[index] == Token::OperatorType::r_brace)) {
    if (IsDeclaration(token, length, index)) {
      if (IsFunctionDeclaration(token, length, index)) {
        methods.push_back(ParseFunctionDeclaration(token, length, index));
      } else if (IsClassDeclaration(token, length, index)) {
        sub_classes.push_back(ParseClassDeclaration(token, length, index));
      } else {
        members.push_back(ParseVariableDeclaration(token, length, index));
        if (token[index].value.oper != Token::OperatorType::semi) {
          LOGGING_WARNING("Expected ';', but not found.");
        } else {
          index++;
        }
      }
    } else if (token[index] == Token::KeywordType::Static) {
      static_members.push_back(ParseStatic(token, length, index));
    } else {
      LOGGING_ERROR(
          "Statements in the class that exceed the function are not "
          "supported.");
    }
  }
  index++;

  return new Ast::Class(*name, static_members, members, methods, sub_classes);
  ;
}

Ast::Variable* Parser::DeclarationParser::ParseVariableDeclaration(
    Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  Ast::Type* type = Ast::Type::CreateType(token, length, index);
  if (type == nullptr) LOGGING_ERROR("type is nullptr.");
  Ast::Expression* name =
      ExpressionParser::ParsePrimaryExpression(token, length, index);
  if (name == nullptr) LOGGING_ERROR("name is nullptr.");

  if (name->GetStatementType() == Ast::Statement::StatementType::kArray) {
    return ParseArrayDeclaration(type, name, token, length, index);
  } else {
    if (token[index].value.oper == Token::OperatorType::equal) {
      return new Ast::Variable(type, name,
                               ExpressionParser::ParseExpressionWithoutComma(
                                   token, length, ++index));
    }
    return new Ast::Variable(type, name);
  }

  return nullptr;
}

Ast::Static* Parser::DeclarationParser::ParseStatic(Token* token,
                                                    std::size_t length,
                                                    std::size_t& index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  if (token[index].type == Token::Type::KEYWORD &&
      token[index] == Token::KeywordType::Static) {
    index++;
  } else {
    LOGGING_ERROR("Unexpected keyword.");
  }

  if (IsDeclaration(token, length, index)) {
    if (IsFunctionDeclaration(token, length, index)) {
      return new Ast::Static(ParseFunctionDeclaration(token, length, index));
    } else if (IsClassDeclaration(token, length, index)) {
      LOGGING_ERROR("Keyword static unsupprot class.");
    } else {
      Ast::Static* static_node =
          new Ast::Static(dynamic_cast<Ast::Declaration*>(
              ParseVariableDeclaration(token, length, index)));
      if (!(token[index] == Token::OperatorType::semi)) {
        LOGGING_WARNING(
            "Expected ';' after static declaration, but not found.");
        return static_node;
      }
      index++;
      return static_node;
    }
  }

  LOGGING_ERROR("Unexpected code.");
  return nullptr;
}

bool Parser::DeclarationParser::HasTypeBeforeExpression(Token* token,
                                                        std::size_t length,
                                                        std::size_t index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  if (token[index] == Token::KeywordType::Auto ||
      token[index] == Token::KeywordType::Bool ||
      token[index] == Token::KeywordType::Char ||
      token[index] == Token::KeywordType::Double ||
      token[index] == Token::KeywordType::Float ||
      token[index] == Token::KeywordType::Int ||
      token[index] == Token::KeywordType::Long ||
      token[index] == Token::KeywordType::Void ||
      token[index] == Token::KeywordType::String ||
      token[index] == Token::KeywordType::Struct ||
      token[index] == Token::KeywordType::Union ||
      token[index] == Token::KeywordType::Enum ||
      token[index] == Token::KeywordType::Namespace ||
      token[index] == Token::KeywordType::Template ||
      token[index] == Token::KeywordType::Typedef ||
      token[index] == Token::KeywordType::Extern ||
      token[index] == Token::KeywordType::Class ||
      token[index] == Token::KeywordType::Const ||
      token[index] == Token::KeywordType::Friend ||
      token[index] == Token::KeywordType::Inline ||
      token[index] == Token::KeywordType::Number ||
      token[index] == Token::KeywordType::Short ||
      token[index] == Token::KeywordType::Signed ||
      token[index] == Token::KeywordType::Unsigned ||
      token[index] == Token::KeywordType::Var ||
      token[index] == Token::KeywordType::Func ||
      token[index] == Token::KeywordType::Virtual ||
      token[index] == Token::KeywordType::Wchar_t) {
    return true;
  }
  return false;
}

bool Parser::DeclarationParser::HasCustomTypeBeforeExpression(
    Token* token, std::size_t length, std::size_t index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");

  // Skips the scopes if has.
  while (index < length) {
    if (token[index] == Token::Type::IDENTIFIER &&
        token[index + 1] == Token::OperatorType::period &&
        token[index + 2] == Token::Type::IDENTIFIER) {
      index += 2;
    } else {
      break;
    }
  }

  if (token[index] == Token::Type::IDENTIFIER &&
      (token[index + 1] == Token::Type::IDENTIFIER ||
       (token[index + 1] == Token::OperatorType::amp &&
        token[index + 2] == Token::Type::IDENTIFIER))) {
    return true;
  }

  return false;
}

Ast::ArrayDeclaration* Parser::DeclarationParser::ParseArrayDeclaration(
    Ast::Type* type, Ast::Expression* name, Token* token, std::size_t length,
    std::size_t& index) {
  if (token == nullptr) LOGGING_ERROR("token is nullptr.");
  if (index >= length) LOGGING_ERROR("index is out of range.");
  Ast::Array* array = dynamic_cast<Ast::Array*>(name);
  if (array == nullptr) LOGGING_ERROR("name is not an array.");

  if (token[index].value.oper == Token::OperatorType::equal) {
    index++;
    if (token[index].type == Token::Type::OPERATOR &&
        token[index].value.oper == Token::OperatorType::l_brace) {
      std::vector<Ast::Expression*> values;
      while (true) {
        // Skip the l_brace or comma.
        values.push_back(ExpressionParser::ParseExpressionWithoutComma(
            token, length, ++index));
        if (token[index] == Token::OperatorType::r_brace) {
          index++;
          break;
        }
        if (!(token[index] == Token::OperatorType::comma)) {
          LOGGING_ERROR(
              "Expected '}' or ',' after '=' in array declaration, but not "
              "found.");
        }
      }
      return new Ast::ArrayDeclaration(type, array->GetExpression(),
                                       array->GetIndexExpression(), values);
    } else {
      LOGGING_ERROR(
          "Expected '{' or ',' after '=' in array declaration, but not found.");
    }
  }
  return new Ast::ArrayDeclaration(type, array->GetExpression(),
                                   array->GetIndexExpression());
}

}  // namespace Compiler
}  // namespace Aq