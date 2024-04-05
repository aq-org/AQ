/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#ifndef AQ_COMPILER_AST_STMT_H_
#define AQ_COMPILER_AST_STMT_H_

#include <cstddef>

#include "compiler/ast/ast.h"
#include "compiler/compiler.h"

namespace Aq {
class Compiler::Ast::Stmt {
 public:
  Stmt();
  virtual ~Stmt() = 0;

 protected:
  Stmt* next_ptr_;
};

class Compiler::Ast::DeclStmt : public Compiler::Ast::Stmt {
 public:
  DeclStmt();
  ~DeclStmt();

  void SetDecl(Decl* decl);
  Decl* GetDecl();

 private:
  Decl* decl_;
};

class Compiler::Ast::NullStmt : public Compiler::Ast::Stmt {
 public:
  NullStmt();
  ~NullStmt();
};

class Compiler::Ast::CompoundStmt : public Compiler::Ast::Stmt {
 public:
  CompoundStmt();
  ~CompoundStmt();

  void SetName(char* name);
  char* GetName();

  void AddStmt(Stmt* stmt);
  Stmt* GetStmt(int index);

 private:
  Stmt* stmt_;
};

class Compiler::Ast::SwitchCase : public Compiler::Ast::Stmt {
 public:
  SwitchCase();
  ~SwitchCase();

  virtual void AddStmt(Stmt* stmt);

 protected:
  Stmt* stmt_;
};

class Compiler::Ast::CaseStmt : public Compiler::Ast::SwitchCase {
 public:
  CaseStmt();
  ~CaseStmt();

  void AddLabel(char* label);
  char* GetLabel();
  Stmt* GetStmt(char* label);

 private:
  char* label_;
};

class Compiler::Ast::DefaultStmt : public Compiler::Ast::SwitchCase {
 public:
  DefaultStmt();
  ~DefaultStmt();

  Stmt* GetStmt();
};

class Compiler::Ast::LabelStmt : public Compiler::Ast::SwitchCase {
 public:
  LabelStmt();
  ~LabelStmt();

  void AddLabel(char* label);
  void AddStmt(Stmt* stmt);
  char* GetLabel();
  Stmt* GetStmt(char* label);

 private:
  Stmt* stmt_;
  char* label_;
};

class Compiler::Ast::AttributedStmt : public Compiler::Ast::Stmt {
 public:
  AttributedStmt();
  ~AttributedStmt();

  void AddStmt(Stmt* stmt);
  Stmt* GetStmt();

 private:
  Stmt* stmt_;
};

class Compiler::Ast::IfStmt : public Compiler::Ast::Stmt {
 public:
  IfStmt();
  ~IfStmt();

  void AddCondition(Expr* condition);
  void AddTrueStmt(Stmt* true_stmt);
  Expr* GetCondition();
  Stmt* GetTrueStmt();
  
  void AddFalseStmt(Stmt* false_stmt);
  Stmt* GetFalseStmt();

 private:
  Expr* condition_;
  Stmt* true_stmt_;
  Stmt* false_stmt_;
};

class Compiler::Ast::SwitchStmt : public Compiler::Ast::Stmt {
 public:
  SwitchStmt();
  ~SwitchStmt();

  void AddCondition(char* condition);
  void AddLabel(Stmt* label);
  char* GetCondition();
  Stmt* GetLabel(char* label);

 private:
  char* condition_;
  SwitchCase* label_;
};

class Compiler::Ast::WhileStmt : public Compiler::Ast::Stmt {
 public:
  WhileStmt();
  ~WhileStmt();

  void Addcondition(Expr* condition);
  void AddStmt(Stmt* stmt);
  Expr* GetCondition();
  Stmt* GetStmt();

 private:
  Expr* condition_;
  Stmt* stmt_;
};

class Compiler::Ast::DoStmt : public Compiler::Ast::Stmt {
 public:
  DoStmt();
  ~DoStmt();

  void AddCondition(Expr* condition);
  void AddStmt(Stmt* stmt);
  Expr* GetCondition();
  Stmt* GetStmt();

 private:
  Expr* condition_;
  Stmt* stmt_;
};

class Compiler::Ast::ForStmt : public Compiler::Ast::Stmt {
 public:
  ForStmt();
  ~ForStmt();

  void AddInit(Expr* init);
  void AddCondition(Expr* condition);
  void AddStep(Expr* step);
  void AddStmt(Stmt* stmt);
  Expr* GetInit();
  Expr* GetCondition();
  Expr* GetStep();
  Stmt* GetStmt();

 private:
  Expr* init_;
  Expr* condition_;
  Expr* step_;
  Stmt* stmt_;
};

class Compiler::Ast::GotoStmt : public Compiler::Ast::Stmt {
 public:
  GotoStmt();
  ~GotoStmt();

  void AddLabel(char* label);
  char* GetLabel();

 private:
  char* label_;
};

class Compiler::Ast::IndirectGotoStmt : public Compiler::Ast::Stmt {
 public:
  IndirectGotoStmt();
  ~IndirectGotoStmt();

  void AddExpr(Expr* expr);
  Expr* GetExpr();

 private:
  Expr* expr_;
};

class Compiler::Ast::ContinueStmt : public Compiler::Ast::Stmt {
 public:
  ContinueStmt();
  ~ContinueStmt();
};

class Compiler::Ast::BreakStmt : public Compiler::Ast::Stmt {
 public:
  BreakStmt();
  ~BreakStmt();
};

class Compiler::Ast::ReturnStmt : public Compiler::Ast::Stmt {
 public:
  ReturnStmt();
  ~ReturnStmt();

  void AddReturnValue(Expr* return_value);
  Expr* GetReturnValue();

 private:
  Expr* return_value_;  
};

class Compiler::Ast::AsmStmt : public Compiler::Ast::Stmt {
 public:
  AsmStmt();
  ~AsmStmt();

  void AddAsm(Expr* asm_);
  Expr* GetAsm();

 private:
  Expr* asm_;
};

class Compiler::Ast::SEHExceptStmt : public Compiler::Ast::Stmt {
 public:
  SEHExceptStmt();
  ~SEHExceptStmt();

  void AddCondition(Expr* condition);
  void AddHandleStmt(Stmt* stmt);
  Expr* GetCondition();
  Stmt* GetHandleStmt();

 private:
  Expr* Condition_;
  Stmt* HandleStmt_;
};

class Compiler::Ast::SEHFinallyStmt : public Compiler::Ast::Stmt {
 public:
  SEHFinallyStmt();
  ~SEHFinallyStmt();

  void AddStmt(Stmt* stmt);
  Stmt* GetStmt();

 private:
  Stmt* Stmt_;
};

class Compiler::Ast::SEHTryStmt : public Compiler::Ast::Stmt {
 public:
  SEHTryStmt();
  ~SEHTryStmt();

  void AddStmt(Stmt* stmt);
  Stmt* GetStmt();

 private:
  Stmt* Stmt_;
};

class Compiler::Ast::SEHLeaveStmt : public Compiler::Ast::Stmt {
 public:
  SEHLeaveStmt();
  ~SEHLeaveStmt();
};

class Compiler::Ast::CapturedStmt : public Compiler::Ast::Stmt {
 public:
  CapturedStmt();
  ~CapturedStmt();

  void AddCaptureList(Expr* capture_list);
  Expr* GetCaptureList();
  void AddStmt(Stmt* stmt);
  Stmt* GetStmt();

 private:
  Expr* capture_list_;
  Stmt* stmt_;
};

}  ///  namespace Aq

#endif