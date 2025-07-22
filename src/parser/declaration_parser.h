// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_PARSER_DECLARATION_PARSER_H_
#define AQ_PARSER_DECLARATION_PARSER_H_

#include "ast/ast.h"
#include "parser/parser.h"

namespace Aq {
class Parser::DeclarationParser {
 public:
  DeclarationParser() = delete;
  virtual ~DeclarationParser() = delete;

  // Returns true if the statement is a declaration.
  static bool IsDeclaration(Token* token, std::size_t length,
                            std::size_t index);

  // Returns true if the statement is a function declaration.
  static bool IsFunctionDeclaration(Token* token, std::size_t length,
                                    std::size_t index);

  // Returns true if the statement is a class declaration.
  static bool IsClassDeclaration(Token* token, std::size_t length,
                                 std::size_t index);

  // Performs syntax analysis on variable declarations. Returns the variable.
  static Ast::Variable* ParseVariableDeclaration(Token* token,
                                                 std::size_t length,
                                                 std::size_t& index);

  // Performs syntax analysis on function declarations. Returns the function
  // declaration.
  static Ast::FunctionDeclaration* ParseFunctionDeclaration(Token* token,
                                                            std::size_t length,
                                                            std::size_t& index);

  // Performs syntax analysis on class declarations. Returns the class.
  static Ast::Class* ParseClassDeclaration(Token* token, std::size_t length,
                                           std::size_t& index);

  // Performs syntax analysis on static declarations. Returns the static
  // declaration.
  static Ast::Static* ParseStatic(Token* token, std::size_t length,
                                  std::size_t& index);

 private:
  // Returns true if the statement is a variable declaration.
  static bool HasTypeBeforeExpression(Token* token, std::size_t length,
                                      std::size_t index);

  // Returns true if the statement is a custom type variable declaration.
  static bool HasCustomTypeBeforeExpression(Token* token, std::size_t length,
                                            std::size_t index);

  // Parses the array declaration. Returns the array declaration.
  static Ast::ArrayDeclaration* ParseArrayDeclaration(Ast::Type* type,
                                                      Ast::Expression* name,
                                                      Token* token,
                                                      std::size_t length,
                                                      std::size_t& index);
};
}  // namespace Aq

#endif