// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "parser/parser.h"

#include <cstddef>
#include <string>
#include <vector>

#include "ast/ast.h"
#include "ast/type.h"
#include "logging/logging.h"
#include "parser/declaration_parser.h"
#include "parser/expression_parser.h"

namespace Aq {
// Parse is the main entry point for parsing a vector of tokens into an AST.
// It processes the token stream sequentially, identifying whether each
// statement is a declaration or a regular statement, and builds a complete
// Abstract Syntax Tree (AST) representing the program structure.
// Returns a Compound node containing all top-level statements.
Ast::Compound* Parser::Parse(std::vector<Token>& token) {
  // Initialize parsing state with token stream information
  Token* token_ptr = token.data();
  std::size_t index = 0;
  std::size_t length = token.size();

  // Vector to accumulate all parsed statements
  std::vector<Ast::Statement*> stmts;

  // Remove the trailing NONE token if present.
  // The lexer adds a NONE token as a sentinel value, which we don't need.
  if (token_ptr[token.size() - 1].type == Token::Type::NONE) {
    token.pop_back();
    length--;
  }

  // Parse all tokens into statements
  while (index < token.size()) {
    // Check if this is a declaration (function, variable, class, etc.)
    if (DeclarationParser::IsDeclaration(token_ptr, length, index)) {
      stmts.push_back(ParseDeclaration(token_ptr, length, index));
    } else {
      // Otherwise, it's a regular statement (expression, control flow, etc.)
      stmts.push_back(ParseStatement(token_ptr, length, index));
    }
  }

  // Wrap all statements in a Compound node which represents the entire program
  Ast::Compound* ast = new Ast::Compound(stmts);
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

  if (DeclarationParser::IsDeclaration(token, length, index))
    return ParseDeclaration(token, length, index);

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

          if (token[index] == Token::KeywordType::Else)
            return new Ast::If(condition, body,
                               ParseStatement(token, length, ++index));

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
                                ParseStatement(token, length, index));
        }

        case Token::KeywordType::Do: {
          index++;

          Ast::Statement* body = ParseStatement(token, length, index);
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

          return new Ast::DoWhile(condition, body);
        }

        case Token::KeywordType::For: {
          index++;

          if (!(token[index] == Token::OperatorType::l_paren)) {
            LOGGING_WARNING("Expected '(' after 'for', but not found.");
          } else {
            index++;
          }

          // TODO(bugs): May have bugs in the for statement parsing.
          Ast::Expression* start = new Ast::Expression();

          if (!(token[index] == Token::OperatorType::semi)) {
            delete start;
            start = ExpressionParser::ParseExpression(token, length, index);
          }

          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING("Expected ';' after 'for' start, but not found.");
          } else {
            index++;
          }

          Ast::Expression* condition = new Ast::Expression();
          if (!(token[index] == Token::OperatorType::semi)) {
            delete condition;
            condition = ExpressionParser::ParseExpression(token, length, index);
            if (condition == nullptr) LOGGING_ERROR("FATAL ERROR");
          }

          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING(
                "Expected ';' after 'for' condition, but not found.");
          } else {
            index++;
          }

          Ast::Expression* end = new Ast::Expression();
          if (!(token[index] == Token::OperatorType::semi)) {
            delete end;
            end = ExpressionParser::ParseExpression(token, length, index);
          }

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
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING("Expected ';' after 'goto', but not found.");
          } else {
            index++;
          }
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

          // No return value.
          if (token[index] == Token::OperatorType::semi) {
            Ast::Return* result = new Ast::Return(nullptr);
            index++;
            return result;
          }

          Ast::Expression* return_expression =
              ExpressionParser::ParseExpression(token, length, index);
          if (return_expression == nullptr)
            LOGGING_ERROR("return_expression is nullptr.");

          // Checks if the return expression is followed by a semicolon.
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_ERROR("Expected ';' after 'return', but not found.");
          } else {
            index++;
          }

          return new Ast::Return(return_expression);
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

          // Location of the import.
          if (token[index].type != Token::Type::STRING)
            LOGGING_ERROR("Unsupported import location.");
          std::string import_location(*token[index].value.string);
          index++;

          // module name.
          if (token[index].type != Token::Type::IDENTIFIER)
            LOGGING_ERROR("Unexpected import behavior.");
          std::string name(token[index].value.identifier.location,
                           token[index].value.identifier.length);
          index++;

          Ast::Import* result = new Ast::Import(import_location, name);

          // Checks if the import is followed by a semicolon.
          if (!(token[index] == Token::OperatorType::semi)) {
            LOGGING_WARNING("Expected ';' after 'import', but not found.");
          } else {
            index++;
          }

          return result;
        }

        default:
          return nullptr;
      }

    default:
      // Label
      if (token[index].type == Token::Type::IDENTIFIER &&
          token[index + 1] == Token::OperatorType::colon) {
        Ast::Label* result = new Ast::Label(Ast::Identifier(token[index]));
        index++;

        // Checks if the label is followed by a colon.
        if (!(token[index] == Token::OperatorType::semi)) {
          LOGGING_WARNING("Expected ';' after 'return', but not found.");
        } else {
          index++;
        }

        return result;
      }

      // Expression
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

      // Checks if the declaration has a semicolon.
      if (!(token[index] == Token::OperatorType::semi)) {
        LOGGING_WARNING(
            "Expected ';' after variable declaration, but not found.");
        return variable;
      }

      // Skips the semicolon.
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
  // LOGGING_INFO(std::to_string(index));
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
    case Token::OperatorType::r_brace: {
      if (token[index] == Token::OperatorType::r_brace) {
        LOGGING_WARNING("Unexpected '}'.");
        index++;
      }
      std::vector<Ast::Statement*> void_statement{new Ast::Statement()};
      return new Ast::Compound(void_statement);
    }

    default:
      Ast::Statement* statement =
          ExpressionParser::ParseExpression(token, length, index);
      if (token[index] == Token::OperatorType::semi) index++;
      return statement;
  }
}

}  // namespace Aq