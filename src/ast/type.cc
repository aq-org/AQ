// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "ast/type.h"

#include "ast/ast.h"
#include "logging/logging.h"
#include "parser/parser.h"
#include "token/token.h"

namespace Aq {
namespace Ast {

Type* Type::CreateType(Token* token, std::size_t length, std::size_t& index) {
  if (token == nullptr) INTERNAL_ERROR("token is nullptr.");
  if (index >= length) INTERNAL_ERROR("index is out of range.");

  Type* type = nullptr;

  bool is_read_base_type = false;

  while (index < length) {
    if (token[index].type == Token::Type::KEYWORD) {
      is_read_base_type = true;
      type = CreateDerivedType(type, token, length, index);
    } else if (token[index].type == Token::Type::OPERATOR) {
      type = CreateOperatorDerivedType(type, token, length, index);
    } else if (token[index].type == Token::Type::IDENTIFIER) {
      // If the type is not read yet, it means that it is a class type.
      if (!is_read_base_type) {
        is_read_base_type = true;
        type = CreateClassDerivedType(token, length, index);
      } else {
        // Checks if the type is an array.
        type = CreateArrayDerivedType(type, token, length, index);
        return type;
      }
    }
    if (type == nullptr) INTERNAL_ERROR("Type is nullptr.");
    index++;
  }

  LOGGING_ERROR("Index is out of range and the type isn't reach the end.");
  return nullptr;
}

Type* Type::CreateDoubleType() {
  Type* type = new Type();
  type->SetBaseType(Type::BaseType::kDouble);
  return type;
}

uint8_t Type::GetVmType() {
  Type* type = this;
  if (type->GetTypeCategory() == Type::TypeCategory::NONE)
    INTERNAL_ERROR("Unexpected code.");

  if (type->GetTypeCategory() == Type::TypeCategory::kBase) {
    switch (type->GetBaseType()) {
      case Type::BaseType::kAuto:
      case Type::BaseType::kVoid:
        return 0x00;

      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        return 0x01;

      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
      case Type::BaseType::kLong:
        return 0x02;

      case Type::BaseType::kFloat:
      case Type::BaseType::kDouble:
        return 0x03;

        // TODO(uint64_t)

      case Type::BaseType::kString:
        return 0x05;

      case Type::BaseType::kFunction:
      default:
        INTERNAL_ERROR("This type is not currently supported.");
        break;
    }
  } else if (type->GetTypeCategory() == Type::TypeCategory::kArray) {
    return 0x06;
  } else if (type->GetTypeCategory() == Type::TypeCategory::kClass) {
    return 0x09;
  }

  LOGGING_ERROR("Unexpected type category.");
  return 0x00;
}

Type* Type::CreateDerivedType(Type* sub_type, Token* token, std::size_t length,
                              std::size_t& index) {
  Type* type = sub_type;
  switch (token[index].value.keyword) {
    case Token::KeywordType::Const:
      return CreateConstDerivedType(type, token, length, index);
    case Token::KeywordType::Void:
      type = new Type();
      type->SetBaseType(Type::BaseType::kVoid);
      break;

    case Token::KeywordType::Bool:
      type = new Type();
      type->SetBaseType(Type::BaseType::kBool);
      break;

    case Token::KeywordType::Char:
      type = new Type();
      type->SetBaseType(Type::BaseType::kChar);
      break;

    case Token::KeywordType::Short:
      type = new Type();
      type->SetBaseType(Type::BaseType::kShort);
      break;

    case Token::KeywordType::Int:
      type = new Type();
      type->SetBaseType(Type::BaseType::kInt);
      break;

    case Token::KeywordType::Long:
      type = new Type();
      type->SetBaseType(Type::BaseType::kLong);
      break;

    case Token::KeywordType::Float:
      type = new Type();
      type->SetBaseType(Type::BaseType::kFloat);
      break;

    case Token::KeywordType::Double:
      type = new Type();
      type->SetBaseType(Type::BaseType::kDouble);
      break;

    case Token::KeywordType::String:
      type = new Type();
      type->SetBaseType(Type::BaseType::kString);
      break;

    case Token::KeywordType::Var:
    case Token::KeywordType::Auto:
      type = new Type();
      type->SetBaseType(Type::BaseType::kAuto);
      break;

    case Token::KeywordType::Func:
      type = new Type();
      type->SetBaseType(Type::BaseType::kFunction);
      break;

    default:
      return type;
  }
  return type;
}

Type* Type::CreateConstDerivedType(Type* sub_type, Token* token,
                                   std::size_t length, std::size_t& index) {
  if (!(*token == Token::KeywordType::Const))
    INTERNAL_ERROR("Unexpected keyword. Expected 'const' keyword.");

  // If it is not a post const type, it is a pre const type.
  if (sub_type == nullptr) sub_type = CreateType(token, length, ++index);

  if (sub_type == nullptr) INTERNAL_ERROR("type is nullptr.");

  LOGGING_WARNING("The C-style const declaration method is now deprecated.");

  return sub_type;
}

Type* Type::CreateOperatorDerivedType(Type* sub_type, Token* token,
                                      std::size_t length, std::size_t& index) {
  if (sub_type == nullptr) INTERNAL_ERROR("type is nullptr.");
  Type* type = sub_type;
  switch (token[index].value.oper) {
    case Token::OperatorType::amp: {
      // ReferenceType* reference_type = new ReferenceType();
      // reference_type->SetSubType(sub_type);
      // type = reference_type;
      LOGGING_WARNING(
          "The C-style reference declaration method is now deprecated.");
      break;
    }

    case Token::OperatorType::l_square: {
      if (token[index + 1].type != Token::Type::OPERATOR &&
          token[index + 1].value.oper != Token::OperatorType::r_square)
        LOGGING_WARNING("Unsupported array size yet.");

      index++;

      ArrayType* array_type = new ArrayType();
      array_type->SetSubType(type);
      type = array_type;
    }

    default:
      return type;
  }
  return type;
}

Type* Type::CreateClassDerivedType(Token* token, std::size_t length,
                                   std::size_t& index) {
  ClassType* type = new ClassType();

  std::string class_name(token[index].value.identifier.location,
                         token[index].value.identifier.length);

  type->SetSubType(class_name);

  // If the next token is a period, it means that the class is in a
  // namespace or a scope.
  while (index + 1 < length && token[index + 1].type == Token::Type::OPERATOR &&
         token[index + 1].value.oper == Token::OperatorType::period) {
    index += 2;
    if (token[index].type != Token::Type::IDENTIFIER)
      LOGGING_ERROR("Unexpected scope name.");

    std::string next_scope_name(token[index].value.identifier.location,
                                token[index].value.identifier.length);

    type->SetSubType(class_name + "." + next_scope_name);
  }

  return type;
}

Type* Type::CreateArrayDerivedType(Type* sub_type, Token* token,
                                   std::size_t length, std::size_t& index) {
  Type* type = sub_type;
  std::size_t index_temp = index;
  Expression* temp_expr =
      Parser::ParsePrimaryExpression(token, length, index_temp);
  if (temp_expr == nullptr) INTERNAL_ERROR("ParsePrimaryExpr return nullptr.");

  // If the expression is an array, return the array type.
  if (*temp_expr == Statement::StatementType::kArray) {
    // ArrayType* array_type = new ArrayType();
    // array_type->SetSubType(type,
    // dynamic_cast<Array*>(temp_expr)->GetIndexExpression()); type =
    // array_type;
    LOGGING_WARNING("The C-style array declaration method is now deprecated.");
  }

  return type;
}

}  // namespace Ast
}  // namespace Aq