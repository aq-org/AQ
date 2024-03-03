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

  class NameInfo;
  class Decl;
  class Expr;
  class Stmt;
  class Type;

  class DeclStmt;
  class NullStmt;
  class CompoundStmt;
  class SwitchCase;
  class CaseStmt;
  class DefaultStmt;
  class LabelStmt;
  class AttributedStmt;
  class IfStmt;
  class SwitchStmt;
  class WhileStmt;
  class DoStmt;
  class ForStmt;
  class GotoStmt;
  class IndirectGotoStmt;
  class ContinueStmt;
  class BreakStmt;
  class ReturnStmt;
  class AsmStmt;
  class SEHExceptStmt;
  class SEHFinallyStmt;
  class SEHTryStmt;
  class SEHLeaveStmt;
  class CapturedStmt;

 private:
  // TODO: Waiting for development.
};
}  // namespace Aq

#endif