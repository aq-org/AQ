// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/parser/parser.h"

#include <cstddef>
#include <string>
#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/ast/type.h"
#include "compiler/logging/logging.h"
#include "compiler/parser/declaration_parser.h"
#include "compiler/parser/expression_parser.h"

namespace Aq {
namespace Compiler {
Ast::Compound* Parser::Parse(std::vector<Token>& token) {
  Token* token_ptr = token.data();
  std::size_t index = 0;
  std::size_t length = token.size();
  Ast::Compound* ast = nullptr;
  std::vector<Ast::Statement*> stmts;

  // Delete the last NONE token.
  if (token_ptr[token.size() - 1].type == Token::Type::NONE) token.pop_back();

  while (index < token.size()) {
    if (DeclarationParser::IsDeclaration(token_ptr, length, index)) {
      stmts.push_back(ParseDeclaration(token_ptr, length, index));
    } else {
      stmts.push_back(ParseStatement(token_ptr, length, index));
    }
  }

  ast = new Ast::Compound(stmts);
  return ast;
}

Ast::Expression* Parser::ParsePrimaryExpression(Token* token,
                                                std::size_t length,
                                                std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");
  return ExpressionParser::ParsePrimaryExpression(token, length, index);
}

Ast::Statement* Parser::ParseStatement(Token* token, std::size_t length,
                                       std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");

  if (DeclarationParser::IsDeclaration(token, length, index)) {
    if (DeclarationParser::IsFunctionDeclaration(token, length, index)) {
      LOGGING_ERROR(
          "Function declaration is not supported in general statement.");
      return nullptr;
    } else if (DeclarationParser::IsClassDeclaration(token, length, index)) {
      LOGGING_ERROR("Class declaration is not supported in general statement.");
      return nullptr;
    } else {
      Ast::Variable* result =
          DeclarationParser::ParseVariableDeclaration(token, length, index);
      if (!(token[index] == Token::OperatorType::semi)) {
        LOGGING_WARNING(
            "Expected ';' after variable declaration, but not found.");
        return dynamic_cast<Ast::Declaration*>(result);
      }

      index++;
      return dynamic_cast<Ast::Declaration*>(result);
    }
  }

  switch (token[index].type) {
    case Token::Type::OPERATOR:
      return ParseStatementWithOperator(token, length, index);

    case Token::Type::KEYWORD:
      switch (token[index].value.keyword) {
        case Token::KeywordType::If: {
          index++;

          if (!(token[index] == Token::OperatorType::l_paren)) {
            LOGGING_WARNING("Expected '(' after 'if', but not found.");
          } else {
            index++;
          }

          Ast::Expression* condition =
              ExpressionParser::ParseExpression(token, length, index);

          if (!(token[index] == Token::OperatorType::r_paren)) {
            LOGGING_WARNING(
                "Expected ')' after 'if' condition, but not found.");
          } else {
            index++;
          }

          Ast::Statement* body = ParseStatement(token, length, index);

          if (token[index] == Token::KeywordType::Else) {
            return new Ast::If(condition, body,
                               ParseStatement(token, length, ++index));
          }
          return new Ast::If(condition, body);
        }

        case Token::KeywordType::While: {
          index++;

          if (!(token[index] == Token::OperatorType::l_paren)) {
            LOGGING_WARNING("Expected '(' after 'while', but not found.");
          } else {
            index++;
          }

          Ast::Expression* condition =
              ExpressionParser::ParseExpression(token, length, index);

          if (!(token[index] == Token::OperatorType::r_paren)) {
            LOGGING_WARNING(
                "Expected ')' after 'while' condition, but not found.");
          } else {
            index++;
          }

          return new Ast::While(condition,
                                ParseStatement(token, length, ++index));
        }

        case Token::KeywordType::Do: {
          index++;

          Ast::Statement* stmt_ = ParseStatement(token, length, index);
          if (!(token[index] == Token::KeywordType::While))
            LOGGING_ERROR("Expected 'while' after 'do', but not found.");

          index++;
          if (!(token[index] == Token::OperatorType::l_paren)) {
            LOGGING_WARNING(
                "Expected '(' after 'while' condition, but not found.");
          } else {
            index++;
          }
          Ast::Expression* condition =
              ExpressionParser::ParseExpression(token, length, index);
          if (!(token[index] == Token::OperatorType::r_paren)) {
            LOGGING_WARNING(
                "Expected ')' after 'while' condition, but not found.");
          } else {
            index++;
          }

          return new Ast::DoWhile(condition, stmt_);
        }

        case Token::KeywordType::For: {
          index++;

          if (!(token[index] == Token::OperatorType::l_paren)) {
            LOGGING_WARNING("Expected '(' after 'for', but not found.");
          } else {
            index++;
          }

          Ast::Expression* start = nullptr;

          if (!(token[index + 1] == Token::OperatorType::semi))
            start = ExpressionParser::ParseExpression(token, length, index);

          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING("Expected ';' after 'for' start, but not found.");
          } else {
            index++;
          }

          Ast::Expression* condition = nullptr;
          if (!(token[index + 1] == Token::OperatorType::semi))
            condition = ExpressionParser::ParseExpression(token, length, index);

          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING(
                "Expected ';' after 'for' condition, but not found.");
          } else {
            index++;
          }

          Ast::Expression* end = nullptr;
          if (!(token[index + 1] == Token::OperatorType::semi))
            end = ExpressionParser::ParseExpression(token, length, index);

          if (!(token[index] == Token::OperatorType::r_paren)) {
            LOGGING_WARNING("Expected ')' after 'for' end, but not found.");
          } else {
            index++;
          }

          return new Ast::For(start, condition, end,
                              ParseStatement(token, length, index));
        }

        case Token::KeywordType::Goto: {
          index++;

          if (token[index].type != Token::Type::IDENTIFIER)
            LOGGING_ERROR("Expected identifier after 'goto', but not found.");
          Ast::Identifier label(token[index]);
          index++;
          Ast::Goto* result = new Ast::Goto(label);
          return result;
        }

        case Token::KeywordType::Break:
          index++;
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_ERROR("Expected ';' after 'break', but not found.");
          } else {
            index++;
          }
          return new Ast::Break();

