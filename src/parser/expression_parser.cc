// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "parser/expression_parser.h"

#include <cstddef>
#include <string>

#include "ast/ast.h"
#include "ast/type.h"
#include "logging/logging.h"
#include "parser/declaration_parser.h"

namespace Aq {
Ast::Expression* Parser::ExpressionParser::ParseExpression(Token* token,
                                                           std::size_t length,
                                                           std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");

  if (DeclarationParser::IsDeclaration(token, length, index))
    return DeclarationParser::ParseVariableDeclaration(token, length, index);

  Ast::Expression* expr = ParsePrimaryExpression(token, length, index);
  if (expr == nullptr) INTERNAL_ERROR("expr is nullptr.");

  // Continues to parse expression if the expression is binary expression.
  expr = ParseBinaryExpression(token, length, index, expr, 0);

  return expr;
}

Ast::Expression* Parser::ExpressionParser::ParseExpressionWithoutComma(
    Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");

  if (DeclarationParser::IsDeclaration(token, length, index))
    return DeclarationParser::ParseVariableDeclaration(token, length, index);

  Ast::Expression* expr = ParsePrimaryExpression(token, length, index);
  if (expr == nullptr) INTERNAL_ERROR("expr is nullptr.");

  // Continues to parse expression if the expression is binary expression.
  expr = ParseBinaryExpressionWithoutComma(token, length, index, expr, 0);
  return expr;
}

Ast::Expression* Parser::ExpressionParser::ParseBinaryExpression(
    Token* token, std::size_t length, std::size_t& index, Ast::Expression* left,
    unsigned int priority) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");
  if (left == nullptr) INTERNAL_ERROR("left is nullptr.");

  Ast::Expression* expr = left;
  while (index < length && GetPriority(token[index]) > priority) {
    if (!(token[index] == Token::Type::OPERATOR))
      INTERNAL_ERROR("Unexpected code.");
    switch (token[index].value.oper) {
      case Token::OperatorType::comma: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kComma, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  1));
        break;
      }

      case Token::OperatorType::star:
      case Token::OperatorType::slash:
      case Token::OperatorType::percent:
      case Token::OperatorType::plus:
      case Token::OperatorType::minus:
      case Token::OperatorType::lessless:
      case Token::OperatorType::greatergreater:
      case Token::OperatorType::less:
      case Token::OperatorType::lessequal:
      case Token::OperatorType::greater:
      case Token::OperatorType::greaterequal:
      case Token::OperatorType::equalequal:
      case Token::OperatorType::exclaimequal:
      case Token::OperatorType::amp:
      case Token::OperatorType::caret:
      case Token::OperatorType::pipe:
      case Token::OperatorType::ampamp:
      case Token::OperatorType::pipepipe:
      case Token::OperatorType::equal:
      case Token::OperatorType::plusequal:
      case Token::OperatorType::minusequal:
      case Token::OperatorType::starequal:
      case Token::OperatorType::slashequal:
      case Token::OperatorType::percentequal:
      case Token::OperatorType::ampequal:
      case Token::OperatorType::caretequal:
      case Token::OperatorType::pipeequal:
      case Token::OperatorType::lesslessequal:
      case Token::OperatorType::greatergreaterequal:
        return ParseBinaryExpressionWithoutComma(token, length, index, left,
                                                 priority);

      default:
        if (expr == nullptr) LOGGING_ERROR("ERRORRRRRRRRRRRRRRRRRR");
        return expr;
    }
  }

  if (expr == nullptr) LOGGING_ERROR("ERRORRRRRRRRRRRRRRRRRR");
  return expr;
}

