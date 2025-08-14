// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_COMPILER_H_
#define AQ_COMPILER_COMPILER_H_

#include <vector>

#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {
// Reads the code from a file and stores it in a vector.
void ReadCodeFromFile(const char* filename, std::vector<char>& code);

// Lexical analysis of the code and stores a vector of tokens.
void LexCode(std::vector<char>& code, std::vector<Token>& tokens);
}  // namespace Compiler
}  // namespace Aq

#endif