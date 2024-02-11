// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEXER_H_
#define AQ_COMPILER_LEXER_LEXER_H_

#include <cstddef>

namespace Aq {
namespace Compiler {
template <typename T1, typename T2>
class LexMap {
 public:
  LexMap(size_t init_capacity);
  ~LexMap();

  int Hash(T1 key);
  void Insert(T1 key, T2 value);

 private:
  struct Pair {
    T1 key;
    T2 value;
  };
};
}  // namespace Compiler
}  // namespace Aq

#endif