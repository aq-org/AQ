// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in the root directory.

#ifndef AQ_COMPILER_PARSER_PARSER_H_
#define AQ_COMPILER_PARSER_PARSER_H_

#include <vector>
#include "compiler/token/token.h"
#include "compiler/ast/ast.h"

namespace Aq {
namespace Compiler {

class Parser {
 public:
  static CompoundNode* Parse(const std::vector<Token>& tokens);
  static ExprNode* ParseExpr(Token* tokens, std::size_t length, std::size_t& index);
  static ExprNode* ParseExprWithoutComma(Token* tokens, std::size_t length, std::size_t& index);
  static ExprNode* ParsePrimaryExpr(Token* tokens, std::size_t length, std::size_t& index);
  static StmtNode* ParseStmt(Token* tokens, std::size_t length, std::size_t& index);
  static VarDeclNode* ParseVarDecl(Token* tokens, std::size_t length, std::size_t& index);
  static FuncDeclNode* ParseFuncDecl(Token* tokens, std::size_t length, std::size_t& index);
  static ClassDeclNode* ParseClassDecl(Token* tokens, std::size_t length, std::size_t& index);
  static StaticNode* ParseStatic(Token* tokens, std::size_t length, std::size_t& index);
  static ExprNode* ParseBinaryExpr(Token* tokens, std::size_t length, std::size_t& index, ExprNode* left, int min_priority);
  static ExprNode* ParseBinaryExprWithoutComma(Token* tokens, std::size_t length, std::size_t& index, ExprNode* left, int min_priority);

 private:
  static bool IsDecl(Token* tokens, std::size_t length, std::size_t index);
  static bool IsFuncDecl(Token* tokens, std::size_t length, std::size_t index);
  static bool IsClassDecl(Token* tokens, std::size_t length, std::size_t index);
  static unsigned int GetPriority(const Token& token);
};

}  // namespace Compiler
}  // namespace Aq

#endif  // AQ_COMPILER_PARSER_PARSER_H_