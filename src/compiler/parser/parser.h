// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_PARSER_PARSER_H_
#define AQ_COMPILER_PARSER_PARSER_H_

#include <cstddef>

#include "compiler/ast/ast.h"
#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {
class Parser {
 public:
  Parser() = default;
  ~Parser() = default;

  // Parses the tokens and returns the root of the AST.
  static Ast::Compound* Parse(std::vector<Token> token);
  static Ast::Expression* ParsePrimaryExpression(Token* token,
                                                 std::size_t length,
                                                 std::size_t& index);

 private:
  class ExpressionParser;

  class DeclarationParser;

  // Performs syntax analysis on general statements. Returns the statement.
  static Ast::Statement* ParseStatement(Token* token, std::size_t length,
                                        std::size_t& index);

  static Ast::Declaration* ParseDeclaration(Token* token, std::size_t length,
                                            std::size_t& index);

  static Ast::Statement* ParseStatementWithOperator(Token* token,
                                                    std::size_t length,
                                                    std::size_t& index);
};
}  // namespace Compiler
}  // namespace Aq

#endif