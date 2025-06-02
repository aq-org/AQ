// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/ast/ast.h"

#include <string>

#include "compiler/logging/logging.h"
#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {
namespace Ast {
int8_t Value::GetByteValue() {
  if (value_.type == Token::Type::KEYWORD) {
    if (value_.value.keyword == Token::KeywordType::True) {
      return true;
    } else if (value_.value.keyword == Token::KeywordType::False) {
      return false;
    } else {
      LOGGING_ERROR(
          "Unexpected keyword type: " +
              std::to_string(static_cast<int>(value_.value.keyword)));
    }
  }
  return value_.value.character;
}
std::string Value::GetStringValue() { return *value_.value.string; }

int64_t Value::GetLongValue() {
  return std::stoll(
      std::string(value_.value.number.location, value_.value.number.length));
}

double Value::GetDoubleValue() {
  return std::stod(
      std::string(value_.value.number.location, value_.value.number.length));
}
uint64_t Value::GetUInt64Value() {
  return std::stoull(
      std::string(value_.value.number.location, value_.value.number.length));
}

std::size_t Value::GetVmType() {
  if (value_.type == Token::Type::KEYWORD) {
    if (value_.value.keyword == Token::KeywordType::True ||
        value_.value.keyword == Token::KeywordType::False) {
      return 0x01;
    } else {
      LOGGING_ERROR(
          "Unexpected keyword type: " +
              std::to_string(static_cast<int>(value_.value.keyword)));
    }
  }
  if (value_.type == Token::Type::CHARACTER) {
    return 0x01;
  }
  if (value_.type == Token::Type::STRING) {
    return 0x05;
  }

  std::string str(value_.value.number.location, value_.value.number.length);

  try {
    std::size_t pos;
    (void)std::stoll(str, &pos);
    if (pos == str.size()) {
      return 0x02;
    }
  } catch (...) {
  }

  try {
    std::size_t pos;
    (void)std::stoull(str, &pos);
    if (pos == str.size()) {
      return 0x04;
    }
  } catch (...) {
  }

  try {
    std::size_t pos;
    (void)std::stod(str, &pos);
    if (pos == str.size()) {
      return 0x03;
    }
  } catch (...) {
  }

  LOGGING_ERROR( "Unexpected value type.");
  return 0x00;
}
Expression::operator std::string() {
  // TODO: Implement a proper conversion for Expression.
  if (statement_type_ == StatementType::kIdentifier)
    return *dynamic_cast<Identifier*>(this);
  LOGGING_WARNING(
      "Expression does not have a valid string representation. Returning empty "
      "string.");
  return std::string();
}

}  // namespace Ast
}  // namespace Compiler
}  // namespace Aq