        case Token::KeywordType::Return: {
          index++;

          if (token[index] == Token::OperatorType::semi) {
            Ast::Return* result = new Ast::Return(nullptr);
            index++;
            return result;
          }

          Ast::Expression* return_expr =
              ExpressionParser::ParseExpression(token, length, index);
          if (return_expr == nullptr) LOGGING_ERROR("return_expr is nullptr.");
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_ERROR("Expected ';' after 'return', but not found.");
          } else {
            index++;
          }
          return new Ast::Return(return_expr);
        }

          /*case Token::KeywordType::From: {
            index++;
            if (token[index].type != Token::Type::STRING)
              LOGGING_ERROR("Unsupported import location.");
            std::string import_location(*token[index].value.string);
            index++;
            if (token[index].type != Token::Type::KEYWORD ||
                token[index].value.keyword != Token::KeywordType::Import)
              LOGGING_ERROR("Unexpected import behavior.");
            index++;
            std::vector<std::string> import_list;
            if (token[index].type != Token::Type::IDENTIFIER)
              LOGGING_ERROR("Unexpected import identifier.");
            import_list.push_back(
                std::string(token[index].value.identifier.location,
                            token[index].value.identifier.length));
            index++;
            while (token[index].type == Token::Type::OPERATOR &&
                   token[index].value.oper == Token::OperatorType::comma) {
              index++;
              if (token[index].type != Token::Type::IDENTIFIER)
                LOGGING_ERROR("Unexpected import identifier.");
              import_list.push_back(
                  std::string(token[index].value.identifier.location,
                              token[index].value.identifier.length));
              index++;
            }

            std::vector<std::string> alias_list;

            if (token[index].type == Token::Type::KEYWORD &&
                token[index].value.keyword == Token::KeywordType::As) {
              index++;
              if (token[index].type != Token::Type::IDENTIFIER)
                LOGGING_ERROR("Unexpected import alias.");
              alias_list.push_back(
                  std::string(token[index].value.identifier.location,
                              token[index].value.identifier.length));
              index++;
              while (token[index].type == Token::Type::OPERATOR &&
                     token[index].value.oper == Token::OperatorType::comma)
          { index++; if (token[index].type != Token::Type::IDENTIFIER)
                  LOGGING_ERROR("Unexpected import alias.");
                alias_list.push_back(
                    std::string(token[index].value.identifier.location,
                                token[index].value.identifier.length));
                index++;
              }
            }

            Ast::Import* result =
                new Ast::Import(import_location, import_list, alias_list);

            return result;
          }*/

