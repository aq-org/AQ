// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/generator/generator.h"

#include <cstdint>
#include <fstream>

#include "compiler/ast/ast.h"
#include "compiler/generator/declaration_generator.h"
#include "compiler/generator/goto_generator.h"
#include "compiler/generator/preprocesser.h"
#include "compiler/generator/statement_generator.h"
#include "compiler/logging/logging.h"

namespace Aq {
namespace Compiler {
namespace Generator {
void Generator::Generate(Ast::Compound* statement, const char* output_file) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Adds the function context for the main function.
  auto function_context = new FunctionContext();
  context.function_context = function_context;

  global_memory.SetCode(&init_code);

  // Main program return value and its reference.
  global_memory.Add(1);
  global_memory.Add(1);

  // Bytecode Running class.
  std::vector<uint8_t> bytecodeclass_vm_type{0x09};
  global_memory.AddWithType(bytecodeclass_vm_type);
  global_code.push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, 0, global_memory.AddString("(void)")));
  global_code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, 1, 0));

  // Sets the scope information.
  context.scopes.push_back("");
  context.function_context->current_scope = context.scopes.size() - 1;

  // Sets the initialize function.
  std::size_t memory_init_name = global_memory.GetMemorySize();
  std::size_t memory_init_name_const = global_memory.GetConstTableSize();
  global_memory.AddString(".!__init");
  global_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_init_name,
                                 memory_init_name_const));
  global_code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, 2,
                                 memory_init_name, 1, global_memory.Add(1)));

  // Sets the current class.
  Class* start_class = &this->main_class;
  context.current_class = start_class;
  start_class->SetName(".__start");
  start_class->GetMemory().Add("@name");
  start_class->GetMemory().Add("@size");

  // Preprocesses the declaration statements.
  PreProcessDeclaration(*this, statement);

  std::vector<Ast::Statement*> statements;
  std::vector<Ast::Import*> imports;

  // Initialize the statements that need to be preprocessed for the parts that
  // have not been preprocessed in the preprocessor.
  for (std::size_t i = 0; i < statement->GetStatements().size(); i++) {
    Ast::Statement* sub_statement = statement->GetStatements()[i];
    switch (sub_statement->GetStatementType()) {
      case Ast::Statement::StatementType::kFunctionDeclaration:
        HandleFunctionDeclaration(
            *this, Ast::Cast<Ast::FunctionDeclaration>(sub_statement));
        break;

      case Ast::Statement::StatementType::kVariable:
        context.variables[context.scopes.back() + "#" +
                          Ast::Cast<Ast::Variable>(sub_statement)
                              ->GetVariableName()] =
            HandleGlobalVariableDeclaration(
                *this, Ast::Cast<Ast::Variable>(sub_statement), init_code);
        break;

      case Ast::Statement::StatementType::kArrayDeclaration:
        context.variables[context.scopes.back() + "#" +
                          Ast::Cast<Ast::ArrayDeclaration>(sub_statement)
                              ->GetVariableName()] =
            HandleGlobalArrayDeclaration(
                *this, Ast::Cast<Ast::ArrayDeclaration>(sub_statement),
                init_code);
        break;

      case Ast::Statement::StatementType::kGoto:
        statements.push_back(sub_statement);
        break;

      case Ast::Statement::StatementType::kImport:
        imports.push_back(Ast::Cast<Ast::Import>(sub_statement));
        break;

      default:
        statements.push_back(sub_statement);
    }
  }

  // Restores function context null caused by possible function generation.
  context.function_context = function_context;
  context.current_class = start_class;

  for (std::size_t i = 0; i < imports.size(); i++) {
    HandleImport(*this, imports[i]);
  }

  for (std::size_t i = 0; i < statements.size(); i++) {
    switch (statements[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kGoto:
        HandleGoto(*this, Ast::Cast<Ast::Goto>(statements[i]), global_code);
        break;

      default:
        HandleStatement(*this, statements[i], global_code);
    }
  }

  // Restores function context null caused by possible function generation.
  context.function_context = function_context;
  context.current_class = start_class;

  while (context.function_context->goto_map.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = context.scopes.size() - 1;
         i >= context.function_context->current_scope; i--) {
      // Searches for the label in the current scope.
      auto iterator = context.function_context->label_map.find(
          context.scopes[i] + "$" +
          context.function_context->goto_map.back().first);

      // If the label is found in the current scope, it will be replaced with
      // the address of the label.
      if (iterator != context.function_context->label_map.end()) {
        goto_location = iterator->second;
        break;
      }

      if (i == 0) LOGGING_ERROR("Label not found.");
    }

    global_code[context.function_context->goto_map.back().second].SetArgs(
        1, global_memory.AddUint64t(goto_location));
    context.function_context->goto_map.pop_back();
  }

  context.scopes.clear();
  context.function_context->current_scope = 0;

  // Generates the bytecode for the main function.
  std::vector<std::size_t> constructor_args;
  std::vector<Bytecode> start_code;
  constructor_args.push_back(global_memory.Add(1));
  std::size_t start_function_name = global_memory.Add(1);

  // Handles the function name for the start function.
  std::string name_str = ".!__start";
  global_memory.GetConstTable().push_back(0x05);
  EncodeUleb128(name_str.size() + 1, global_memory.GetConstTable());
  for (std::size_t i = 0; i < name_str.size(); i++) {
    global_memory.GetConstTable().push_back(name_str[i]);
  }
  global_memory.GetConstTable().push_back(0x00);
  global_memory.GetConstTableSize()++;
  std::size_t name_const_index = global_memory.GetConstTableSize() - 1;

  // Adds the start function name into the global memory.
  start_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2,
                                start_function_name, name_const_index));

  // Adds the start function name into the constructor arguments. And makes the
  // invoke for the start function.
  std::vector<std::size_t> invoke_start_arguments = {2, start_function_name, 1,
                                                     1};
  start_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_start_arguments));

  // Makes the constructor function for the start function.
  Function constructor_func("@constructor", constructor_args, start_code);
  functions.push_back(constructor_func);

  // Adds the start function into the global memory.
  std::vector<std::size_t> arguments;
  arguments.push_back(1);

  // Adds the main function invoke into the global code.
  std::size_t main_func = global_memory.AddString(".main");
  std::vector<std::size_t> invoke_main_arguments = {2, main_func, 1, 1};
  global_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_main_arguments));
  Function start_func(".!__start", arguments, global_code);
  functions.push_back(start_func);

  // Checks if the break statement is used outside of loops or switches.
  if (context.function_context->loop_break_index.size() != 0)
    LOGGING_ERROR("Break cannot be used outside of loops and switches.");

  // Generates the bytecode for the initialization function.
  std::vector<std::size_t> memory_init_args;
  memory_init_args.push_back(global_memory.Add(1));
  Function memory_init_func(".!__init", memory_init_args, init_code);
  functions.push_back(memory_init_func);

  GenerateBytecodeFile(*this, output_file);
}