Ast::Expression* Parser::ExpressionParser::ParseBinaryExpressionWithoutComma(
    Token* token, std::size_t length, std::size_t& index, Ast::Expression* left,
    unsigned int priority) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");
  if (left == nullptr) INTERNAL_ERROR("left is nullptr.");

  Ast::Expression* expr = left;
  while (index < length && GetPriority(token[index]) > priority) {
    if (!(token[index] == Token::Type::OPERATOR))
      INTERNAL_ERROR("Unexpected code.");
    switch (token[index].value.oper) {
      case Token::OperatorType::star: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kMul, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  13));
        break;
      }
      case Token::OperatorType::slash: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kDiv, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  13));
        break;
      }
      case Token::OperatorType::percent: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kRem, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  13));
        break;
      }

      case Token::OperatorType::plus: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kAdd, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  12));
        break;
      }
      case Token::OperatorType::minus: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kSub, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  12));
        break;
      }

      case Token::OperatorType::lessless: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kShl, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  11));
        break;
      }
      case Token::OperatorType::greatergreater: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kShr, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  11));
        break;
      }

      case Token::OperatorType::less: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kLT, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  10));
        break;
      }
      case Token::OperatorType::lessequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kLE, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  10));
        break;
      }
      case Token::OperatorType::greater: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kGT, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  10));
        break;
      }
      case Token::OperatorType::greaterequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kGE, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  10));
        break;
      }

      case Token::OperatorType::equalequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kEQ, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  9));
        break;
      }
      case Token::OperatorType::exclaimequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kNE, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  9));
        break;
      }

      case Token::OperatorType::amp: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kAnd, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  8));
        break;
      }

      case Token::OperatorType::caret: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kXor, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  7));
        break;
      }

      case Token::OperatorType::pipe: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kOr, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  6));
        break;
      }

      case Token::OperatorType::ampamp: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kLAnd, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  5));
        break;
      }

      case Token::OperatorType::pipepipe: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kLOr, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  4));
        break;
      }

      case Token::OperatorType::equal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::plusequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kAddAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::minusequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kSubAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::starequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kMulAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::slashequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kDivAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::percentequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kRemAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::ampequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kAndAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::caretequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kXorAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::pipeequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kOrAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::lesslessequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kShlAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }
      case Token::OperatorType::greatergreaterequal: {
        index++;
        expr = new Ast::Binary(
            Ast::Binary::Operator::kShrAssign, expr,
            ParseBinaryExpression(token, length, index,
                                  ParsePrimaryExpression(token, length, index),
                                  2));
        break;
      }

      default:
        // LOGGING_INFO("OOOOOOOOOOOOOOOOOOOO1OOOOOOOOOOOOOOOO");
        return expr;
    }
  }

  // LOGGING_INFO("OOOOOOOOOOOOOOOOOOOO2OOOOOOOOOOOOOOOO");

  return expr;
}

unsigned int Parser::ExpressionParser::GetPriority(Token token) {
  if (token == Token::Type::OPERATOR) {
    switch (token.value.oper) {
      case Token::OperatorType::star:
      case Token::OperatorType::slash:
      case Token::OperatorType::percent:
        return 13;
      case Token::OperatorType::plus:
      case Token::OperatorType::minus:
        return 12;
      case Token::OperatorType::lessless:
      case Token::OperatorType::greatergreater:
        return 11;
      case Token::OperatorType::less:
      case Token::OperatorType::lessequal:
      case Token::OperatorType::greater:
      case Token::OperatorType::greaterequal:
        return 10;
      case Token::OperatorType::equalequal:
      case Token::OperatorType::exclaimequal:
        return 9;
      case Token::OperatorType::amp:
        return 8;
      case Token::OperatorType::caret:
        return 7;
      case Token::OperatorType::pipe:
        return 6;
      case Token::OperatorType::ampamp:
        return 5;
      case Token::OperatorType::pipepipe:
        return 4;
      case Token::OperatorType::question:
        return 3;
      case Token::OperatorType::equal:
      case Token::OperatorType::plusequal:
      case Token::OperatorType::minusequal:
      case Token::OperatorType::starequal:
      case Token::OperatorType::slashequal:
      case Token::OperatorType::percentequal:
      case Token::OperatorType::ampequal:
      case Token::OperatorType::caretequal:
      case Token::OperatorType::pipeequal:
      case Token::OperatorType::lesslessequal:
      case Token::OperatorType::greatergreaterequal:
        return 2;
      case Token::OperatorType::comma:
        return 1;
      default:
        return 0;
    }
  }
  return 0;
}

