// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_AST_DECL_H_
#define AQ_COMPILER_AST_DECL_H_

#include "compiler/ast/ast.h"
#include "compiler/compiler.h"

namespace Aq {
// TODO: Decl AST
class Compiler::Ast::FuncDecl {
 public:
  FuncDecl();
  ~FuncDecl();

  FuncDecl(const FuncDecl&) = default;
  FuncDecl(FuncDecl&&) noexcept = default;
  FuncDecl& operator=(const FuncDecl&) = default;
  FuncDecl& operator=(FuncDecl&&) noexcept = default;

 private:
  // std::string name;
  // std::vector<Expr*> args;
  // Stmt* body;
  // TODO: Add more.
};
}  // namespace Aq

#endif