        case Token::KeywordType::Import: {
          index++;
          if (token[index].type != Token::Type::STRING)
            LOGGING_ERROR("Unsupported import location.");
          std::string import_location(*token[index].value.string);
          index++;
          if (token[index].type != Token::Type::IDENTIFIER)
            LOGGING_ERROR("Unexpected import behavior.");
          std::string name(token[index].value.identifier.location,
                           token[index].value.identifier.length);
          Ast::Import* result = new Ast::Import(import_location, name);
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_ERROR("Expected ';' after 'import', but not found.");
          } else {
            index++;
          }
          return result;
        }

        default:
          return nullptr;
      }

    default:
      if (token[index].type == Token::Type::IDENTIFIER &&
          token[index + 1].type == Token::Type::OPERATOR &&
          token[index + 1].value.oper == Token::OperatorType::colon) {
        Ast::Identifier identifier_node(token[index]);
        Ast::Label* result = new Ast::Label(identifier_node);
        if (!(token[index] == Token::OperatorType::semi)) {
          LOGGING_ERROR("Expected ';' after 'return', but not found.");
          index++;
        } else {
          index += 2;
        }
        return result;
      }
      Ast::Statement* stmt_node =
          ExpressionParser::ParseExpression(token, length, index);
      if (token[index] == Token::OperatorType::semi) index++;
      return stmt_node;
  }
}

Ast::Declaration* Parser::ParseDeclaration(Token* token, std::size_t length,
                                           std::size_t& index) {
  if (DeclarationParser::IsDeclaration(token, length, index)) {
    if (DeclarationParser::IsFunctionDeclaration(token, length, index)) {
      return DeclarationParser::ParseFunctionDeclaration(token, length, index);
    } else if (DeclarationParser::IsClassDeclaration(token, length, index)) {
      return DeclarationParser::ParseClassDeclaration(token, length, index);
    } else {
      Ast::Declaration* variable = dynamic_cast<Ast::Declaration*>(
          DeclarationParser::ParseVariableDeclaration(token, length, index));
      if (!(token[index] == Token::OperatorType::semi)) {
        LOGGING_WARNING(
            "Expected ';' after variable declaration, but not found.");
        return variable;
      }
      index++;
      return variable;
    }
  }

  INTERNAL_ERROR("Unexpected declaration.");
  return nullptr;
}

Ast::Statement* Parser::ParseStatementWithOperator(Token* token,
                                                   std::size_t length,
                                                   std::size_t& index) {
  switch (token[index].value.oper) {
    // Null statement.
    case Token::OperatorType::semi:
      index++;
      return new Ast::Statement();

    case Token::OperatorType::l_brace: {
      index++;
      std::vector<Ast::Statement*> statements;

      while (!(token[index] == Token::OperatorType::r_brace) &&
             index < length) {
        statements.push_back(ParseStatement(token, length, index));
      }

      if (!(token[index] == Token::OperatorType::r_brace)) {
        LOGGING_WARNING(
            "Expected '}' after compound statement, but not found.");
        return new Ast::Compound(statements);
      }

      index++;
      return new Ast::Compound(statements);
    }

    case Token::OperatorType::r_square:
    case Token::OperatorType::r_paren:
    case Token::OperatorType::r_brace:
      if (!(token[index] == Token::OperatorType::r_brace)) {
        LOGGING_WARNING("Unexpected '}'.");
        index++;
      }

    default:
      Ast::Statement* statement =
          ExpressionParser::ParseExpression(token, length, index);
      if (token[index] == Token::OperatorType::semi) index++;
      return statement;
  }
}

}  // namespace Compiler
}  // namespace Aq