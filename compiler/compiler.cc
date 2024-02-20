// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/compiler.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "debugger/debugger.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"

namespace Aq {
Compiler::Compiler() = default;
Compiler::~Compiler() = default;

int Compiler::CompileFile(const char* filename) {
  // TODO: Waiting for improvements.
  auto start = std::chrono::high_resolution_clock::now();
  std::ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    Aq::Debugger error_info(Aq::Debugger::Level::ERROR, "Aq::Main",
                            "Main_ReadFileError", "Can't open file.", nullptr);
    return -1;
  }

  std::vector<char> code;
  char ch;
  while (file.get(ch)) {
    code.push_back(ch);
  }
  code.push_back('\0');
  file.close();
  Lexer lexer(code.data(), code.size() - 1);
  Token token;
  while (true) {
    int return_value = lexer.LexToken(token);
    if (token.length == 0) {
      std::cout << "END OF THE CODE.";
    } else {
      std::cout << std::string(token.location, token.length) << std::endl;
    }
    if (lexer.IsReadEnd()) {
      break;
    }
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto duration_in_milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  double duration_in_seconds =
      static_cast<double>(duration_in_milliseconds.count()) / 1000.0;
  std::cout << "Execution time: " << duration_in_seconds << " seconds.\n";
  return 0;
}
}  // namespace Aq