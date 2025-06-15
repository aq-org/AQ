// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_AST_TYPE_H_
#define AQ_COMPILER_AST_TYPE_H_

#include <string>
#include <vector>
#include <cstdint>

#include "compiler/logging/logging.h"
#include "compiler/token/token.h"


namespace Aq {
namespace Compiler {
namespace Ast {

class Expression;

class Type {
 public:
  enum class TypeCategory { NONE, kBase, kConst, kArray, kReference, kClass };

  enum class BaseType {
    NONE,
    kVoid,
    kBool,
    kChar,
    kShort,
    kInt,
    kLong,
    kFloat,
    kDouble,
    kString,
    kFunction,
    kAuto
  };

  Type() {
    type_category_ = TypeCategory::kBase;
    base_data_ = BaseType::NONE;
    type_data_ = nullptr;
  }

  // Base type only. Sets the base type of the Type object. This is a virtual
  // method that allows derived classes to set their own type.
  void SetBaseType(BaseType type) { base_data_ = type; }
  virtual ~Type() = default;

  Type(const Type&) = default;
  Type& operator=(const Type&) = default;

  TypeCategory GetTypeCategory() { return type_category_; }

  // Base type only. Returns the base type of the Type object.
  BaseType GetBaseType() { return base_data_; }

  // Creates a type based on the token and length. This is a static method that
  // returns a pointer to a Type object. The |index| parameter is used to
  // track the current position in the token array.
  static Type* CreateType(Token* token, std::size_t length, std::size_t& index);

  // Creates a double type. This is a static method that returns a pointer to a
  // Type object. Only applicables for division operations.
  static Type* CreateDoubleType();

  // Returns the vm type of the Type object as a vector of uint8_t.
  std::vector<uint8_t> GetVmType();

  virtual operator std::string() {
    LOGGING_WARNING("Unexpected Type std::string.");
    return "";
  }

 protected:
  // Shows the type category of the Type object. This is a member variable
  // that indicates the category of the type, such as base type, const type,
  // etc.
  TypeCategory type_category_ = TypeCategory::NONE;

  // Derived type only. This is a pointer to another Type object that
  // represents the sub type of the Type object.
  Type* type_data_ = nullptr;

 private:
  BaseType base_data_ = BaseType::NONE;

  // Creates a base type based on the token and length. This is a static method
  // that returns a pointer to a Type object. The |index| parameter is used to
  // track the current position in the token array.
  Type* CreateBaseType(Token* token, std::size_t length, std::size_t& index);

  // Creates a class type based on the token and length. This is a static method
  // that returns a pointer to a Type object. The |index| parameter is used to
  // track the current position in the token array.
  static Type* CreateDerivedType(Type* sub_type, Token* token,
                                 std::size_t length, std::size_t& index);

  // Creates a const derived type based on the token and length. This is a
  // static method that returns a pointer to a Type object. The |index|
  // parameter is used to track the current position in the token array.
  static Type* CreateConstDerivedType(Type* sub_type, Token* token,
                                      std::size_t length, std::size_t& index);

  // Creates a pointer or reference derived type based on the token and length
  // if the operator is a part of the type. This is a static method that returns
  // a pointer to a Type object. The |index| parameter is used to track the
  // current position in the token array.
  static Type* CreateOperatorDerivedType(Type* sub_type, Token* token,
                                         std::size_t length,
                                         std::size_t& index);

  // Creates a class type based on the token and length. This is a static method
  // that returns a pointer to a Type object. The |index| parameter is used to
  // track the current position in the token array.
  static Type* CreateClassDerivedType(Token* token, std::size_t length,
                                      std::size_t& index);

  // Creates an array derived type based on the sub type, token, and length if
  // the array exists. This is a static method that returns a pointer to a Type
  // object. The |index| parameter is used to track the current position in the
  // token array.
  static Type* CreateArrayDerivedType(Type* sub_type, Token* token,
                                      std::size_t length, std::size_t& index);
};

class ConstType : public Type {
 public:
  ConstType() { type_category_ = TypeCategory::kConst; }
  void SetSubType(Type* type) { type_data_ = type; }
  virtual ~ConstType() = default;

  Type* GetSubType() { return type_data_; }

  operator std::string() override {
    // Regardless of whether the code is written as const before or after, the
    // standard of const before is adopted.
    return std::string("const ") + std::string(*this->GetSubType());
  }

  ConstType(const ConstType&) = default;
  ConstType& operator=(const ConstType&) = default;
};

class ArrayType : public Type {
 public:
  ArrayType() {
    type_category_ = TypeCategory::kArray;
    size_ = nullptr;
  }
  void SetSubType(Type* type, Expression* size) {
    type_data_ = type;
    size_ = size;
  }
  virtual ~ArrayType() = default;

  Type* GetSubType() { return type_data_; }

  Expression* GetArraySize() { return size_; }

  operator std::string() override { return std::string(*type_data_) + "*"; }

  ArrayType(const ArrayType&) = default;
  ArrayType& operator=(const ArrayType&) = default;

 private:
  Expression* size_;
};

class ReferenceType : public Type {
 public:
  ReferenceType() { type_category_ = TypeCategory::kReference; }
  void SetSubType(Type* type) { type_data_ = type; }
  virtual ~ReferenceType() = default;

  Type* GetSubType() { return type_data_; }

  operator std::string() override { return std::string(*type_data_) + "&"; }

  ReferenceType(const ReferenceType&) = default;
  ReferenceType& operator=(const ReferenceType&) = default;
};

class ClassType : public Type {
 public:
  ClassType() { type_category_ = TypeCategory::kClass; }
  void SetSubType(std::string class_name) { class_name_ = class_name; }
  virtual ~ClassType() = default;

  std::string GetClassName() { return class_name_; }

  std::vector<std::string>& GetNames() { return names_; }

  operator std::string() override { return class_name_; }

  ClassType(const ClassType&) = default;
  ClassType& operator=(const ClassType&) = default;

 private:
  std::string class_name_;
  std::vector<std::string> names_;
};

}  // namespace Ast
}  // namespace Compiler
}  // namespace Aq

#endif