// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEXER_H_
#define AQ_COMPILER_LEXER_LEXER_H_

#include <cstddef>

#include "compiler/lexer/token.h"

namespace Aq {
namespace Compiler {
class Lexer {
 public:
  // Initialize the Lexer class and store |source_code| to |buffer_ptr_|.
  Lexer(char* source_code, size_t length);
  ~Lexer();

  // Lexical analysis |buffer_ptr_|, and store the analyzed token into
  // |return_token|.
  int LexToken(Token& return_token);

  // Return true if the lexer is at the end of |buffer_ptr_|.
  bool IsReadEnd();

 private:
  char* buffer_ptr_;
  char* buffer_end_;
  TokenMap token_map_;
};
}  // namespace Compiler
}  // namespace Aq

#endif