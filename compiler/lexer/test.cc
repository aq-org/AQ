// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

#include "aq/aq.h"
#include "compiler/lexer/lex_map.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"

int LexerTest(int argc, char* argv[]) {
  std::ifstream file;
  const char* filename = argv[1];
  file.open(filename);
  if (!file.is_open()) {
    std::cerr << "Can't open file." << std::endl;
    return 1;
  }

  std::vector<char> code;
  char ch;
  while (file.get(ch)) {
    code.push_back(ch);
  }
  code.push_back('\0');
  file.close();

  Aq::Compiler::Lexer lexer(code.data(), code.size() - 1);
  Aq::Compiler::Token token;

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

  return 0;
}

int LexMapTest(int argc, char* argv[]) {

  return 0;
}

int main(int argc, char* argv[]) {
  LexerTest(argc, argv);
  LexMapTest(argc, argv);
  return 0;
}
