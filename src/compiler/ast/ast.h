// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_AST_AST_H_
#define AQ_COMPILER_AST_AST_H_

#include <map>
#include <string>
#include <vector>

#include "compiler/ast/type.h"
#include "compiler/token/token.h"

namespace Aq {
namespace Compiler {
namespace Ast {
class Statement {
 public:
  Statement() { statement_type_ = StatementType::kStatement; }
  virtual ~Statement() = default;

  enum class StatementType {
    kStatement,
    kImport,
    kBreak,
    kCompound,
    kDeclaration,
    kExpression,
    kFunctionDeclaration,
    kVariable,
    kClass,
    kIf,
    kWhile,
    kDowhile,
    kFor,
    kCase,
    kLabel,
    kGoto,
    kValue,
    kIdentifier,
    kUnary,
    kBinary,
    kConditional,
    kFunction,
    kArrayDeclaration,
    kArray,
    kReturn,
    kStatic
  };

  StatementType GetStatementType() { return statement_type_; }

  Statement(const Statement&) = default;
  Statement& operator=(const Statement&) = default;

 protected:
  StatementType statement_type_;
};

class Import : public Statement {
 public:
  // General import only.
  Import(std::string import_location, std::string name) {
    statement_type_ = StatementType::kImport;
    is_from_import_ = false;
    import_location_ = import_location;
    name_ = name;
  }

  // From import only.
  Import(std::string import_location,
         std::map<std::string, std::string> modules) {
    statement_type_ = StatementType::kImport;
    is_from_import_ = true;
    import_location_ = import_location;
    modules_ = modules;
  }
  virtual ~Import() = default;

  bool IsFromImport() { return is_from_import_; }
  std::string GetImportLocation() { return import_location_; }

  // General import only.
  std::string GetName() { return name_; }

  // From import only.
  std::map<std::string, std::string> GetModule() { return modules_; };

  Import(const Import&) = default;
  Import& operator=(const Import&) = default;

 private:
  bool is_from_import_ = false;
  std::string import_location_;

  // General import only.
  std::string name_;

  // From import only.
  std::map<std::string, std::string> modules_;
};

class Break : public Statement {
 public:
  Break() { statement_type_ = StatementType::kBreak; }
  virtual ~Break() = default;

  Break(const Break&) = default;
  Break& operator=(const Break&) = default;
};

class Compound : public Statement {
 public:
  Compound(std::vector<Statement*> statements) {
    statement_type_ = StatementType::kCompound;
    statements_ = statements;
  }
  virtual ~Compound() = default;

  std::vector<Statement*> GetStatements() { return statements_; }

  Compound(const Compound&) = default;
  Compound& operator=(const Compound&) = default;

 private:
  std::vector<Statement*> statements_;
};

class Expression : public Statement {
 public:
  Expression() { statement_type_ = StatementType::kExpression; }
  virtual ~Expression() = default;

  virtual operator std::string();

  Expression(const Expression&) = default;
  Expression& operator=(const Expression&) = default;
};

class Value : public Expression {
 public:
  Value(Token value) {
    statement_type_ = StatementType::kValue;
    value_ = value;
  }
  virtual ~Value() = default;

  int8_t GetByteValue();
  std::string GetStringValue();
  int64_t GetLongValue();
  double GetDoubleValue();
  uint64_t GetUInt64Value();
  std::size_t GetVmType();
  Type* GetValueType();

  Token GetToken() { return value_; }

  Value(const Value&) = default;
  Value& operator=(const Value&) = default;

 private:
  Token value_;
};

class Unary : public Expression {
 public:
  enum class Operator {
    NONE,
    kPostInc,     // ++
    kPostDec,     // --
    kPreInc,      // ++
    kPreDec,      // --
    kAddrOf,      // &
    kDeref,       // *
    kPlus,        // +
    kMinus,       // -
    kNot,         // !
    kBitwiseNot,  // ~
    ARRAY,        // []
    CONVERT       // ()
  };

  Unary() { statement_type_ = StatementType::kUnary; }

  Unary(Operator oper, Expression* expression) {
    statement_type_ = StatementType::kUnary;
    operator_ = oper;
    expression_ = expression;
  }

  Operator GetOperator() { return operator_; }

  Expression* GetExpression() { return expression_; }

  virtual ~Unary() = default;

