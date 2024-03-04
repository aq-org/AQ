// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/ast/stmt.h"

#include "compiler/ast/ast.h"

namespace Aq {
Compiler::Ast::Stmt::Stmt() = default;
Compiler::Ast::Stmt::~Stmt() = default;

Compiler::Ast::DeclStmt::DeclStmt() = default;
Compiler::Ast::DeclStmt::~DeclStmt() = default;

Compiler::Ast::NullStmt::NullStmt() = default;
Compiler::Ast::NullStmt::~NullStmt() = default;

Compiler::Ast::CompoundStmt::CompoundStmt() = default;
Compiler::Ast::CompoundStmt::~CompoundStmt() = default;

Compiler::Ast::SwitchCase::SwitchCase() = default;
Compiler::Ast::SwitchCase::~SwitchCase() = default;

Compiler::Ast::CaseStmt::CaseStmt() = default;
Compiler::Ast::CaseStmt::~CaseStmt() = default;

Compiler::Ast::DefaultStmt::DefaultStmt() = default;
Compiler::Ast::DefaultStmt::~DefaultStmt() = default;

Compiler::Ast::LabelStmt::LabelStmt() = default;
Compiler::Ast::LabelStmt::~LabelStmt() = default;

Compiler::Ast::AttributedStmt::AttributedStmt() = default;
Compiler::Ast::AttributedStmt::~AttributedStmt() = default;

Compiler::Ast::IfStmt::IfStmt() = default;
Compiler::Ast::IfStmt::~IfStmt() = default;

Compiler::Ast::SwitchStmt::SwitchStmt() = default;
Compiler::Ast::SwitchStmt::~SwitchStmt() = default;

Compiler::Ast::WhileStmt::WhileStmt() = default;
Compiler::Ast::WhileStmt::~WhileStmt() = default;

Compiler::Ast::DoStmt::DoStmt() = default;
Compiler::Ast::DoStmt::~DoStmt() = default;

Compiler::Ast::ForStmt::ForStmt() = default;
Compiler::Ast::ForStmt::~ForStmt() = default;

Compiler::Ast::GotoStmt::GotoStmt() = default;
Compiler::Ast::GotoStmt::~GotoStmt() = default;

Compiler::Ast::IndirectGotoStmt::IndirectGotoStmt() = default;
Compiler::Ast::IndirectGotoStmt::~IndirectGotoStmt() = default;

Compiler::Ast::ContinueStmt::ContinueStmt() = default;
Compiler::Ast::ContinueStmt::~ContinueStmt() = default;

Compiler::Ast::BreakStmt::BreakStmt() = default;
Compiler::Ast::BreakStmt::~BreakStmt() = default;

Compiler::Ast::ReturnStmt::ReturnStmt() = default;
Compiler::Ast::ReturnStmt::~ReturnStmt() = default;

Compiler::Ast::AsmStmt::AsmStmt() = default;
Compiler::Ast::AsmStmt::~AsmStmt() = default;

Compiler::Ast::SEHExceptStmt::SEHExceptStmt() = default;
Compiler::Ast::SEHExceptStmt::~SEHExceptStmt() = default;

Compiler::Ast::SEHFinallyStmt::SEHFinallyStmt() = default;
Compiler::Ast::SEHFinallyStmt::~SEHFinallyStmt() = default;

Compiler::Ast::SEHTryStmt::SEHTryStmt() = default;
Compiler::Ast::SEHTryStmt::~SEHTryStmt() = default;

Compiler::Ast::SEHLeaveStmt::SEHLeaveStmt() = default;
Compiler::Ast::SEHLeaveStmt::~SEHLeaveStmt() = default;

Compiler::Ast::CapturedStmt::CapturedStmt() = default;
Compiler::Ast::CapturedStmt::~CapturedStmt() = default;

// Wait development.

}  // namespace Aq