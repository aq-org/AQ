// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "aq/aq.h"
#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"

int main(int argc, char* argv[]) {
  // Read files
  std::ifstream file;
  const char* filename = argv[1];
  file.open(filename);
  if (!file.is_open()) {
    std::cerr << "Can't open file." << std::endl;
    return 1;
  }

  // Use std::vector for dynamic memory allocation
  std::vector<char> code;
  char ch;
  while (file.get(ch)) {
    code.push_back(ch);
  }
  code.push_back('\0');  // Null-terminate the code string
  file.close();

  Aq::Compiler::Lexer lexer(
      code.data(), code.size() - 1);  // Pass size excluding null terminator
  Aq::Compiler::Token token;

  while (true) {
    int return_value = lexer.LexToken(token);
    if (token.length == 0) {
      std::cout << "END OF THE CODE. TEST STOP!";
    } else {
      std::cout << std::string(token.location, token.length) << std::endl;
    }
    if (lexer.IsReadEnd()) {
      break;
    }
  }

  return 0;
}
