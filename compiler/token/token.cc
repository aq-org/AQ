// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/token/token.h"

#include "compiler/compiler.h"
#include "debugger/debugger.h"

namespace Aq {
Compiler::Token::Token() = default;
Compiler::Token::~Token() = default;

void Compiler::Token::SetKind(Kind kind) {
    this->kind_ = kind;
}

Compiler::Token::Kind Compiler::Token::GetKind() const {
    return kind_;
}

void Compiler::Token::SetDataPtr(void* data_ptr) {
    this->data_ptr_ = data_ptr;
}

void* Compiler::Token::GetDataPtr() const {
    return data_ptr_;
}
}  // namespace Aq