  Unary(const Unary&) = default;
  Unary& operator=(const Unary&) = default;

 protected:
  Operator operator_;
  Expression* expression_ = nullptr;
};

class Array : public Unary {
 public:
  Array(Expression* expression, Expression* index) {
    statement_type_ = StatementType::kArray;
    operator_ = Operator::ARRAY;
    expression_ = expression;
    index_ = index;
  }
  virtual ~Array() = default;

  Expression* GetIndexExpression() { return index_; }

  Array(const Array&) = default;
  Array& operator=(const Array&) = default;

 private:
  Expression* index_ = nullptr;
};

class Binary : public Expression {
 public:
  enum class Operator {
    NONE,
    kAdd,        // +
    kSub,        // -
    kMul,        // *
    kDiv,        // /
    kRem,        // %
    kAnd,        // &
    kOr,         // |
    kXor,        // ^
    kShl,        // <<
    kShr,        // >>
    kLT,         // <
    kGT,         // >
    kLE,         // <=
    kGE,         // >=
    kEQ,         // ==
    kNE,         // !=
    kLAnd,       // &&
    kLOr,        // ||
    kAssign,     // =
    kAddAssign,  // +=
    kSubAssign,  // -=
    kMulAssign,  // *=
    kDivAssign,  // /=
    kRemAssign,  // %=
    kAndAssign,  // &=
    kOrAssign,   // |=
    kXorAssign,  // ^=
    kShlAssign,  // <<=
    kShrAssign,  // >>=
    kComma,      // ,
    kArrow,      // ->
    kMember,     // .
    kPtrMemD,    // .*
    kPtrMemI,    // ->*
  };

  Binary(Operator oper, Expression* left, Expression* right) {
    statement_type_ = StatementType::kBinary;
    left_ = nullptr;
    operator_ = Operator::NONE;
    right_ = nullptr;
  }
  void SetBinary(Operator oper, Expression* left, Expression* right) {
    operator_ = oper;
    left_ = left;
    right_ = right;
  }
  virtual ~Binary() = default;

  Operator GetOperator() { return operator_; }

  Expression* GetLeftExpression() { return left_; }
  Expression* GetRightExpression() { return right_; }

  Binary(const Binary&) = default;
  Binary& operator=(const Binary&) = default;

 private:
  Operator operator_;
  Expression* left_ = nullptr;
  Expression* right_ = nullptr;
};

class Conditional : public Expression {
 public:
  Conditional(Expression* condition, Expression* true_expression,
              Expression* false_expression) {
    statement_type_ = StatementType::kConditional;
    condition_ = condition;
    true_expression_ = true_expression;
    false_expression_ = false_expression;
  }
  virtual ~Conditional() = default;

  Expression* GetConditionExpression() { return condition_; }
  Expression* GetTrueExpression() { return true_expression_; }
  Expression* GetFalseExpression() { return false_expression_; }

  Conditional(const Conditional&) = default;
  Conditional& operator=(const Conditional&) = default;

 private:
  Expression* condition_ = nullptr;
  Expression* true_expression_ = nullptr;
  Expression* false_expression_ = nullptr;
};

class Function : public Expression {
 public:
  Function(Expression* function_name, std::vector<Expression*> parameters,
           bool is_variadic) {
    statement_type_ = StatementType::kFunction;
    function_name_ = function_name;
    parameters_ = parameters;
    is_variadic_ = is_variadic;
  }
  virtual ~Function() = default;

  Expression* GetFunctionNameExpression() { return function_name_; }
  std::vector<Expression*> GetParameters() { return parameters_; }

  bool IsVariadic() { return is_variadic_; }

  Function(const Function&) = default;
  Function& operator=(const Function&) = default;

 private:
  Expression* function_name_ = nullptr;
  std::vector<Expression*> parameters_;
  bool is_variadic_ = false;
};

class Identifier : public Expression {
 public:
  Identifier() { statement_type_ = StatementType::kIdentifier; }
  Identifier(Token name) {
    statement_type_ = StatementType::kIdentifier;
    name_ = name;
  }
  virtual ~Identifier() = default;

  Token& GetNameToken() { return name_; }

  operator std::string() override {
    return std::string(name_.value.identifier.location,
                       name_.value.identifier.length);
  }

  Identifier(const Identifier&) = default;
  Identifier& operator=(const Identifier&) = default;

