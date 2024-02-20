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
Compiler::Compiler(const char* filename) {
  // TODO: Waiting for improvements.
  auto start = std::chrono::high_resolution_clock::now();
  std::ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    Aq::Debugger error_info(Aq::Debugger::Level::ERROR, "Aq::Main",
                            "Main_ReadFileError", "Can't open file.", nullptr);
    return;
  }

  std::vector<char> code;
  char ch;
  while (file.get(ch)) {
    code.push_back(ch);
  }
  code.push_back('\0');
  file.close();
  buffer_ptr_ = code.data();
  Lexer lexer(buffer_ptr_, code.size());
  Token token;
  while (true) {
    lexer.LexToken(token);
    if (token.type == Token::Type::NONE) {
      std::cout << "END OF THE CODE.";
    } else {
      std::cout << "" << std::endl;
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
}
Compiler::~Compiler() = default;
}  // namespace Aq