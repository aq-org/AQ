// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEXER_H_
#define AQ_COMPILER_LEXER_LEXER_H_

namespace Aq {
namespace Compiler {
class Lexer {
 public:
  // Initialize the Lexer class and store |source_code| to |code_buffer_|.
  Lexer(char* source_code);

 private:
  char* code_buffer_;

  // The basic token type used for preliminary lexical analysis.
  struct BaseToken {
    enum Type {
      // Including keywords and defined identifiers that come with the
      // programming language, etc.
      IDENTIFIER,
      // Contains all operators and other operators with specific meanings.
      OPERATOR,
      // Contains all separators that have no specific meaning.
      SEPARATOR,
      NUMBER,
      CHARACTER,
      STRING,
      COMMENT
    };

    Type type;
    char* token;
  };
};
}  // namespace Compiler
}  // namespace Aq

#endif
