// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/compiler.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "compiler/lexer/lexer.h"
#include "compiler/logging/logging.h"
#include "compiler/token/token.h"

int main(int argc, char* argv[]) {
  // TODO(command-line arguments): Add more command-line arguments and
  // related-functions for the compiler.
  if (argc < 3) {
    Aq::Compiler::Logging::ERROR(
        __FUNCTION__, "Usage: " + std::string(argv[0]) + " <code> <output>");
    return -1;
  }

  std::vector<char> code;
  Aq::Compiler::ReadCodeFromFile(argv[1], code);

  std::vector<Aq::Compiler::Token> token;
  Aq::Compiler::LexCode(code, token);

  Aq::Compiler::CompoundNode* ast = Aq::Compiler::Parser::Parse(token);
  if (ast == nullptr)
    Aq::Compiler::Logging::ERROR(__FUNCTION__, "ast is nullptr.");

  Aq::Compiler::BytecodeGenerator bytecode_generator;
  bytecode_generator.GenerateBytecode(ast, argv[2]);

  Aq::Compiler::Logging::INFO(__FUNCTION__, "Generate Bytecode SUCCESS!");

  return 0;
}

namespace Aq {
namespace Compiler {
void ReadCodeFromFile(const char* filename, std::vector<char>& code) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs.is_open()) {
    Logging::ERROR(__FUNCTION__,
                   "Error: Could not open file " + std::string(filename));
  }

  std::streampos size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  ifs.read(buffer.data(), size);
}

void LexCode(std::vector<char>& code, std::vector<Token>& tokens) {
  Aq::Compiler::Lexer lexer(code.data(), code.size());
  Aq::Compiler::Token first_token;
  lexer.LexToken(first_token, first_token);
  tokens.push_back(first_token);
  while (true) {
    Aq::Compiler::Token token_buffer;
    lexer.LexToken(tokens.back(), token_buffer);
    tokens.push_back(token_buffer);
    if (lexer.IsReadEnd()) {
      break;
    }
  }
}
}  // namespace Compiler
}  // namespace Aq