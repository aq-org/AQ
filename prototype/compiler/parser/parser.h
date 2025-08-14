// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_PARSER_PARSER_H_
#define AQ_COMPILER_PARSER_PARSER_H_

#include <vector>

#include "compiler/ast/ast.h"
#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {

class Parser {
 public:
  static Ast::Compound* Parse(std::vector<Token>& token);
  static Ast::Expression* ParseExpression(Token* tokens, std::size_t length,
                                          std::size_t& index);
  static Ast::Expression* ParseExpressionWithoutComma(Token* tokens,
                                                      std::size_t length,
                                                      std::size_t& index);
  static Ast::Expression* ParsePrimaryExpression(Token* tokens,
                                                 std::size_t length,
                                                 std::size_t& index);
  static Ast::Statement* ParseStatement(Token* tokens, std::size_t length,
                                        std::size_t& index);
  static Ast::Variable* ParseVariableDeclaration(Token* tokens,
                                                 std::size_t length,
                                                 std::size_t& index);
  static Ast::FunctionDeclaration* ParseFunctionDeclaration(Token* tokens,
                                                            std::size_t length,
                                                            std::size_t& index);
  static Ast::Class* ParseClassDeclaration(Token* tokens, std::size_t length,
                                           std::size_t& index);
  static Ast::Static* ParseStatic(Token* tokens, std::size_t length,
                                  std::size_t& index);
  static Ast::Expression* ParseBinaryExpression(Token* tokens,
                                                std::size_t length,
                                                std::size_t& index,
                                                Ast::Expression* left,
                                                int min_priority);
  static Ast::Expression* ParseBinaryExpressionWithoutComma(
      Token* tokens, std::size_t length, std::size_t& index,
      Ast::Expression* left, int min_priority);

 private:
  class DeclarationParser;
  class ExpressionParser;

  static bool IsDecl(Token* tokens, std::size_t length, std::size_t index);
  static bool IsFuncDecl(Token* tokens, std::size_t length, std::size_t index);
  static bool IsClassDecl(Token* tokens, std::size_t length, std::size_t index);
  static unsigned int GetPriority(const Token& token);

  static Ast::Declaration* ParseDeclaration(Token* token, std::size_t length,
                                            std::size_t& index);
  static Ast::Statement* ParseStatementWithOperator(Token* token,
                                                    std::size_t length,
                                                    std::size_t& index);
};

}  // namespace Compiler
}  // namespace Aq

#endif  // AQ_COMPILER_PARSER_PARSER_H_