void GenerateBytecodeFile(Generator& generator, const char* output_file) {
  if (output_file == nullptr) INTERNAL_ERROR("output_file is nullptr.");

  // Gets the reference of context.
  auto& global_memory = generator.global_memory;
  auto& is_big_endian = generator.is_big_endian;
  auto& classes = generator.classes;
  auto& start_class = generator.context.current_class;
  auto& functions = generator.functions;

  std::vector<uint8_t> code;

  code.push_back(0x41);
  code.push_back(0x51);
  code.push_back(0x42);
  code.push_back(0x43);

  // Version
  code.push_back(0x00);
  code.push_back(0x00);
  code.push_back(0x00);
  code.push_back(0x03);

  // Gets the constant pool size.
  std::vector<uint8_t> consant_pool_size;
  EncodeUleb128(global_memory.GetConstTableSize(), consant_pool_size);
  code.insert(code.end(), consant_pool_size.begin(), consant_pool_size.end());

  // Writes the constant pool data.
  code.insert(code.end(), global_memory.GetConstTable().begin(),
              global_memory.GetConstTable().end());

  // Gets the bytecode size.
  std::vector<uint8_t> memory_size;
  EncodeUleb128(global_memory.GetMemorySize(), memory_size);
  code.insert(code.end(), memory_size.begin(), memory_size.end());

  // Writes the memory data.
  code.insert(code.end(), global_memory.GetMemoryType().begin(),
              global_memory.GetMemoryType().end());

  for (std::size_t i = 0; i < classes.size(); i++) {
    // Writes the class name.
    std::string class_name_str = classes[i].GetName();
    const char* class_name = class_name_str.c_str();
    code.insert(code.end(), reinterpret_cast<const uint8_t*>(class_name),
                reinterpret_cast<const uint8_t*>(class_name +
                                                 class_name_str.size() + 1));

    // Writes the class memory information.
    std::vector<uint8_t> class_memory_size;
    EncodeUleb128(classes[i].GetMemory().GetMemorySize(), class_memory_size);
    code.insert(code.end(), class_memory_size.begin(), class_memory_size.end());

    // Writes the class memory data.
    code.insert(code.end(), classes[i].GetMemory().GetMemoryInfo().begin(),
                classes[i].GetMemory().GetMemoryInfo().end());

    // Writes the class methods size.
    std::vector<uint8_t> methods_size;
    EncodeUleb128(classes[i].GetFunctionList().size(), methods_size);
    code.insert(code.end(), methods_size.begin(), methods_size.end());

    std::vector<Function> func_list = classes[i].GetFunctionList();

    for (std::size_t z = 0; z < func_list.size(); z++) {
      std::string func_name_str = func_list[z].GetName();
      const char* func_name = func_name_str.c_str();
      code.insert(code.end(), reinterpret_cast<const uint8_t*>(func_name),
                  reinterpret_cast<const uint8_t*>(
                      func_name + func_list[z].GetName().size() + 1));

      if (func_list[z].IsVariadic()) code.push_back(0xFF);

      std::vector<uint8_t> args_buffer;

      // Writes the function parameters size and its parameters.
      EncodeUleb128(func_list[z].GetParameters().size(), args_buffer);
      code.insert(code.end(), args_buffer.begin(), args_buffer.end());
      for (std::size_t j = 0; j < func_list[z].GetParameters().size(); j++) {
        args_buffer.clear();
        EncodeUleb128(func_list[z].GetParameters()[j], args_buffer);
        code.insert(code.end(), args_buffer.begin(), args_buffer.end());
      }

      // Writes the instructions size.
      std::vector<uint8_t> instructions_size;
      EncodeUleb128(func_list[z].GetCode().size(), instructions_size);
      code.insert(code.end(), instructions_size.begin(),
                  instructions_size.end());

      for (std::size_t j = 0; j < func_list[z].GetCode().size(); j++) {
        std::vector<uint8_t> buffer;
        switch (func_list[z].GetCode()[j].GetOper()) {
          case _AQVM_OPERATOR_NOP:
            code.push_back(_AQVM_OPERATOR_NOP);
            break;

          case _AQVM_OPERATOR_LOAD:
            code.push_back(_AQVM_OPERATOR_LOAD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected LOAD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_STORE:
            code.push_back(_AQVM_OPERATOR_STORE);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected STORE args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEW:
            code.push_back(_AQVM_OPERATOR_NEW);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected NEW args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ARRAY:
            code.push_back(_AQVM_OPERATOR_ARRAY);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected ARRAY args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_PTR:
            code.push_back(_AQVM_OPERATOR_PTR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected PTR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ADD:
            code.push_back(_AQVM_OPERATOR_ADD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected ADD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SUB:
            code.push_back(_AQVM_OPERATOR_SUB);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected SUB args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_MUL:
            code.push_back(_AQVM_OPERATOR_MUL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected MUL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_DIV:
            code.push_back(_AQVM_OPERATOR_DIV);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected DIV args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REM:
            code.push_back(_AQVM_OPERATOR_REM);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected REM args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEG:
            code.push_back(_AQVM_OPERATOR_NEG);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected NEG args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHL:
            code.push_back(_AQVM_OPERATOR_SHL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected SHL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHR:
            code.push_back(_AQVM_OPERATOR_SHR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected SHR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REFER:
            code.push_back(_AQVM_OPERATOR_REFER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected REFER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            break;

          case _AQVM_OPERATOR_IF:
            code.push_back(_AQVM_OPERATOR_IF);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected IF args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_AND:
            code.push_back(_AQVM_OPERATOR_AND);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected AND args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_OR:
            code.push_back(_AQVM_OPERATOR_OR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected OR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_XOR:
            code.push_back(_AQVM_OPERATOR_XOR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected XOR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CMP:
            code.push_back(_AQVM_OPERATOR_CMP);

            if (func_list[z].GetCode()[j].GetArgs().size() != 4)
              INTERNAL_ERROR("Unexpected CMP args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[3], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE:
            code.push_back(_AQVM_OPERATOR_INVOKE);

            if (func_list[z].GetCode()[j].GetArgs().size() < 2)
              INTERNAL_ERROR("Unexpected INVOKE args size.");

            for (std::size_t k = 0;
                 k != func_list[z].GetCode()[j].GetArgs()[1] + 2; k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code.insert(code.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_EQUAL:
            code.push_back(_AQVM_OPERATOR_EQUAL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected EQUAL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_GOTO:
            code.push_back(_AQVM_OPERATOR_GOTO);

            if (func_list[z].GetCode()[j].GetArgs().size() != 1)
              INTERNAL_ERROR("Unexpected GOTO args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_LOAD_CONST:
            code.push_back(_AQVM_OPERATOR_LOAD_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected LOAD_CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONVERT:
            code.push_back(_AQVM_OPERATOR_CONVERT);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected CONVERT args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONST:
            code.push_back(_AQVM_OPERATOR_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              INTERNAL_ERROR("Unexpected CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE_METHOD:
            code.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

            if (func_list[z].GetCode()[j].GetArgs().size() < 3)
              INTERNAL_ERROR("Unexpected INVOKE_METHOD args size.");

            for (std::size_t k = 0;
                 k < func_list[z].GetCode()[j].GetArgs().size(); k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code.insert(code.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_LOAD_MEMBER:
            code.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              INTERNAL_ERROR("Unexpected LOAD_MEMBER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_WIDE:
            code.push_back(_AQVM_OPERATOR_WIDE);
            break;

          default:
            break;
        }
      }
    }
  }

  std::string class_name_str = ".!__start";

  const char* class_name = class_name_str.c_str();
  code.insert(
      code.end(), reinterpret_cast<const uint8_t*>(class_name),
      reinterpret_cast<const uint8_t*>(class_name + class_name_str.size() + 1));

  // Writes the start class memory information.
  std::vector<uint8_t> start_memory_size;
  EncodeUleb128(start_class->GetMemory().GetMemorySize(), start_memory_size);
  code.insert(code.end(), start_memory_size.begin(), start_memory_size.end());

  code.insert(code.end(), start_class->GetMemory().GetMemoryInfo().begin(),
              start_class->GetMemory().GetMemoryInfo().end());

  // Writes the start functions information.
  std::vector<uint8_t> functions_size;
  EncodeUleb128(functions.size(), functions_size);
  code.insert(code.end(), functions_size.begin(), functions_size.end());

  std::vector<Function> func_list = functions;

  for (std::size_t i = 0; i < func_list.size(); i++) {
    std::string func_name_str = func_list[i].GetName();
    const char* func_name = func_name_str.c_str();
    code.insert(code.end(), reinterpret_cast<const uint8_t*>(func_name),
                reinterpret_cast<const uint8_t*>(
                    func_name + func_list[i].GetName().size() + 1));

    if (func_list[i].IsVariadic()) code.push_back(0xFF);

    std::vector<uint8_t> args_buffer;

    // Writes the function parameters size and its parameters.
    EncodeUleb128(func_list[i].GetParameters().size(), args_buffer);
    code.insert(code.end(), args_buffer.begin(), args_buffer.end());
    for (std::size_t j = 0; j < func_list[i].GetParameters().size(); j++) {
      args_buffer.clear();
      EncodeUleb128(func_list[i].GetParameters()[j], args_buffer);
      code.insert(code.end(), args_buffer.begin(), args_buffer.end());
    }

    // Writes the instructions size.
    std::vector<uint8_t> instructions_size;
    EncodeUleb128(func_list[i].GetCode().size(), instructions_size);
    code.insert(code.end(), instructions_size.begin(), instructions_size.end());

    for (std::size_t j = 0; j < func_list[i].GetCode().size(); j++) {
      std::vector<uint8_t> buffer;
      switch (func_list[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          code.push_back(_AQVM_OPERATOR_NOP);
          break;

        case _AQVM_OPERATOR_LOAD:
          code.push_back(_AQVM_OPERATOR_LOAD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected LOAD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_STORE:
          code.push_back(_AQVM_OPERATOR_STORE);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected STORE args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEW:
          code.push_back(_AQVM_OPERATOR_NEW);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected NEW args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ARRAY:
          code.push_back(_AQVM_OPERATOR_ARRAY);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected ARRAY args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_PTR:
          code.push_back(_AQVM_OPERATOR_PTR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected PTR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ADD:
          code.push_back(_AQVM_OPERATOR_ADD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected ADD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SUB:
          code.push_back(_AQVM_OPERATOR_SUB);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected SUB args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_MUL:
          code.push_back(_AQVM_OPERATOR_MUL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected MUL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_DIV:
          code.push_back(_AQVM_OPERATOR_DIV);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected DIV args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REM:
          code.push_back(_AQVM_OPERATOR_REM);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected REM args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEG:
          code.push_back(_AQVM_OPERATOR_NEG);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected NEG args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHL:
          code.push_back(_AQVM_OPERATOR_SHL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected SHL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHR:
          code.push_back(_AQVM_OPERATOR_SHR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected SHR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REFER:
          code.push_back(_AQVM_OPERATOR_REFER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected REFER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          break;

        case _AQVM_OPERATOR_IF:
          code.push_back(_AQVM_OPERATOR_IF);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected IF args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_AND:
          code.push_back(_AQVM_OPERATOR_AND);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected AND args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_OR:
          code.push_back(_AQVM_OPERATOR_OR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected OR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_XOR:
          code.push_back(_AQVM_OPERATOR_XOR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected XOR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CMP:
          code.push_back(_AQVM_OPERATOR_CMP);

          if (func_list[i].GetCode()[j].GetArgs().size() != 4)
            INTERNAL_ERROR("Unexpected CMP args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[3], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE:
          code.push_back(_AQVM_OPERATOR_INVOKE);

          if (func_list[i].GetCode()[j].GetArgs().size() < 2)
            INTERNAL_ERROR("Unexpected INVOKE args size.");

          for (std::size_t k = 0;
               k != func_list[i].GetCode()[j].GetArgs()[1] + 2; k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_EQUAL:
          code.push_back(_AQVM_OPERATOR_EQUAL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected EQUAL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_GOTO:
          code.push_back(_AQVM_OPERATOR_GOTO);

          if (func_list[i].GetCode()[j].GetArgs().size() != 1)
            INTERNAL_ERROR("Unexpected GOTO args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_LOAD_CONST:
          code.push_back(_AQVM_OPERATOR_LOAD_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected LOAD_CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONVERT:
          code.push_back(_AQVM_OPERATOR_CONVERT);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected CONVERT args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONST:
          code.push_back(_AQVM_OPERATOR_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            INTERNAL_ERROR("Unexpected CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          code.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

          if (func_list[i].GetCode()[j].GetArgs().size() < 3)
            INTERNAL_ERROR("Unexpected INVOKE_METHOD args size.");

          for (std::size_t k = 0;
               k < func_list[i].GetCode()[j].GetArgs().size(); k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code.insert(code.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          code.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            INTERNAL_ERROR("Unexpected LOAD_MEMBER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code.insert(code.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_WIDE:
          code.push_back(_AQVM_OPERATOR_WIDE);
          break;

        default:
          break;
      }
    }
  }

  std::string filename(output_file);
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile) {
    INTERNAL_ERROR("Can't open file.");
    return;
  }

  outFile.write(reinterpret_cast<const char*>(code.data()), code.size());
  outFile.close();

  if (!outFile) {
    INTERNAL_ERROR("Failed to write file.");
  } else {
    std::cout << "[INFO] " << "Write file success: " << filename << std::endl;
  }

  bool is_output_mnemonic = true;
  if (is_output_mnemonic) {
    // GenerateMnemonicFile(output_file);
  }
}

void InsertUint64ToCode(uint64_t value, std::vector<uint8_t>& code) {
  for (int i = 0; i < 8; ++i) {
    code.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }
}

}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq