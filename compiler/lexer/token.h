// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_TOKEN_H_
#define AQ_COMPILER_LEXER_TOKEN_H_

namespace Aq {
namespace Compiler {
struct Token {
  enum Type {};
  Type type;
  char* token;
};
}  // namespace Compiler
}  // namespace Aq
#endif