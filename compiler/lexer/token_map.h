// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_TOKEN_MAP_H_
#define AQ_COMPILER_LEXER_TOKEN_MAP_H_

#include "compiler/compiler.h"
#include "compiler/hash_map/hash_map.h"
#include "compiler/token/token.h"

namespace Aq {
/// \class TokenMap
/// \brief Maps tokens to kinds.
class Compiler::TokenMap {
 public:
  TokenMap();
  ~TokenMap();

  /// \fn GetKind
  /// \brief Gets the kind of a token's key.
  /// \param key std::string Type
  /// \return Token::Kind Type of the token's key
  Token::Kind GetKind(std::string key);

  TokenMap(const TokenMap&) = default;
  TokenMap(TokenMap&&) noexcept = default;
  TokenMap& operator=(const TokenMap&) = default;
  TokenMap& operator=(TokenMap&&) noexcept = default;

 private:
  /// \brief Maps tokens to kinds.
  HashMap<Token::Kind> token_map_;
};
}  // namespace Aq

#endif