Ast::Expression* Parser::ExpressionParser::ParsePrimaryExpression(
    Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");

  LOGGING_INFO("Parsing primary expression.");

  enum class State { kPreOper, kPostOper, kEnd };
  State state = State::kPreOper;
  Ast::Expression* full_expression = nullptr;
  Ast::Expression* main_expression = nullptr;
  Ast::Expression* pre_operator_expression = nullptr;

  while (state != State::kEnd && index < length) {
    if (token[index] == Token::Type::OPERATOR) {
      LOGGING_INFO("Parsing operator: " +
                   std::to_string(static_cast<int>(token[index].value.oper)));
      switch (token[index].value.oper) {
        case Token::OperatorType::plus:  // +
          if (state == State::kPreOper) {
            // The plus expression has no effect, skip it.
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::minus:  // -
          if (state == State::kPreOper) {
            ParsePrimaryGeneralPreExpression(Ast::Unary::Operator::kMinus,
                                             full_expression,
                                             pre_operator_expression);
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::exclaim:  // !
          if (state == State::kPreOper) {
            ParsePrimaryGeneralPreExpression(Ast::Unary::Operator::kNot,
                                             full_expression,
                                             pre_operator_expression);
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::tilde:  // ~
          if (state == State::kPreOper) {
            ParsePrimaryGeneralPreExpression(Ast::Unary::Operator::kBitwiseNot,
                                             full_expression,
                                             pre_operator_expression);
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::l_square:  // [
          if (state == State::kPostOper) {
            ParseArrayExpression(token, length, index, full_expression,
                                 main_expression, pre_operator_expression);
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::r_square:  // ]
          state = State::kEnd;
          break;

        case Token::OperatorType::l_paren:  // (
          if (state == State::kPreOper) {
            index++;

            ParseParenthesizedExpression(ParseExpression(token, length, index),
                                         full_expression, main_expression,
                                         pre_operator_expression);

            if (!(token[index] == Token::OperatorType::r_paren)) {
              LOGGING_WARNING("Expected ')', but not found ')'.");
            } else {
              index++;
            }

            state = State::kPostOper;

          } else if (state == State::kPostOper) {
            LOGGING_INFO("Parsing function call expression.");
            ParseFunctionCallExpression(token, length, index, full_expression,
                                        main_expression,
                                        pre_operator_expression);

          } else {
            state = State::kEnd;
          }
          break;

        case Token::OperatorType::r_paren:  // )
          state = State::kEnd;
          break;

        case Token::OperatorType::plusplus: {  // ++
          if (state == State::kPreOper) {
            ParseIncrementAndDecrementOperatorsExpression(
                true, true, full_expression, pre_operator_expression);
          } else {
            ParseIncrementAndDecrementOperatorsExpression(
                false, true, full_expression, pre_operator_expression);
          }
          index++;
          break;
        }

        case Token::OperatorType::minusminus: {  // --
          if (state == State::kPreOper) {
            ParseIncrementAndDecrementOperatorsExpression(
                true, false, full_expression, pre_operator_expression);
          } else {
            ParseIncrementAndDecrementOperatorsExpression(
                false, false, full_expression, pre_operator_expression);
          }
          index++;
          break;
        }

        case Token::OperatorType::period: {  // .
          index++;
          if (index >= length) LOGGING_ERROR("index is out of range.");
          Ast::Identifier* identifier_node = new Ast::Identifier(token[index]);
          if (identifier_node == nullptr)
            INTERNAL_ERROR("identifier_node is nullptr.");
          index++;

          Ast::Binary* binary_node = new Ast::Binary(
              Ast::Binary::Operator::kMember, main_expression, identifier_node);
          if (binary_node == nullptr) INTERNAL_ERROR("binary_node is nullptr.");

          if (full_expression == main_expression) {
            full_expression = main_expression = binary_node;
          } else {
            HandlePreOperatorExpression(pre_operator_expression, binary_node);
            pre_operator_expression = main_expression = binary_node;
          }

          // Continues reading the expression if the next token is a period.
          if (!(token[index] == Token::OperatorType::period))
            state = State::kPostOper;
          break;
        }

        default:
          state = State::kEnd;
          break;
      }

    } else if (token[index] == Token::Type::IDENTIFIER) {
      Ast::Identifier* identifier_node = new Ast::Identifier(token[index]);
      if (identifier_node == nullptr)
        INTERNAL_ERROR("identifier_node is nullptr.");
      index++;

      if (full_expression == nullptr || pre_operator_expression == nullptr) {
        full_expression = main_expression = identifier_node;
      } else {
        HandlePreOperatorExpression(pre_operator_expression, identifier_node);
        main_expression = identifier_node;
      }

      // Continues reading the expression if the next token is a period.
      if (!(token[index] == Token::OperatorType::period)) {
        state = State::kPostOper;
      } else {
        LOGGING_INFO("DEBUG POINT 1");
      }

    } else if (token[index] == Token::Type::NUMBER ||
               token[index] == Token::Type::CHARACTER ||
               token[index] == Token::Type::STRING) {
      Ast::Value* value = new Ast::Value(token[index]);
      if (value == nullptr) INTERNAL_ERROR("value is nullptr.");
      index++;

      if (full_expression == nullptr || pre_operator_expression == nullptr) {
        full_expression = main_expression = value;
      } else {
        HandlePreOperatorExpression(pre_operator_expression, value);
        main_expression = value;
      }

      state = State::kEnd;

    } else if (token[index] == Token::Type::KEYWORD) {
      switch (token[index].value.keyword) {
        case Token::KeywordType::True:
        case Token::KeywordType::False: {
          Ast::Value* bool_node = new Ast::Value(token[index]);
          if (bool_node == nullptr) INTERNAL_ERROR("value is nullptr.");
          index++;

          if (full_expression == nullptr ||
              pre_operator_expression == nullptr) {
            full_expression = main_expression = bool_node;
          } else {
            HandlePreOperatorExpression(pre_operator_expression, bool_node);
            main_expression = bool_node;
          }

          state = State::kEnd;
          break;
        }

        default:
          LOGGING_ERROR("Unexpected keyword \"" +
                        std::string(Token::GetKeywordTypeString(
                                        token[index].value.keyword) +
                                    "\"."));
      }

    } else {
      state = State::kEnd;
    }
  }

  if (full_expression == nullptr) INTERNAL_ERROR("full_expression is nullptr.");
  return full_expression;
}

void Parser::ExpressionParser::ParsePrimaryGeneralPreExpression(
    Ast::Unary::Operator unary_operator, Ast::Expression*& full_expression,
    Ast::Expression*& pre_operator_expression) {
  Ast::Unary* unary_expression = new Ast::Unary(unary_operator, nullptr);
  if (full_expression == nullptr || pre_operator_expression == nullptr) {
    full_expression = pre_operator_expression = unary_expression;
  } else {
    HandlePreOperatorExpression(pre_operator_expression, unary_expression);
    pre_operator_expression = unary_expression;
  }
}

void Parser::ExpressionParser::HandlePreOperatorExpression(
    Ast::Expression*& pre_operator_expression,
    Ast::Expression* new_expression) {
  if (pre_operator_expression != nullptr) {
    Ast::Unary* old_expression = Ast::Cast<Ast::Unary>(pre_operator_expression);
    pre_operator_expression =
        new Ast::Unary(old_expression->GetOperator(), new_expression);
    delete old_expression;
  } else {
    LOGGING_WARNING("pre_operator_expression is nullptr.");
  }
}

void Parser::ExpressionParser::ParseParenthesizedExpression(
    Ast::Expression* new_expression, Ast::Expression*& full_expression,
    Ast::Expression*& main_expression,
    Ast::Expression*& pre_operator_expression) {
  if (full_expression == nullptr || pre_operator_expression == nullptr) {
    full_expression = main_expression = new_expression;
  } else {
    Ast::Expression* full_expression_node = new_expression;
    HandlePreOperatorExpression(pre_operator_expression, full_expression_node);
  }
}

void Parser::ExpressionParser::ParseArrayExpression(
    Token* token, std::size_t length, std::size_t& index,
    Ast::Expression*& full_expression, Ast::Expression*& main_expression,
    Ast::Expression*& pre_operator_expression) {
  index++;

  Ast::Array* array_node =
      new Ast::Array(main_expression, ParseExpression(token, length, index));

  if (full_expression == nullptr || pre_operator_expression == nullptr) {
    full_expression = main_expression = array_node;
  } else {
    HandlePreOperatorExpression(pre_operator_expression, array_node);
    main_expression = array_node;
  }

  // Skips the r_square operator.
  if (!(token[index] == Token::OperatorType::r_square)) {
    LOGGING_WARNING("Expected ']', but not found ']'.");
  } else {
    index++;
  }
}

void Parser::ExpressionParser::ParseFunctionCallExpression(
    Token* token, std::size_t length, std::size_t& index,
    Ast::Expression*& full_expression, Ast::Expression*& main_expression,
    Ast::Expression*& pre_operator_expression) {
  LOGGING_INFO("Parsing function call expression.");

  index++;

  std::vector<Ast::Expression*> arguments;
  bool is_variadic = false;

  // Parses the arguments of the function call.
  while (index < length &&
         token[index].value.oper != Token::OperatorType::r_paren) {
    // Handles variable parameters.
    if (token[index] == Token::OperatorType::ellipsis) {
      is_variadic = true;

      if (!(token[index + 1] == Token::OperatorType::r_paren)) {
        LOGGING_WARNING("Expected ')' after '...', but not fonud ')'.");
      } else {
        index++;
      }

      break;
    }

    if (token[index] == Token::OperatorType::comma)
      LOGGING_WARNING("Unexpected comma in function arguments.");

    arguments.push_back(ParseExpressionWithoutComma(token, length, index));

    if (token[index] == Token::OperatorType::comma) {
      index++;
    } else if (token[index] == Token::OperatorType::r_paren) {
      break;
    } else {
      LOGGING_WARNING("Unexpected token in arguments.");
      break;
    }
  }

  if (!(token[index] == Token::OperatorType::r_paren)) {
    LOGGING_WARNING("Expected ')' after arguments, but not fonud ')'.");
  } else {
    index++;
  }

  Ast::Function* function = nullptr;
  if (main_expression != nullptr &&
      *main_expression == Ast::Statement::StatementType::kBinary) {
    LOGGING_INFO("Handling function with scopes.");
    // Handles the function with scopes.
    Ast::Binary* old_expression = Ast::Cast<Ast::Binary>(main_expression);
    function = new Ast::Function(old_expression->GetRightExpression(),
                                 arguments, is_variadic);
    Ast::Binary* new_expression =
        new Ast::Binary(old_expression->GetOperator(),
                        old_expression->GetLeftExpression(), function);

    if (full_expression == nullptr || pre_operator_expression == nullptr) {
      full_expression = main_expression = new_expression;
    } else {
      HandlePreOperatorExpression(pre_operator_expression, new_expression);
      main_expression = new_expression;
    }

    delete old_expression;

  } else {
    // Handles the function without scopes.
    function = new Ast::Function(main_expression, arguments, is_variadic);
    if (full_expression == nullptr || pre_operator_expression == nullptr) {
      full_expression = main_expression = function;
    } else {
      HandlePreOperatorExpression(pre_operator_expression, function);
      main_expression = function;
    }
  }
}

void Parser::ExpressionParser::ParseIncrementAndDecrementOperatorsExpression(
    bool is_pre_operator, bool is_increment, Ast::Expression*& full_expression,
    Ast::Expression*& pre_operator_expression) {
  // Gets the operator type based on whether it is an increment or decrement
  // operator and whether it is a pre- or post-operator.
  Ast::Unary::Operator oper;
  if (is_increment) {
    if (is_pre_operator) {
      oper = Ast::Unary::Operator::kPreInc;
    } else {
      oper = Ast::Unary::Operator::kPostInc;
    }
  } else {
    if (is_pre_operator) {
      oper = Ast::Unary::Operator::kPreDec;
    } else {
      oper = Ast::Unary::Operator::kPostDec;
    }
  }

  Ast::Unary* unary = nullptr;
  if (is_pre_operator) {
    unary = new Ast::Unary(oper, nullptr);
    if (full_expression == nullptr || pre_operator_expression == nullptr) {
      pre_operator_expression = full_expression = unary;
    } else {
      HandlePreOperatorExpression(pre_operator_expression, unary);
      pre_operator_expression = unary;
    }

  } else {
    unary = new Ast::Unary(oper, full_expression);
    full_expression = unary;
  }
}

}  // namespace Aq