 private:
  Token name_;
};

class Declaration : virtual public Statement {
 public:
  Declaration() { statement_type_ = StatementType::kDeclaration; }
  virtual ~Declaration() = default;

  Declaration(const Declaration&) = default;
  Declaration& operator=(const Declaration&) = default;
};

class Variable : public Declaration, public Expression {
 public:
  Variable() {
    Declaration::statement_type_ = StatementType::kVariable;
    Expression::statement_type_ = StatementType::kVariable;
  }

  Variable(Type* type, Expression* name) {
    Declaration::statement_type_ = StatementType::kVariable;
    Expression::statement_type_ = StatementType::kVariable;
    variable_type_ = type;
    variable_name_ = name;
    variable_value_.push_back(nullptr);
  }
  Variable(Type* type, Expression* name, Expression* value) {
    Declaration::statement_type_ = StatementType::kVariable;
    Expression::statement_type_ = StatementType::kVariable;
    variable_type_ = type;
    variable_name_ = name;
    variable_value_[0] = value;
  }

  virtual ~Variable() = default;

  Variable(const Variable&) = default;
  Variable& operator=(const Variable&) = default;

  Type* GetVariableType() { return variable_type_; }
  Expression* GetVariableNameExpression() { return variable_name_; }
  std::vector<Expression*> GetVariableValue() { return variable_value_; }

 protected:
  Type* variable_type_ = nullptr;
  Expression* variable_name_ = nullptr;
  std::vector<Expression*> variable_value_;
};

class ArrayDeclaration : public Variable {
 public:
  ArrayDeclaration(Type* type, Expression* name, Expression* size) {
    Declaration::statement_type_ = StatementType::kArrayDeclaration;
    variable_type_ = type;
    variable_name_ = name;
    array_size_ = size;
    variable_value_.clear();
  }

  ArrayDeclaration(Type* type, Expression* name, Expression* size,
                   std::vector<Expression*> value) {
    Declaration::statement_type_ = StatementType::kArrayDeclaration;
    variable_type_ = type;
    variable_name_ = name;
    array_size_ = size;
    variable_value_ = value;
  }

  virtual ~ArrayDeclaration() = default;

  Expression* GetArraySizeExpression() { return array_size_; }

  ArrayDeclaration(const ArrayDeclaration&) = default;
  ArrayDeclaration& operator=(const ArrayDeclaration&) = default;

 private:
  Expression* array_size_ = nullptr;
};

class FunctionDeclaration : public Declaration {
 public:
  FunctionDeclaration(Type* type, Function* statement, Compound* body) {
    statement_type_ = StatementType::kFunctionDeclaration;
    return_type_ = type;
    statement_ = statement;
    body_ = body;
  }
  virtual ~FunctionDeclaration() = default;

  Type* GetReturnType() { return return_type_; }
  Function* GetFunctionStatement() { return statement_; }
  Compound* GetFunctionBody() { return body_; }

  FunctionDeclaration(const FunctionDeclaration&) = default;
  FunctionDeclaration& operator=(const FunctionDeclaration&) = default;

 private:
  Type* return_type_ = nullptr;
  Function* statement_ = nullptr;
  Compound* body_ = nullptr;
};

class Static : public Declaration {
 public:
  Static(Declaration* declaration) {
    statement_type_ = StatementType::kStatic;
    declaration_ = declaration;
  }
  ~Static() = default;

  Declaration* GetStaticDeclaration() { return declaration_; }

  Static(const Static&) = default;
  Static& operator=(const Static&) = default;

 private:
  Declaration* declaration_ = nullptr;
};

class Class : public Declaration {
 public:
  Class(Identifier name, std::vector<Static*> static_members,
        std::vector<Variable*> members,
        std::vector<FunctionDeclaration*> methods,
        std::vector<Class*> sub_class) {
    statement_type_ = StatementType::kClass;
    name_ = name;
    static_members_ = static_members;
    members_ = members;
    methods_ = methods;
    sub_class_ = sub_class;
  }
  ~Class() = default;

  Identifier GetClassName() { return name_; }
  std::vector<Static*> GetStaticMembers() { return static_members_; }
  std::vector<Variable*> GetMembers() { return members_; }
  std::vector<FunctionDeclaration*> GetMethods() { return methods_; }
  std::vector<Class*> GetSubClasses() { return sub_class_; }

