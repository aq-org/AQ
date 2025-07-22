// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "token/token.h"

int main(int argc, char* argv[]) {
  // TODO(command-line arguments): Add more command-line arguments and
  // related-functions for the compiler.
  if (argc < 3) {
    Aq::LOGGING_ERROR("Usage: " + std::string(argv[0]) +
                                " <code> <output>");
    return -1;
  }

  // Gets the code from the file.
  std::vector<char> code;
  Aq::ReadCodeFromFile(argv[1], code);

  // Lexes the code and stores it in a vector of tokens.
  std::vector<Aq::Token> token;
  Aq::LexCode(code, token);

  // Parses the code and generates the AST.
  Aq::Ast::Compound* ast = Aq::Parser::Parse(token);
  if (ast == nullptr) Aq::LOGGING_ERROR("ast is nullptr.");

  // Generates the bytecode from the AST.
  Aq::Interpreter::Interpreter interpreter;
  interpreter.Generate(ast, argv[2]);

  Aq::LOGGING_INFO("Generate Bytecode SUCCESS!");

  return 0;
}

namespace Aq {
void ReadCodeFromFile(const char* filename, std::vector<char>& code) {
  std::ifstream ifs(filename, std::ios::in | std::ios::binary | std::ios::ate);
  if (!ifs.is_open()) {
    LOGGING_ERROR("Error: Could not open file " + std::string(filename));
  }

  std::streampos size = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  code.resize(size);
  ifs.read(code.data(), size);
  code.push_back('\0');
  ifs.close();
}

void LexCode(std::vector<char>& code, std::vector<Token>& tokens) {
  Aq::Lexer lexer(code.data(), code.size());

  Aq::Token first_token;
  lexer.LexToken(first_token, first_token);
  tokens.push_back(first_token);

  // Lexes the code until the end of the file.
  while (!lexer.IsReadEnd()) {
    Aq::Token token_buffer;
    lexer.LexToken(tokens.back(), token_buffer);
    tokens.push_back(token_buffer);
  }
}
}  // namespace Aq