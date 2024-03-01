// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_AST_AST_H_
#define AQ_COMPILER_AST_AST_H_

#include "compiler/compiler.h"

namespace Aq {
class Compiler::Ast {
 public:
  Ast();
  ~Ast();

  class FuncDecl;
  class Expr;
  class Stmt;
  class Type;

 private:
  // TODO: Waiting for development.
};
}  // namespace Aq

#endif