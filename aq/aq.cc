// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include <iostream>
#include <fstream>
#include <cstring>

#include "compiler/lexer/lexer.h"
#include "compiler/lexer/token.h"

namespace Aq {

int main(int argc, char *argv[]) {
  //read files
  /*std::ifstream file;
  file.open("/home/ax/aq/aq/test.h");
  if (!file.is_open()) {
    std::cerr << "Can't open file: test.h" << std::endl;
    return 1;
  }
  std::string str;
  std::string line;
  while (std::getline(file, line)) {
    str += line;
  }
  file.close();
  // std::string str to char testptr[]
  char testptr[str.size() + 1];
  std::strcpy(testptr, str.c_str());*/
  char testptr[] = "#include \"test.h\\\";a = 10.12312131231231;\000";
  Aq::Compiler::Lexer lexer(testptr, sizeof(testptr));
  Aq::Compiler::Token token;
  int testsize = 0;
  while (1) {
    lexer.LexToken(token);
    std::cout << std::string(token.location,token.length) << std::endl;
    testsize++;
    if (lexer.IsReadEnd() == true) {
      break;
    }
  }
  return 0;
}
}  // namespace Aq

int main(int argc, char *argv[]) { return Aq::main(argc, argv); }