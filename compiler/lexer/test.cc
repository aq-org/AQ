// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include <cstring>
#include <fstream>
#include <iostream>

#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"

int main(int argc, char *argv[]) {
  // Read files
  std::ifstream file;
  const char* filename =argv[1];
  file.open(filename);
  if (!file.is_open()) {
    std::cerr << "Can't open file." << std::endl;
    return 1;
  }
  std::string str;
  std::string line;
  while (std::getline(file, line)) {
    str += line;
  }
  file.close();
  char code[str.size() + 1];
  std::strcpy(code, str.c_str());
  Aq::Compiler::Lexer lexer(code, sizeof(code));
  Aq::Compiler::Token token;
  if (code[str.size()] == '\0') {
    while (1) {
      int return_value = lexer.LexToken(token);
      std::cout << std::string(token.location, token.length) << std::endl;
      if (lexer.IsReadEnd() == true) {
        break;
      }
    }
  } else {
    std::cout << "Read file error.";
  }
  return 0;
}