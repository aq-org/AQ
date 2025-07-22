// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_PARSER_EXPRESSION_PARSER_H_
#define AQ_PARSER_EXPRESSION_PARSER_H_

#include "ast/ast.h"
#include "parser/parser.h"

namespace Aq {
class Parser::ExpressionParser {
 public:
  ExpressionParser() = delete;
  virtual ~ExpressionParser() = delete;

  // Parses the tokens and returns the expression.
  static Ast::Expression* ParseExpression(Token* token, std::size_t length,
                                          std::size_t& index);

  // Parses the tokens and returns the expression without comma.
  static Ast::Expression* ParseExpressionWithoutComma(Token* token,
                                                      std::size_t length,
                                                      std::size_t& index);

  // Parses the tokens and returns the primary expression. Performs syntax
  // analysis on simple expression structures such as unary expressions and
  // function calls.
  static Ast::Expression* ParsePrimaryExpression(Token* token,
                                                 std::size_t length,
                                                 std::size_t& index);

  // Performs syntax analysis on function calls. Returns the function call.
  static Ast::Expression* ParseBinaryExpression(Token* token,
                                                std::size_t length,
                                                std::size_t& index,
                                                Ast::Expression* left,
                                                unsigned int priority);

  // Performs syntax analysis on binary expressions. Returns the binary
  // expression.
  static Ast::Expression* ParseBinaryExpressionWithoutComma(
      Token* token, std::size_t length, std::size_t& index,
      Ast::Expression* left, unsigned int priority);

  // Gets the priority of the expressions. Returns the priority. The larger the
  // number, the higher the priority.
  static unsigned int GetPriority(Token token);

 private:
  // Parses the primary general expression (plus, mimus, not, bitwise not, etc).
  static void ParsePrimaryGeneralPreExpression(
      Ast::Unary::Operator unary_operator, Ast::Expression*& full_expression,
      Ast::Expression*& pre_operator_expression);

  // Updates the experssion if |pre_operator_expression| is not nullptr.
  static void HandlePreOperatorExpression(
      Ast::Expression*& pre_operator_expression,
      Ast::Expression* new_expression);

  // Parses the parenthesized expression.
  static void ParseParenthesizedExpression(
      Ast::Expression* new_expression, Ast::Expression*& full_expression,
      Ast::Expression*& main_expression,
      Ast::Expression*& pre_operator_expression);

  // Parses the array expression.
  static void ParseArrayExpression(Token* token, std::size_t length,
                                   std::size_t& index,
                                   Ast::Expression*& full_expression,
                                   Ast::Expression*& main_expression,
                                   Ast::Expression*& pre_operator_expression);

  // Parses the function call expression.
  static void ParseFunctionCallExpression(
      Token* token, std::size_t length, std::size_t& index,
      Ast::Expression*& full_expression, Ast::Expression*& main_expression,
      Ast::Expression*& pre_operator_expression);

  // Parses the increment and decrement operators expression.
  static void ParseIncrementAndDecrementOperatorsExpression(
      bool is_pre_operator, bool is_increment,
      Ast::Expression*& full_expression,
      Ast::Expression*& pre_operator_expression);
};
}  // namespace Aq

#endif