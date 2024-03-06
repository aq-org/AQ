// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILE_COMPILE_H_
#define AQ_COMPILE_COMPILE_H_

namespace Aq {
class Compiler {
 public:
  Compiler(const char* filename);
  ~Compiler();

 private:
  char* buffer_ptr_;

  class Lexer;
  class Parser;

  template <typename T>
  class HashMap;
  class DynArray;
  class TokenMap;
  class Token;
  class Ast;

  // Will be replaced by HashMap.
  template <typename T>
  class LexMap;
};
}  // namespace Aq

#endif  // AQ_COMPILE_H_