  Class(const Class&) = default;
  Class& operator=(const Class&) = default;

 private:
  Identifier name_;
  std::vector<Static*> static_members_;
  std::vector<Variable*> members_;
  std::vector<FunctionDeclaration*> methods_;
  std::vector<Class*> sub_class_;
};

class If : public Statement {
 public:
  If(Expression* condition, Statement* body) {
    statement_type_ = StatementType::kIf;
    condition_ = condition;
    body_ = body;
  }
  If(Expression* condition, Statement* body, Statement* else_body) {
    statement_type_ = StatementType::kIf;
    condition_ = condition;
    body_ = body;
    else_body_ = else_body;
  }

  Expression* GetConditionExpression() { return condition_; }
  Statement* GetIfBody() { return body_; }
  Statement* GetElseBody() { return else_body_; }

  If(const If&) = default;
  If& operator=(const If&) = default;

 private:
  Expression* condition_ = nullptr;
  Statement* body_ = nullptr;
  Statement* else_body_ = nullptr;
};

class While : public Statement {
 public:
  While(Expression* condition, Statement* body) {
    statement_type_ = StatementType::kWhile;
    condition_ = condition;
    body_ = body;
  }
  virtual ~While() = default;

  Expression* GetConditionExpression() { return condition_; }
  Statement* GetWhileBody() { return body_; }

  While(const While&) = default;
  While& operator=(const While&) = default;

 private:
  Expression* condition_ = nullptr;
  Statement* body_ = nullptr;
};

class DoWhile : public Statement {
 public:
  DoWhile(Expression* condition, Statement* body) {
    statement_type_ = StatementType::kDowhile;
    condition_ = condition;
    body_ = body;
  }
  virtual ~DoWhile() = default;

  Expression* GetConditionExpression() { return condition_; }
  Statement* GetDoWhileBody() { return body_; }

  DoWhile(const DoWhile&) = default;
  DoWhile& operator=(const DoWhile&) = default;

 private:
  Expression* condition_ = nullptr;
  Statement* body_ = nullptr;
};

class For : public Statement {
 public:
  For(Expression* start, Expression* condition, Expression* end,
      Statement* body) {
    statement_type_ = StatementType::kFor;
    body_ = body;
    start_ = start;
    condition_ = condition;
    end_ = end;
  }
  virtual ~For() = default;

  Expression* GetStartExpression() { return start_; }
  Expression* GetConditionExpression() { return condition_; }
  Expression* GetEndExpression() { return end_; }
  Statement* GetForBody() { return body_; }

  For(const For&) = default;
  For& operator=(const For&) = default;

 private:
  Expression* start_ = nullptr;
  Expression* condition_ = nullptr;
  Expression* end_ = nullptr;
  Statement* body_ = nullptr;
};

class Case : public Statement {
 public:
  Case(Expression* expression, std::vector<Statement*> statements) {
    statement_type_ = StatementType::kCase;
    expression_ = expression;
    statements_ = statements;
  }
  virtual ~Case() = default;

  Expression* GetCaseExpression() { return expression_; }

  std::vector<Statement*> GetStatements() { return statements_; }

 private:
  Expression* expression_ = nullptr;
  std::vector<Statement*> statements_;
};

class Label : public Statement {
 public:
  Label(Identifier label) {
    statement_type_ = StatementType::kLabel;
    label_ = label;
  }
  virtual ~Label() = default;

  Identifier GetLabel() { return label_; }

  Label(const Label&) = default;
  Label& operator=(const Label&) = default;

 private:
  Identifier label_;
};

class Goto : public Statement {
 public:
  Goto(Identifier label) {
    statement_type_ = StatementType::kGoto;
    label_ = label;
  }
  virtual ~Goto() = default;

  Identifier GetLabel() { return label_; }

  Goto(const Goto&) = default;
  Goto& operator=(const Goto&) = default;

 private:
  Identifier label_;
};

class Return : public Statement {
 public:
  Return(Expression* expression) {
    statement_type_ = StatementType::kReturn;
    expression_ = expression;
  }
  virtual ~Return() = default;

  Expression* GetExpression() { return expression_; }

  Return(const Return&) = default;
  Return& operator=(const Return&) = default;

 private:
  Expression* expression_ = nullptr;
};

}  // namespace Ast
}  // namespace Compiler
}  // namespace Aq
#endif
