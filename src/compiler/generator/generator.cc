// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/generator/generator.h"

#include "compiler/ast/ast.h"
#include "compiler/logging/logging.h"

namespace Aq {
namespace Compiler {
namespace Generator {
std::unordered_map<std::string, Generator*> import_generator_map;

void Generator::GenerateBytecode(Ast::Compound* stmt, const char* output_file) {
  global_memory_.SetCode(&init_code_);

  // Main program return value.
  global_memory_.Add(1);
  global_memory_.Add(1);

  // Bytecode Running class.
  std::vector<uint8_t> bytecode_class_vm_type;
  bytecode_class_vm_type.push_back(0x09);
  global_memory_.AddWithType(bytecode_class_vm_type);

  global_code_.push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, 0, global_memory_.AddString("(void)")));
  // std::size_t return_value_ptr = global_memory_.Add(1);
  // global_code_.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, 0,
  // return_value_ptr));
  global_code_.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, 1, 0));
  // current_scope_.push_back("global");
  current_scope_.push_back("");
  // single_scope_.push_back("global");
  current_func_index_ = current_scope_.size() - 1;
  if (stmt == nullptr)
    EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*,const char*)",
                  "stmt is nullptr.");

  std::size_t memory_init_name = global_memory_.GetMemorySize();
  std::size_t memory_init_name_const = global_memory_.GetConstTableSize();

  global_memory_.AddString(".!__init");

  global_code_.push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2,
                                  memory_init_name, memory_init_name_const));
  global_code_.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, 2,
                                  memory_init_name, 1, global_memory_.Add(1)));

  start_class_.SetName(".__start");
  start_class_.GetMemory().Add("@name");
  start_class_.GetMemory().Add("@size");

  PreProcessDecl(stmt);

  std::vector<Ast::Statement*> start_stmts;
  std::vector<Ast::Import*> import_stmts;

  for (std::size_t i = 0; i < stmt->GetStatements().size(); i++) {
    switch (stmt->GetStatements()[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kFunctionDeclaration:
        HandleFuncDecl(dynamic_cast<Ast::Function*>(stmt->GetStatements()[i]));
        break;

      case Ast::Statement::StatementType::kVariable:
        var_decl_map_.emplace(
            current_scope_.back() + "#" +
                static_cast<std::string>(
                    *dynamic_cast<VarDeclNode*>(stmt->GetStatements()[i])
                         ->GetName()),
            std::pair<VarDeclNode*, std::size_t>(
                dynamic_cast<VarDeclNode*>(stmt->GetStatements()[i]),
                HandleStartVarDecl(
                    dynamic_cast<VarDeclNode*>(stmt->GetStatements()[i]),
                    global_code_)));
        // HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt->GetStatements()[i]),
        //               global_code_);
        break;

      case Ast::Statement::StatementType::kArrayDeclaration:
        var_decl_map_.emplace(
            current_scope_.back() + "#" +
                static_cast<std::string>(
                    *dynamic_cast<ArrayDeclNode*>(stmt->GetStatements()[i])
                         ->GetName()),
            std::pair<VarDeclNode*, std::size_t>(
                dynamic_cast<ArrayDeclNode*>(stmt->GetStatements()[i]),
                HandleStartVarDecl(
                    dynamic_cast<ArrayDeclNode*>(stmt->GetStatements()[i]),
                    global_code_)));
        // HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt->GetStatements()[i]),
        //                 global_code_);
        break;

      case Ast::Statement::StatementType::kGoto:
        start_stmts.push_back(stmt->GetStatements()[i]);
        // HandleStartGoto(dynamic_cast<GotoNode*>(stmt->GetStatements()[i]),
        //                 global_code_);
        break;

      case Ast::Statement::StatementType::kImport:
        import_stmts.push_back(
            dynamic_cast<Ast::Import*>(stmt->GetStatements()[i]));
        break;

      default:
        start_stmts.push_back(stmt->GetStatements()[i]);
        // HandleStmt(stmt->GetStatements()[i], global_code_);
        /*EXIT_COMPILER(
            "Generator::GenerateBytecode(Ast::Compound*,const char*)",
            "Unexpected code.");*/
    }
  }

  for (std::size_t i = 0; i < import_stmts.size(); i++) {
    HandleImport(import_stmts[i]);
  }

  for (std::size_t i = 0; i < start_stmts.size(); i++) {
    switch (stmt->GetStatements()[i]->GetStatementType()) {
      case Ast::Statement::StatementType::kGoto:
        HandleStartGoto(dynamic_cast<GotoNode*>(start_stmts[i]), global_code_);
        break;

      default:
        HandleStmt(start_stmts[i], global_code_);
        /*EXIT_COMPILER(
            "Generator::GenerateBytecode(Ast::Compound*,const char*)",
            "Unexpected code.");*/
    }
  }

  while (start_goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator = label_map_.find(current_scope_[i] + "$" +
                                      start_goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == 0)
        EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*,const char*)",
                      "Label not found.");
    }
    // std::cout << global_code_[start_goto_map_.back().second].GetOper() <<
    // std::endl;
    global_code_[start_goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    start_goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;

  std::vector<std::size_t> constructor_args;
  std::vector<Bytecode> start_code;
  constructor_args.push_back(global_memory_.Add(1));
  std::size_t start_func_name = global_memory_.Add(1);
  std::string name_str = ".!__start";
  global_memory_.GetConstTable().push_back(0x05);
  EncodeUleb128(name_str.size() + 1, global_memory_.GetConstTable());
  for (std::size_t i = 0; i < name_str.size(); i++) {
    global_memory_.GetConstTable().push_back(name_str[i]);
  }
  global_memory_.GetConstTable().push_back(0x00);
  global_memory_.GetConstTableSize()++;
  std::size_t name_const_index = global_memory_.GetConstTableSize() - 1;
  start_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, start_func_name,
                                name_const_index));
  std::vector<std::size_t> invoke_start_args = {2, start_func_name, 1, 1};
  start_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_start_args));
  Function constructor_func("@constructor", constructor_args, start_code);
  func_list_.push_back(constructor_func);

  std::vector<std::size_t> args;
  args.push_back(1);
  // std::vector<Bytecode> start_code;
  // std::size_t main_func = global_memory_.AddString("global.main");
  std::size_t main_func = global_memory_.AddString(".main");
  /*for (size_t i = 0; i < global_memory_.GetCode().size(); i++) {
    start_code.push_back(global_memory_.GetCode()[i]);
  }
  for (size_t i = 0; i < global_code_.size(); i++) {
    start_code.push_back(global_code_[i]);
  }*/
  /*start_code.insert(start_code.end(), global_memory_.GetCode().begin(),
                    global_memory_.GetCode().end());*/
  // start_code.insert(start_code.end(), global_code_.begin(),
  // global_code_.end());
  std::vector<std::size_t> invoke_main_args = {2, main_func, 1, 1};
  // start_code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_main_args));
  global_code_.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_main_args));
  // Function start_func("__start", args, start_code);
  Function start_func(".!__start", args, global_code_);
  func_list_.push_back(start_func);

  if (loop_break_index_.size() != 0)
    EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*,const char*)",
                  "Break cannot be used outside of loops and switches.");

  std::vector<std::size_t> memory_init_args;
  memory_init_args.push_back(global_memory_.Add(1));
  Function memory_init_func(".!__init", memory_init_args, init_code_);
  func_list_.push_back(memory_init_func);

  GenerateBytecodeFile(output_file);
}

void Generator::GenerateBytecodeFile(const char* output_file) {
  code_.push_back(0x41);
  code_.push_back(0x51);
  code_.push_back(0x42);
  code_.push_back(0x43);

  // Version
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x03);

  InsertUint64ToCode(is_big_endian_
                         ? global_memory_.GetConstTableSize()
                         : SwapUint64t(global_memory_.GetConstTableSize()));
  for (std::size_t i = 0; i < global_memory_.GetConstTable().size(); i++) {
    code_.push_back(global_memory_.GetConstTable()[i]);
  }
  std::size_t memory_size = global_memory_.GetMemorySize();
  InsertUint64ToCode(is_big_endian_ ? memory_size : SwapUint64t(memory_size));
  for (std::size_t i = 0; i < global_memory_.GetMemoryType().size(); i++) {
    code_.push_back(global_memory_.GetMemoryType()[i]);
  }

  for (std::size_t i = 0; i < class_list_.size(); i++) {
    std::string class_name_str = class_list_[i].GetName();

    const char* class_name = class_name_str.c_str();
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(class_name),
                 reinterpret_cast<const uint8_t*>(class_name +
                                                  class_name_str.size() + 1));

    std::size_t memory_size = class_list_[i].GetMemory().GetMemorySize();
    memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);

    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
                 reinterpret_cast<const uint8_t*>(&memory_size + 1));

    for (std::size_t j = 0;
         j < class_list_[i].GetMemory().GetMemoryType().size(); j++) {
      for (std::size_t k = 0;
           k < class_list_[i].GetMemory().GetVarName()[j].size() + 1; k++) {
        code_.push_back(class_list_[i].GetMemory().GetVarName()[j].c_str()[k]);
      }
      code_.push_back(class_list_[i].GetMemory().GetMemoryType()[j]);
    }

    // std::cout << "Point B" << std::endl;

    std::size_t methods_size = class_list_[i].GetFuncList().size();
    methods_size = is_big_endian_ ? methods_size : SwapUint64t(methods_size);
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&methods_size),
                 reinterpret_cast<const uint8_t*>(&methods_size + 1));

    std::vector<Function> func_list = class_list_[i].GetFuncList();

    // std::cout << "Point C" << std::endl;

    for (std::size_t z = 0; z < func_list.size(); z++) {
      // std::cout << func_list.size() << std::endl;
      //  Function name (with '\0')
      std::string func_name_str = func_list[z].GetName();
      const char* func_name = func_name_str.c_str();
      // std::cout << func_name_str << std::endl;
      code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                   reinterpret_cast<const uint8_t*>(
                       func_name + func_list[z].GetName().size() + 1));

      if (func_list[z].GetVaFlag()) code_.push_back(0xFF);

      std::vector<uint8_t> args_buffer;

      EncodeUleb128(func_list[z].GetArgs().size(), args_buffer);
      code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
      for (std::size_t j = 0; j < func_list[z].GetArgs().size(); j++) {
        args_buffer.clear();
        EncodeUleb128(func_list[z].GetArgs()[j], args_buffer);
        code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
      }

      uint64_t value = is_big_endian_
                           ? func_list[z].GetCode().size()
                           : SwapUint64t(func_list[z].GetCode().size());
      code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                   reinterpret_cast<uint8_t*>(&value) + 8);

      for (std::size_t j = 0; j < func_list[z].GetCode().size(); j++) {
        std::vector<uint8_t> buffer;
        switch (func_list[z].GetCode()[j].GetOper()) {
          case _AQVM_OPERATOR_NOP:
            code_.push_back(_AQVM_OPERATOR_NOP);
            break;

          case _AQVM_OPERATOR_LOAD:
            code_.push_back(_AQVM_OPERATOR_LOAD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected LOAD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_STORE:
            code_.push_back(_AQVM_OPERATOR_STORE);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected STORE args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEW:
            code_.push_back(_AQVM_OPERATOR_NEW);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected NEW args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ARRAY:
            code_.push_back(_AQVM_OPERATOR_ARRAY);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected ARRAY args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_PTR:
            code_.push_back(_AQVM_OPERATOR_PTR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected PTR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ADD:
            code_.push_back(_AQVM_OPERATOR_ADD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected ADD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SUB:
            code_.push_back(_AQVM_OPERATOR_SUB);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected SUB args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_MUL:
            code_.push_back(_AQVM_OPERATOR_MUL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected MUL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_DIV:
            code_.push_back(_AQVM_OPERATOR_DIV);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected DIV args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REM:
            code_.push_back(_AQVM_OPERATOR_REM);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected REM args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEG:
            code_.push_back(_AQVM_OPERATOR_NEG);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected NEG args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHL:
            code_.push_back(_AQVM_OPERATOR_SHL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected SHL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHR:
            code_.push_back(_AQVM_OPERATOR_SHR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected SHR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REFER:
            code_.push_back(_AQVM_OPERATOR_REFER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected REFER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            break;

          case _AQVM_OPERATOR_IF:
            code_.push_back(_AQVM_OPERATOR_IF);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected IF args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_AND:
            code_.push_back(_AQVM_OPERATOR_AND);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected AND args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_OR:
            code_.push_back(_AQVM_OPERATOR_OR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected OR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_XOR:
            code_.push_back(_AQVM_OPERATOR_XOR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected XOR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CMP:
            code_.push_back(_AQVM_OPERATOR_CMP);

            if (func_list[z].GetCode()[j].GetArgs().size() != 4)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected CMP args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[3], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE:
            code_.push_back(_AQVM_OPERATOR_INVOKE);

            if (func_list[z].GetCode()[j].GetArgs().size() < 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected INVOKE args size.");

            for (std::size_t k = 0;
                 k != func_list[z].GetCode()[j].GetArgs()[1] + 2; k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code_.insert(code_.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_EQUAL:
            code_.push_back(_AQVM_OPERATOR_EQUAL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected EQUAL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_GOTO:
            code_.push_back(_AQVM_OPERATOR_GOTO);

            if (func_list[z].GetCode()[j].GetArgs().size() != 1)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected GOTO args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_LOAD_CONST:
            code_.push_back(_AQVM_OPERATOR_LOAD_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected LOAD_CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONVERT:
            code_.push_back(_AQVM_OPERATOR_CONVERT);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected CONVERT args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONST:
            code_.push_back(_AQVM_OPERATOR_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE_METHOD:
            code_.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

            if (func_list[z].GetCode()[j].GetArgs().size() < 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected INVOKE_METHOD args size.");

            /*std::cout << "INVOKE_METHOD SIZE: "
                      << func_list[z].GetCode()[j].GetArgs().size()
                      << std::endl;*/

            for (std::size_t k = 0;
                 k < func_list[z].GetCode()[j].GetArgs().size(); k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code_.insert(code_.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_LOAD_MEMBER:
            code_.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                            "Unexpected LOAD_MEMBER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_WIDE:
            code_.push_back(_AQVM_OPERATOR_WIDE);
            break;

          default:
            break;
        }
      }
    }
  }

  // std::cout << "SUCCESS BEFORE __start" << std::endl;

  std::string class_name_str = ".!__start";

  const char* class_name = class_name_str.c_str();
  code_.insert(
      code_.end(), reinterpret_cast<const uint8_t*>(class_name),
      reinterpret_cast<const uint8_t*>(class_name + class_name_str.size() + 1));

  /*memory_size = 0;
  memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);
  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
               reinterpret_cast<const uint8_t*>(&memory_size + 1));
  for (std::size_t i = 0;
       i < class_list_[i].GetMemory().GetMemoryType().size(); i++) {
    code_.push_back(class_list_[i].GetMemory().GetMemoryType()[i]);
  }*/

  memory_size = start_class_.GetMemory().GetMemorySize();
  memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);

  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
               reinterpret_cast<const uint8_t*>(&memory_size + 1));

  for (std::size_t j = 0; j < start_class_.GetMemory().GetMemoryType().size();
       j++) {
    for (std::size_t k = 0;
         k < start_class_.GetMemory().GetVarName()[j].size() + 1; k++) {
      code_.push_back(start_class_.GetMemory().GetVarName()[j].c_str()[k]);
    }
    code_.push_back(start_class_.GetMemory().GetMemoryType()[j]);
  }

  std::size_t methods_size = func_list_.size();
  methods_size = is_big_endian_ ? methods_size : SwapUint64t(methods_size);
  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&methods_size),
               reinterpret_cast<const uint8_t*>(&methods_size + 1));

  std::vector<Function> func_list = func_list_;

  for (std::size_t i = 0; i < func_list.size(); i++) {
    // std::cout << func_list.size() << std::endl;
    //  Function name (with '\0')
    std::string func_name_str = func_list[i].GetName();
    const char* func_name = func_name_str.c_str();
    // std::cout << func_name_str << std::endl;
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                 reinterpret_cast<const uint8_t*>(
                     func_name + func_list[i].GetName().size() + 1));

    if (func_list[i].GetVaFlag()) code_.push_back(0xFF);

    std::vector<uint8_t> args_buffer;

    EncodeUleb128(func_list[i].GetArgs().size(), args_buffer);
    code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    for (std::size_t j = 0; j < func_list[i].GetArgs().size(); j++) {
      args_buffer.clear();
      EncodeUleb128(func_list[i].GetArgs()[j], args_buffer);
      code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    }

    uint64_t value = is_big_endian_
                         ? func_list[i].GetCode().size()
                         : SwapUint64t(func_list[i].GetCode().size());
    code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                 reinterpret_cast<uint8_t*>(&value) + 8);

    for (std::size_t j = 0; j < func_list[i].GetCode().size(); j++) {
      std::vector<uint8_t> buffer;
      switch (func_list[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          code_.push_back(_AQVM_OPERATOR_NOP);
          break;

        case _AQVM_OPERATOR_LOAD:
          code_.push_back(_AQVM_OPERATOR_LOAD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected LOAD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_STORE:
          code_.push_back(_AQVM_OPERATOR_STORE);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected STORE args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEW:
          code_.push_back(_AQVM_OPERATOR_NEW);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected NEW args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ARRAY:
          code_.push_back(_AQVM_OPERATOR_ARRAY);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected ARRAY args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_PTR:
          code_.push_back(_AQVM_OPERATOR_PTR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected PTR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ADD:
          code_.push_back(_AQVM_OPERATOR_ADD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected ADD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SUB:
          code_.push_back(_AQVM_OPERATOR_SUB);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected SUB args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_MUL:
          code_.push_back(_AQVM_OPERATOR_MUL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected MUL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_DIV:
          code_.push_back(_AQVM_OPERATOR_DIV);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected DIV args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REM:
          code_.push_back(_AQVM_OPERATOR_REM);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected REM args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEG:
          code_.push_back(_AQVM_OPERATOR_NEG);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected NEG args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHL:
          code_.push_back(_AQVM_OPERATOR_SHL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected SHL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHR:
          code_.push_back(_AQVM_OPERATOR_SHR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected SHR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REFER:
          code_.push_back(_AQVM_OPERATOR_REFER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected REFER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          break;

        case _AQVM_OPERATOR_IF:
          code_.push_back(_AQVM_OPERATOR_IF);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected IF args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_AND:
          code_.push_back(_AQVM_OPERATOR_AND);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected AND args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_OR:
          code_.push_back(_AQVM_OPERATOR_OR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected OR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_XOR:
          code_.push_back(_AQVM_OPERATOR_XOR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected XOR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CMP:
          code_.push_back(_AQVM_OPERATOR_CMP);

          if (func_list[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected CMP args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[3], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE:
          code_.push_back(_AQVM_OPERATOR_INVOKE);

          if (func_list[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected INVOKE args size.");

          for (std::size_t k = 0;
               k != func_list[i].GetCode()[j].GetArgs()[1] + 2; k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_EQUAL:
          code_.push_back(_AQVM_OPERATOR_EQUAL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected EQUAL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_GOTO:
          code_.push_back(_AQVM_OPERATOR_GOTO);

          if (func_list[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected GOTO args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_LOAD_CONST:
          code_.push_back(_AQVM_OPERATOR_LOAD_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected LOAD_CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONVERT:
          code_.push_back(_AQVM_OPERATOR_CONVERT);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected CONVERT args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONST:
          code_.push_back(_AQVM_OPERATOR_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          code_.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

          if (func_list[i].GetCode()[j].GetArgs().size() < 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected INVOKE_METHOD args size.");

          /*std::cout << "INVOKE_METHOD SIZE: "
                    << func_list[i].GetCode()[j].GetArgs().size() <<
             std::endl;*/

          for (std::size_t k = 0;
               k < func_list[i].GetCode()[j].GetArgs().size(); k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          code_.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateBytecode(Ast::Compound*)",
                          "Unexpected LOAD_MEMBER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_WIDE:
          code_.push_back(_AQVM_OPERATOR_WIDE);
          break;

        default:
          break;
      }
    }
  }

  std::string filename(output_file);
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile) {
    EXIT_COMPILER("Generator::GenerateBytecodeFile(const char*)",
                  "Can't open file.");
    return;
  }

  outFile.write(reinterpret_cast<const char*>(code_.data()), code_.size());
  outFile.close();

  if (!outFile) {
    EXIT_COMPILER("Generator::GenerateBytecodeFile(const char*)",
                  "Failed to write file.");
  } else {
    std::cout << "[INFO] " << "Write file success: " << filename << std::endl;
  }

  bool is_output_mnemonic = true;
  if (is_output_mnemonic) {
    GenerateMnemonicFile(output_file);
  }
}

void Generator::GenerateMnemonicFile(const char* output_file_name) {
  std::ofstream output_file(std::string(output_file_name) + ".mnemonic.txt");
  if (!output_file) {
    EXIT_COMPILER("Generator::GenerateMnemonicFile()", "Can't open file.");
  }

  std::streambuf* cout_buffer = std::cout.rdbuf();
  std::cout.rdbuf(output_file.rdbuf());

  std::size_t memory_size = global_memory_.GetMemorySize();
  std::cout << "Memory Size: " << memory_size << std::endl;
  std::cout << std::endl << std::endl << std::endl;

  for (std::size_t i = 0; i < func_list_.size(); i++) {
    std::cout << "Function Name: " << func_list_[i].GetName()
              << ", Size: " << func_list_[i].GetCode().size() << std::endl;

    for (std::size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
      switch (func_list_[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          std::cout << "NOP" << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected LOAD args size.");
          std::cout << "LOAD: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_STORE:
          std::cout << "STORE: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected STORE args size.");
          break;

        case _AQVM_OPERATOR_NEW:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected NEW args size.");
          std::cout << "NEW: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_ARRAY:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected ARRAY args size.");
          std::cout << "ARRAY: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_PTR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected PTR args size.");
          std::cout << "PTR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_ADD:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected ADD args size.");
          std::cout << "ADD: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SUB:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected SUB args size.");
          std::cout << "SUB: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_MUL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected MUL args size.");
          std::cout << "MUL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_DIV:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected DIV args size.");
          std::cout << "DIV: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_REM:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected REM args size.");
          std::cout << "REM: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_NEG:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected NEG args size.");
          std::cout << "NEG: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_SHL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected SHL args size.");
          std::cout << "SHL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SHR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected SHR args size.");
          std::cout << "SHR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_REFER:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected REFER args size.");
          std::cout << "REFER: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_IF:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected IF args size.");
          std::cout << "IF: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_AND:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected AND args size.");
          std::cout << "AND: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_OR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected OR args size.");
          std::cout << "OR: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_XOR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected XOR args size.");
          std::cout << "XOR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_CMP:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected CMP args size.");
          std::cout << "CMP: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[3] << std::endl;
          break;

        case _AQVM_OPERATOR_INVOKE:
          if (func_list_[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected INVOKE args size.");
          std::cout << "INVOKE: ";
          for (std::size_t k = 0;
               k != func_list_[i].GetCode()[j].GetArgs()[1] + 2; k++) {
            std::cout << func_list_[i].GetCode()[j].GetArgs()[k] << " ,";
          }
          std::cout << std::endl;
          break;

        case _AQVM_OPERATOR_EQUAL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected GOTO args size.");
          std::cout << "EQUAL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_GOTO:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected GOTO args size.");
          std::cout << "GOTO: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD_CONST:
          std::cout << "LOAD_CONST: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_CONVERT:
          std::cout << "CONVERT: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_CONST:
          std::cout << "CONST: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          if (func_list_[i].GetCode()[j].GetArgs().size() < 3)
            EXIT_COMPILER("Generator::GenerateMnemonicFile()",
                          "Unexpected INVOKE_METHOD args size.");
          std::cout << "INVOKE_METHOD: ";
          for (std::size_t k = 0;
               k < func_list_[i].GetCode()[j].GetArgs().size(); k++) {
            std::cout << func_list_[i].GetCode()[j].GetArgs()[k] << " ,";
          }
          std::cout << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          std::cout << "LOAD_MEMBER: "
                    << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_WIDE:
          std::cout << "WIDE" << std::endl;
          break;

        default:
          break;
      }
    }
    std::cout << std::endl << std::endl << std::endl;
  }

  std::cout.rdbuf(cout_buffer);
  output_file.close();
}


void Generator::HandleFuncDecl(Ast::Function* func_decl) {
  /*current_scope_.push_back(
      static_cast<std::string>(*func_decl->GetStatement()->GetName()));*/
  if (func_decl == nullptr)
    EXIT_COMPILER("Generator::HandleFuncDecl(Ast::Function*)",
                  "func_decl is nullptr.");

  std::vector<Bytecode> code;
  std::string func_name;
  // for (std::size_t i = 0; i < current_scope_.size(); i++) {
  func_name += current_scope_.back();
  func_name += ".";
  //}
  // std::string single_scope_name = *func_decl->GetStatement()->GetName();
  func_name += *func_decl->GetStatement()->GetName();

  std::string scope_name = func_name;
  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStatement()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      // single_scope_name += "@";
      scope_name += "@";
    } else {
      // single_scope_name += ",";
      scope_name += ",";
    }

    if (args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kVarDecl &&
        args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kArrayDecl) {
      EXIT_COMPILER("Generator::HandleFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      // single_scope_name +=
      // *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      // single_scope_name +=
      // *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      scope_name += ",...";
    }
  }
  if (args.size() == 0 && func_decl->GetStatement()->GetVaFlag()) {
    scope_name += "@...";
  }

  goto_map_.clear();
  current_scope_.push_back(scope_name);
  // single_scope_.push_back(single_scope_name);
  current_func_index_ = current_scope_.size() - 1;
  // std::cout << "func_name: " << func_name << std::endl;
  /*if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<Ast::Function> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }*/

  if (func_decl->GetStatements() == nullptr) {
    current_scope_.pop_back();
    // single_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();

  std::size_t return_value_index = global_memory_.AddWithType(vm_type);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));
  args_index.push_back(return_value_reference_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i] == Ast::Statement::StatementType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("Generator::HandleFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  if (args.size() == 0 && func_decl->GetStatement()->GetVaFlag()) {
    va_array_index = global_memory_.Add(1);
    args_index.push_back(va_array_index);
  }

  for (size_t i = 0; i < func_decl->GetStatement()->GetArgs().size(); i++) {
    if (func_decl->GetStatement()->GetArgs()[i] ==
        Ast::Statement::StatementType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.AddWithType(vm_type)));
    } else if (func_decl->GetStatement()->GetArgs()[i] ==
               Ast::Statement::StatementType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(
              array_decl, global_memory_.AddWithType(vm_type)));
    } else {
      EXIT_COMPILER("Generator::HandleFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStatement()->GetArgs().size() - 1 &&
        func_decl->GetStatement()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  if (func_decl->GetStatement()->GetArgs().size() == 0 &&
      func_decl->GetStatement()->GetVaFlag()) {
    var_decl_map_.emplace(
        current_scope_.back() + "#args",
        std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
  }

  exit_index_.clear();
  HandleStmt(func_decl->GetStatements(), code);
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStatement()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  func_list_.push_back(func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("Generator::HandleFuncDecl(Ast::Function*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void Generator::HandleClassFuncDecl(Ast::Function* func_decl) {
  if (func_decl == nullptr)
    EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                  "func_decl is nullptr.");
  if (current_class_ == nullptr)
    EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                  "current_class_ is nullptr.");

  std::vector<Bytecode> code;
  std::string scope_name;
  scope_name += current_scope_.back();
  scope_name += ".";
  // std::string single_scope_name = *func_decl->GetStatement()->GetName();
  scope_name += *func_decl->GetStatement()->GetName();

  std::string func_name = *func_decl->GetStatement()->GetName();

  if (std::string(current_class_->GetClassDecl()->GetName()) == func_name) {
    HandleClassConstructor(func_decl);
    return;
  }

  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStatement()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      // single_scope_name += "@";
      scope_name += "@";
    } else {
      // single_scope_name += ",";
      scope_name += ",";
    }

    if (args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kVarDecl &&
        args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kArrayDecl) {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      // single_scope_name +=
      // *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      // single_scope_name +=
      // *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      scope_name += ",...";
    }
  }

  goto_map_.clear();
  // single_scope_.push_back(single_scope_name);
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;
  if (current_class_->GetFuncDeclMap().find(func_name) ==
      current_class_->GetFuncDeclMap().end()) {
    std::vector<Ast::Function> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    current_class_->GetFuncDeclMap().emplace(func_name, func_decl_vector);
  } else {
    current_class_->GetFuncDeclMap()[func_name].push_back(*func_decl);
  }

  if (func_decl->GetStatements() == nullptr) {
    current_scope_.pop_back();
    // single_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();

  std::size_t return_value_index = global_memory_.AddWithType(vm_type);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));
  args_index.push_back(return_value_reference_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i] == Ast::Statement::StatementType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  for (size_t i = 0; i < func_decl->GetStatement()->GetArgs().size(); i++) {
    if (func_decl->GetStatement()->GetArgs()[i] ==
        Ast::Statement::StatementType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.AddWithType(vm_type)));
    } else if (func_decl->GetStatement()->GetArgs()[i] ==
               Ast::Statement::StatementType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(
              array_decl, global_memory_.AddWithType(vm_type)));
    } else {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStatement()->GetArgs().size() - 1 &&
        func_decl->GetStatement()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  exit_index_.clear();
  HandleClassStmt(func_decl->GetStatements(), code);
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStatement()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  current_class_->GetFuncList().push_back(func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void Generator::HandleClassConstructor(Ast::Function* func_decl) {
  if (func_decl == nullptr)
    EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                  "func_decl is nullptr.");
  if (current_class_ == nullptr)
    EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                  "current_class_ is nullptr.");

  // std::cout << "HandleClassConstructor" << std::endl;

  std::vector<Bytecode> code;
  std::string class_name = current_class_->GetName();
  std::string scope_name = current_scope_.back();
  // scope_name += ".";
  // scope_name += *func_decl->GetStatement()->GetName();

  // std::string original_func_name = *func_decl->GetStatement()->GetName();
  std::string original_func_name = "@constructor";
  // std::cout << "Class Name: " << original_func_name << std::endl;
  std::string func_name = scope_name;

  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStatement()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      scope_name += "@";
    } else {
      scope_name += ",";
    }

    if (args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kVarDecl &&
        args[i]->GetStatementType() !=
            Ast::Statement::StatementType::kArrayDecl) {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      scope_name += ",...";
    }
  }

  goto_map_.clear();
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;
  if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<Ast::Function> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }

  if (func_decl->GetStatements() == nullptr) {
    current_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  // std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();
  // std::vector<uint8_t> vm_type;
  // vm_type.push_back(0x09);

  std::size_t return_value_index = global_memory_.Add(1);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  /*std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));*/
  args_index.push_back(return_value_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i] == Ast::Statement::StatementType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i] == Ast::Statement::StatementType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStatement()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  for (size_t i = 0; i < func_decl->GetStatement()->GetArgs().size(); i++) {
    if (func_decl->GetStatement()->GetArgs()[i] ==
        Ast::Statement::StatementType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.Add(1)));
    } else if (func_decl->GetStatement()->GetArgs()[i] ==
               Ast::Statement::StatementType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStatement()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(array_decl,
                                               global_memory_.Add(1)));
    } else {
      EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStatement()->GetArgs().size() - 1 &&
        func_decl->GetStatement()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_value_index,
                          global_memory_.AddUint64t(0),
                          global_memory_.AddString(class_name)));

  if (current_class_->GetFuncDeclMap().find(original_func_name) ==
      current_class_->GetFuncDeclMap().end()) {
    std::vector<Ast::Function> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    current_class_->GetFuncDeclMap().emplace(original_func_name,
                                             func_decl_vector);
  } else {
    current_class_->GetFuncDeclMap()[original_func_name].push_back(*func_decl);
  }

  std::vector<std::size_t> invoke_class_args;
  invoke_class_args.push_back(return_value_index);
  invoke_class_args.push_back(global_memory_.AddString(original_func_name));
  invoke_class_args.push_back(args_index.size());
  invoke_class_args.push_back(global_memory_.Add(1));
  invoke_class_args.insert(invoke_class_args.end(), args_index.begin() + 1,
                           args_index.end());
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_class_args));

  // code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStatement()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  func_list_.push_back(func_decl_bytecode);

  current_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();

  code.clear();
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;

  exit_index_.clear();

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  for (std::size_t i = 0; i < current_class_->GetCode().size(); i++) {
    code.push_back(current_class_->GetCode()[i]);
  }

  HandleClassStmt(func_decl->GetStatements(), code);
  // code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function new_func_decl_bytecode(original_func_name, args_index, code);
  if (func_decl->GetStatement()->GetVaFlag())
    new_func_decl_bytecode.EnableVaFlag();
  current_class_->GetFuncList().push_back(new_func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("Generator::HandleClassFuncDecl(Ast::Function*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void Generator::HandleClassDecl(Ast::Class* class_decl) {
  if (class_decl == nullptr)
    EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                  "class_decl is nullptr.");

  std::string class_name =
      current_scope_.back() + "." + std::string(class_decl->GetName());

  // std::string single_scope_name = std::string(class_decl->GetName());
  current_scope_.push_back(class_name);
  // single_scope_.push_back(single_scope_name);
  /*for (std::size_t i = 0; i < class_decl->GetMembers().size(); i++) {
    if (class_decl->GetMembers()[i]==
        Ast::Statement::StatementType::kVarDecl) {
      HandleVarDecl(dynamic_cast<VarDeclNode*>(class_decl->GetMembers()[i]),
                    global_code_);
    } else if (class_decl->GetMembers()[i]==
               Ast::Statement::StatementType::kArrayDecl) {
      HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(class_decl->GetMembers()[i]),
                      global_code_);
    } else {
      EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                    "Unexpected code.");
    }
  }*/

  if (class_decl_map_.find(class_name) == class_decl_map_.end())
    EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                  "Not found class decl.");
  Class* current_class = class_decl_map_[class_name];
  current_class_ = current_class;
  /*Class* current_class = new Class();
  current_class->SetName(class_name);
  current_class->SetClass(class_decl);
  current_class_ = current_class;

  current_class->GetMemory().SetCode(&current_class->GetCode());
  current_class->GetMemory().SetGlobalMemory(&global_memory_);

  if (class_decl_map_.find(class_name) != class_decl_map_.end())
    EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                  "Has same name class.");
  class_decl_map_.emplace(class_name, current_class);*/

  current_class->GetMemory().Add("@name");
  current_class->GetMemory().Add("@size");

  for (std::size_t i = 0; i < class_decl->GetSubClasses().size(); i++) {
    if (class_decl->GetSubClasses()[i] ==
        Ast::Statement::StatementType::kClassDecl) {
      HandleClassDecl(
          dynamic_cast<Ast::Class*>(class_decl->GetSubClasses()[i]));
    } else {
      EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                    "Unexpected code.");
    }
  }
  current_class_ = current_class;

  for (std::size_t i = 0; i < class_decl->GetStaticMembers().size(); i++) {
    // class_decl->GetStaticMembers()[i]->GetDecl()
    // std::cout << "HANDLE STATIC MEMBERS." << std::endl;
    if (class_decl->GetStaticMembers()[i]->GetDecl() ==
        Ast::Statement::StatementType::kVarDecl) {
      HandleStaticVarDecl(dynamic_cast<VarDeclNode*>(
                              class_decl->GetStaticMembers()[i]->GetDecl()),
                          current_class->GetCode());
    } else if (class_decl->GetStaticMembers()[i]->GetDecl() ==
               Ast::Statement::StatementType::kArrayDecl) {
      HandleStaticArrayDecl(dynamic_cast<ArrayDeclNode*>(
                                class_decl->GetStaticMembers()[i]->GetDecl()),
                            current_class->GetCode());
    } else if (class_decl->GetStaticMembers()[i]->GetDecl() ==
               Ast::Statement::StatementType::kFuncDecl) {
      HandleFuncDecl(dynamic_cast<Ast::Function*>(
          class_decl->GetStaticMembers()[i]->GetDecl()));
    } else {
      EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < class_decl->GetMembers().size(); i++) {
    // std::cout << "Handle var in HandleClassDecl" << std::endl;
    if (class_decl->GetMembers()[i] ==
        Ast::Statement::StatementType::kVarDecl) {
      HandleClassVarDecl(
          current_class->GetMemory(), current_class->GetVarDeclMap(),
          dynamic_cast<VarDeclNode*>(class_decl->GetMembers()[i]),
          current_class->GetCode());
    } else if (class_decl->GetMembers()[i] ==
               Ast::Statement::StatementType::kArrayDecl) {
      HandleClassArrayDecl(
          current_class->GetMemory(), current_class->GetVarDeclMap(),
          dynamic_cast<ArrayDeclNode*>(class_decl->GetMembers()[i]),
          current_class->GetCode());
    } else {
      EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < class_decl->GetMethods().size(); i++) {
    if (class_decl->GetMethods()[i] ==
        Ast::Statement::StatementType::kFuncDecl) {
      HandleClassFuncDecl(
          dynamic_cast<Ast::Function*>(class_decl->GetMethods()[i]));
    } else {
      EXIT_COMPILER("Generator::HandleClassDecl(Ast::Class*)",
                    "Unexpected code.");
    }
  }

  auto iterator = func_decl_map_.find(current_scope_.back());
  if (iterator == func_decl_map_.end()) {
    std::vector<Ast::Function> func_decl_vector;
    Ast::Function* func_decl = new Ast::Function();
    func_decl->SetAst::Function(nullptr, nullptr, nullptr);
    std::vector<Bytecode> code;
    std::string class_name = current_class_->GetName();
    std::string scope_name;
    scope_name += current_scope_.back();
    std::string original_func_name = "@constructor";
    std::string func_name = scope_name;

    goto_map_.clear();
    current_scope_.push_back(scope_name);
    current_func_index_ = current_scope_.size() - 1;
    if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
      std::vector<Ast::Function> func_decl_vector;
      func_decl_vector.push_back(*func_decl);
      func_decl_map_.emplace(func_name, func_decl_vector);
    } else {
      func_decl_map_[func_name].push_back(*func_decl);
    }

    std::vector<std::size_t> args_index;

    std::size_t return_value_index = global_memory_.Add(1);
    var_decl_map_.emplace(
        scope_name + "#!return",
        std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

    args_index.push_back(return_value_index);

    std::size_t va_array_index = 0;

    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_value_index,
                            global_memory_.AddUint64t(0),
                            global_memory_.AddString(class_name)));

    if (current_class_->GetFuncDeclMap().find(original_func_name) ==
        current_class_->GetFuncDeclMap().end()) {
      func_decl_vector.push_back(*func_decl);
      current_class_->GetFuncDeclMap().emplace(original_func_name,
                                               func_decl_vector);
    } else {
      current_class_->GetFuncDeclMap()[original_func_name].push_back(
          *func_decl);
    }

    std::vector<std::size_t> invoke_class_args;
    invoke_class_args.push_back(return_value_index);
    invoke_class_args.push_back(global_memory_.AddString(original_func_name));
    invoke_class_args.push_back(args_index.size());
    invoke_class_args.push_back(global_memory_.Add(1));
    invoke_class_args.insert(invoke_class_args.end(), args_index.begin() + 1,
                             args_index.end());
    code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_class_args));

    Function func_decl_bytecode(func_name, args_index, code);
    func_list_.push_back(func_decl_bytecode);

    current_scope_.pop_back();
    current_func_index_ = 0;
    goto_map_.clear();

    code.clear();
    current_scope_.push_back(scope_name);
    current_func_index_ = current_scope_.size() - 1;

    exit_index_.clear();

    code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

    Function new_func_decl_bytecode(original_func_name, args_index, code);
    current_class_->GetFuncList().push_back(new_func_decl_bytecode);
    exit_index_.clear();

    current_scope_.pop_back();
    current_func_index_ = 0;
    goto_map_.clear();
  }

  // current_class_ = current_class;

  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_class_ = nullptr;
  class_list_.push_back(*current_class);
}

std::size_t Generator::HandleVarDecl(VarDeclNode* var_decl,
                                     std::vector<Bytecode>& code) {
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
        "var_decl is nullptr.");

  /*Type* var_type = var_decl->GetVarType();
  if (var_type == nullptr)
    EXIT_COMPILER(
        "Generator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
        "var_type is nullptr.");

  while (var_type->GetStatementType() != Type::TypeType::kBase &&
         var_type->GetStatementType() != Type::TypeType::kPointer &&
         var_type->GetStatementType() != Type::TypeType::kArray &&
         var_type->GetStatementType() != Type::TypeType::kReference) {
    if (var_type== Type::TypeType::NONE)
      EXIT_COMPILER(
          "Generator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
          "Unexpected code.");
    if (var_type== Type::TypeType::kConst)
      var_type = dynamic_cast<ConstType*>(var_type)->GetSubType();
  }

  uint8_t vm_type = 0x00;
  if (var_type== Type::TypeType::kBase) {
    switch (var_type->GetBaseType()) {
      case Type::BaseType::kAuto:
      case Type::BaseType::kVoid:
        vm_type = 0x00;
        break;
      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        vm_type = 0x01;
        break;
      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
      case Type::BaseType::kLong:
        vm_type = 0x02;
        break;
      case Type::BaseType::kFloat:
      case Type::BaseType::kDouble:
        vm_type = 0x03;
        break;
        // TODO(uint64_t)
        vm_type = 0x04;
        break;
      case Type::BaseType::kString:
        vm_type = 0x05;
        break;
      case Type::BaseType::kClass:
      case Type::BaseType::kStruct:
      case Type::BaseType::kUnion:
      case Type::BaseType::kEnum:
      case Type::BaseType::kPointer:
      case Type::BaseType::kArray:
      case Type::BaseType::kFunction:
      case Type::BaseType::kTypedef:
        // TODO
        vm_type = 0x06;
        break;
      default:
        EXIT_COMPILER(
            "Generator::HandleVarDecl(VarDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }
  } else if (var_type== Type::TypeType::kPointer) {
    vm_type = 0x06;
  } else if (var_type== Type::TypeType::kReference) {
    vm_type = 0x07;
  }*/

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType() == Type::TypeType::kConst) {
    // return_type.insert(return_type.begin(), 0x08);
    vm_type.erase(vm_type.begin());
  }

  // TODO(Class)

  if (var_decl->GetValue()[0] == nullptr) {
    // std::cout << "None Value" << std::endl;
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, var_index_reference,
                   global_memory_.AddString("@constructor"), 1, 0));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    /*if (var_decl->GetVarType()== Type::TypeType::kConst)
    { EXIT_COMPILER(
          "Generator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&"
          ")",
          "Const doesn't have value.");
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }*/
    /*if (var_decl->GetVarType()==
      Type::TypeType::kReference) EXIT_COMPILER(
          "Generator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&"
          ")",
          "Reference doesn't have value.");*/
    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    // std::cout << "Has Value" << std::endl;
    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);
      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    }
    if (var_decl->GetVarType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, value_index, value_ptr_index));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, value_ptr_index));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  }
}

std::size_t Generator::HandleStaticVarDecl(VarDeclNode* var_decl,
                                           std::vector<Bytecode>& code) {
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
        "Bytecode>&)",
        "var_decl is nullptr.");

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType() == Type::TypeType::kConst)
      EXIT_COMPILER(
          "Generator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "const var without value not support.");
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, var_index_reference,
                   global_memory_.AddString("@constructor"), 1, 0));
    }
    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    }
    if (var_decl->GetVarType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map_.emplace(
          current_scope_.back() + "." +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    global_code_.push_back(
        Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      // std::vector<uint8_t> value_ptr = vm_type;
      // value_ptr.insert(value_ptr.begin(), 0x06);
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map_.emplace(
          current_scope_.back() + "." +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  }
}

std::size_t Generator::HandleClassVarDecl(
    ClassMemory& memory,
    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
        var_decl_map,
    VarDeclNode* var_decl, std::vector<Bytecode>& code) {
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleClassVarDecl(ClassMemory&,std::"
        "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
        "VarDeclNode*,std::vector<Bytecode>&)",
        "var_decl is nullptr.");

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  /*std::cout << "HandleClassVarDeclaration: "
            << static_cast<std::string>(*var_decl->GetName()) << std::endl;*/

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = memory.AddWithType(
        static_cast<std::string>(*var_decl->GetName()), vm_type);
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleClassVarDecl(ClassMemory&,std::"
              "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
              "VarDeclNode*,std::vector<Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 0,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      /*code.push_back(
      Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));*/

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    /*if (var_decl->GetVarType()== Type::TypeType::kConst)
    { EXIT_COMPILER( "Generator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }*/
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "Generator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
    }
    if (var_decl->GetVarType() == Type::TypeType::kReference)
      EXIT_COMPILER(
          "Generator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Reference doesn't have value.");
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    std::size_t original_var_index = 0;
    std::size_t var_index = 0;

    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      var_index = global_memory_.AddWithType(vm_type);
    } else {
      original_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), vm_type);
      var_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index, 0,
                              global_memory_.AddString(static_cast<std::string>(
                                  *var_decl->GetName()))));
    }
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleClassVarDecl(ClassMemory&,std::"
              "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
              "VarDeclNode*,std::vector<Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/

      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 0,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      /*code.push_back(
      Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference,
      reference_index));*/

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "Generator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
      /*std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;*/
    }

    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    if (var_decl->GetVarType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = memory.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, value_index, value_ptr_index));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      std::size_t const_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = memory.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, value_ptr_index));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, original_var_index));
    return original_var_index;
  }
}

std::size_t Generator::HandleStartVarDecl(VarDeclNode* var_decl,
                                          std::vector<Bytecode>& code) {
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStartVarDecl(VarDeclNode*,std::vector<"
        "Bytecode>&)",
        "var_decl is nullptr.");

  ClassMemory& memory = start_class_.GetMemory();
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
      var_decl_map = start_class_.GetVarDeclMap();

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = memory.AddWithType(
        static_cast<std::string>(*var_decl->GetName()), vm_type);

    std::size_t var_index_reference = global_memory_.Add(1);
    code.push_back(
        Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 2,
                 global_memory_.AddString(
                     static_cast<std::string>(*var_decl->GetName()))));

    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleStartVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference, 2,
                              global_memory_.AddString(func_name)));
    }
    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "Generator::HandleStartVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "Const doesn't have value.");
    }
    if (var_decl->GetVarType() == Type::TypeType::kReference)
      EXIT_COMPILER(
          "Generator::HandleStartVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "Reference doesn't have value.");
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index_reference;
  } else {
    std::size_t original_var_index = 0;
    std::size_t var_index = 0;

    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      var_index = global_memory_.AddWithType(vm_type);
    } else {
      original_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), vm_type);
      var_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index, 2,
                              global_memory_.AddString(static_cast<std::string>(
                                  *var_decl->GetName()))));
    }
    if (var_decl->GetVarType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "Generator::HandleStartVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 2,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
    }

    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    if (var_decl->GetVarType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }

    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));

    if (var_decl->GetVarType() == Type::TypeType::kConst) {
      std::size_t const_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), vm_type);
      std::size_t const_var_reference_index = global_memory_.Add(1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, const_var_reference_index, 2,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));
      // std::vector<uint8_t> value_ptr = vm_type;
      // value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(Bytecode(_AQVM_OPERATOR_CONST, 2,
                              const_var_reference_index, var_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_reference_index;
    }
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, original_var_index));
    return var_index;
  }
}

std::size_t Generator::HandleArrayDecl(ArrayDeclNode* array_decl,
                                       std::vector<Bytecode>& code) {
  // std::cout << "Array DECL Handle. 1" << std::endl;

  if (array_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_type == Type::TypeType::kConst)
    EXIT_COMPILER(
        "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "const array not support.");

  if (array_decl->GetValue().empty()) {
    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    // std::size_t array_ptr_index = global_memory_.Add(1);
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            global_memory_.AddByte(1)
                            /*HandleExpr(array_decl->GetSize(), code)*/,
                            array_type_index));

    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddByte(0)));

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                              global_memory_.AddString("@constructor"), 1, 0));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  } else {
    // std::cout << "Array DECL Handle. value" << std::endl;
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    // std::cout << "array type .1" << std::endl;
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    // std::cout << "array type ." << std::endl;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    std::size_t size_index =
        global_memory_.AddUint64t(array_decl->GetValue().size());
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            size_index
                            /*HandleExpr(array_decl->GetSize(), code)*/,
                            array_type_index));

    // std::size_t array_ptr_index = global_memory_.Add(1);
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      // std::cout << "Handle Array DECL with value." << std::endl;
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  }
}

std::size_t Generator::HandleStaticArrayDecl(ArrayDeclNode* array_decl,
                                             std::vector<Bytecode>& code) {
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_type == Type::TypeType::kConst)
    EXIT_COMPILER(
        "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "const array not support.");

  if (array_decl->GetValue().empty()) {
    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    global_code_.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                                    global_memory_.AddByte(1)
                                    /*HandleExpr(array_decl->GetSize(), code)*/,
                                    array_type_index));

    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::size_t current_index = global_memory_.Add(1);
      global_code_.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                                      array_index, global_memory_.AddByte(0)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                   global_memory_.AddString("@constructor"), 1, 0));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  } else {
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleStaticArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    std::size_t size_index =
        global_memory_.AddUint64t(array_decl->GetValue().size());
    global_code_.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                                    size_index
                                    /*HandleExpr(array_decl->GetSize(), code)*/,
                                    array_type_index));

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      global_code_.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                                      array_index,
                                      global_memory_.AddUint64t(i)));
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  }
}

std::size_t Generator::HandleClassArrayDecl(
    ClassMemory& memory,
    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
        var_decl_map,
    ArrayDeclNode* array_decl, std::vector<Bytecode>& code) {
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_decl->GetValue().empty()) {
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());

    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 0,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  } else {
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());
    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 0,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            HandleExpr(array_decl->GetSize(), code),
                            array_type_index));
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  }
}

std::size_t Generator::HandleStartArrayDecl(ArrayDeclNode* array_decl,
                                            std::vector<Bytecode>& code) {
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  ClassMemory& memory = start_class_.GetMemory();
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
      var_decl_map = start_class_.GetVarDeclMap();

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_decl->GetValue().empty()) {
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());

    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 2,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  } else {
    if (array_decl->GetSize()->GetStatementType() !=
        Ast::Statement::StatementType::kValue)
      EXIT_COMPILER(
          "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());
    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 2,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "Generator::HandleStartArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            HandleExpr(array_decl->GetSize(), code),
                            array_type_index));

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  }
}

std::size_t Generator::HandleExpr(ExprNode* expr, std::vector<Bytecode>& code) {
  if (expr == nullptr)
    EXIT_COMPILER("Generator::HandleExpr(ExprNode*,std::vector<Bytecode>&)",
                  "expr is nullptr.");

  if (expr == Ast::Statement::StatementType::kUnary ||
      expr == Ast::Statement::StatementType::kArray) {
    return HandleUnaryExpr(dynamic_cast<UnaryNode*>(expr), code);
  } else if (expr == Ast::Statement::StatementType::kBinary) {
    return HandleBinaryExpr(dynamic_cast<BinaryNode*>(expr), code);
  }

  return GetIndex(expr, code);
}

std::size_t Generator::HandleUnaryExpr(UnaryNode* expr,
                                       std::vector<Bytecode>& code) {
  if (expr == nullptr)
    EXIT_COMPILER(
        "Generator::HandleUnaryExpr(UnaryNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  std::size_t sub_expr = HandleExpr(expr->GetExpr(), code);
  switch (expr->GetOperator()) {
    case UnaryNode::Operator::kPostInc: {  // ++ (postfix)
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));

      return new_index;
    }
    case UnaryNode::Operator::kPostDec: {  // -- (postfix)
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));

      return new_index;
    }
    case UnaryNode::Operator::kPreInc:  // ++ (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));

      return sub_expr;
    case UnaryNode::Operator::kPreDec:  // -- (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));

      return sub_expr;

    case UnaryNode::Operator::kPlus:  // + (unary plus)
      return sub_expr;
    case UnaryNode::Operator::kMinus: {  // - (unary minus)

      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }
    case UnaryNode::Operator::kNot: {  // ! (logical NOT)
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }

    case UnaryNode::Operator::ARRAY: {  // []

      std::size_t offset =
          HandleExpr(dynamic_cast<ArrayNode*>(expr)->GetIndex(), code);

      std::size_t new_index = global_memory_.Add(1);

      code.push_back(
          Bytecode(_AQVM_OPERATOR_ARRAY, 3, new_index, sub_expr, offset));
      return new_index;
    }
    case UnaryNode::Operator::kBitwiseNot:  // ~ (bitwise NOT)
                                            // TODO
    default:
      return sub_expr;
  }
}
std::size_t Generator::HandleBinaryExpr(BinaryNode* expr,
                                        std::vector<Bytecode>& code) {
  if (expr == nullptr)
    EXIT_COMPILER(
        "Generator::HandleBinaryExpr(BinaryNode*,std::vector<Bytecode>&"
        ")",
        "expr is nullptr.");

  // std::cout << "HandleBinaryExpr RUNNING" << std::endl;

  ExprNode* right_expr = expr->GetRightExpr();
  ExprNode* left_expr = expr->GetLeftExpr();

  std::size_t left = 0;
  std::size_t right = 0;
  if (expr->GetOperator() != BinaryNode::Operator::kMember &&
      expr->GetOperator() != BinaryNode::Operator::kArrow)
    right = HandleExpr(right_expr, code);
  if (expr->GetOperator() != BinaryNode::Operator::kMember)
    left = HandleExpr(left_expr, code);
  // uint8_t right_type = GetExprVmType(right_expr);
  // uint8_t left_type = GetExprVmType(left_expr);
  // uint8_t result_type = left_type > right_type ? left_type : right_type;

  switch (expr->GetOperator()) {
    case BinaryNode::Operator::kAdd: {  // +
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kSub: {  // -
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kMul: {  // *
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kDiv: {  // /
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kRem: {  // %
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAnd: {  // &
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kOr: {  // |
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kXor: {  // ^
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShl: {  // <<
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShr: {  // >>
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLT: {  // <
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x04,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kGT: {  // >
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x02,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kLE: {  // <=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x05,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kGE: {  // >=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x03,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kEQ: {  // ==
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x00,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kNE: {  // !=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x01,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kLAnd: {  // &&
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLOr: {  // ||
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAssign:  // =
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, left, right));

      return left;
    case BinaryNode::Operator::kAddAssign:  // +=
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kSubAssign:  // -=
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kMulAssign:  // *=
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kDivAssign:  // /=
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kRemAssign:  // %=
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kAndAssign:  // &=
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kOrAssign:  // |=

      return left;
    case BinaryNode::Operator::kXorAssign:  // ^=
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kShlAssign:  // <<=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kShrAssign:  // >>=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, left, left, right));

      return left;
    case BinaryNode::Operator::kMember: {
      return HandlePeriodExpr(expr, code);

      break;
    }

    case BinaryNode::Operator::kArrow:

    case BinaryNode::Operator::kComma:    // ,
                                          // std::cout << "Comma" << std::endl;
    case BinaryNode::Operator::kPtrMemD:  // .*
    case BinaryNode::Operator::kPtrMemI:  // ->*
    default:
      // TODO
      EXIT_COMPILER(
          "Generator::HandleBinaryExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unexpected code.");
      break;
  }
  EXIT_COMPILER(
      "Generator::HandleBinaryExpr(BinaryNode*,std::vector<Bytecode>&)",
      "Unexpected code.");
  return 0;
}

std::size_t Generator::HandlePeriodExpr(BinaryNode* expr,
                                        std::vector<Bytecode>& code) {
  // std::cout << "HandlePeriodExpr CALLED." << std::endl;
  if (expr->GetOperator() != BinaryNode::Operator::kMember)
    EXIT_COMPILER(
        "Generator::HandlePeriodExpr(BinaryNode*,std::vector<Bytecode>&"
        ")",
        "Not period expr.");

  ExprNode* handle_expr = expr;
  std::vector<ExprNode*> exprs;
  // exprs.push_back(expr->GetRightExpr());
  while (handle_expr != nullptr) {
    if (handle_expr == Ast::Statement::StatementType::kBinary) {
      if (dynamic_cast<BinaryNode*>(handle_expr)->GetOperator() !=
          BinaryNode::Operator::kMember)
        break;
      exprs.insert(exprs.begin(),
                   dynamic_cast<BinaryNode*>(handle_expr)->GetRightExpr());
      handle_expr = dynamic_cast<BinaryNode*>(handle_expr)->GetLeftExpr();
    } else {
      exprs.insert(exprs.begin(), handle_expr);
      handle_expr = nullptr;
    }
  }

  bool is_end = true;
  std::string full_name;
  for (std::size_t i = 0; i < exprs.size() - 1; i++) {
    if (exprs[i]->GetStatementType() !=
        Ast::Statement::StatementType::kIdentifier) {
      is_end = false;
      break;
    }
    full_name += (std::string) * dynamic_cast<IdentifierNode*>(exprs[i]) +
                 std::string(".");
  }
  if (is_end) {
    if (exprs.back() == Ast::Statement::StatementType::kFunc) {
      full_name += *dynamic_cast<FuncNode*>(exprs.back())->GetName();
      for (int64_t k = current_scope_.size() - 1; k >= 0; k--) {
        auto iterator =
            func_decl_map_.find(current_scope_[k] + "." + full_name);
        if (iterator != func_decl_map_.end()) {
          full_name = current_scope_[k] + "." + full_name;
          std::size_t return_value_index = global_memory_.Add(1);
          std::size_t return_value_reference_index = global_memory_.Add(1);
          code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                  return_value_reference_index,
                                  return_value_index));
          std::vector<std::size_t> args;
          args.push_back(2);
          args.push_back(global_memory_.AddString(full_name));
          args.push_back(
              dynamic_cast<FuncNode*>(exprs.back())->GetArgs().size() + 1);
          args.push_back(return_value_reference_index);
          for (std::size_t i = 0;
               i < dynamic_cast<FuncNode*>(exprs.back())->GetArgs().size();
               i++) {
            args.push_back(HandleExpr(
                dynamic_cast<FuncNode*>(exprs.back())->GetArgs()[i], code));
          }
          code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, args));
          return return_value_index;
        }
      }
    } else if (exprs.back() == Ast::Statement::StatementType::kIdentifier) {
      full_name += *dynamic_cast<IdentifierNode*>(exprs.back());
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(current_scope_[i] + "." + full_name);
        if (iterator != var_decl_map_.end()) {
          return iterator->second.second;
        }
      }
    } else {
      EXIT_COMPILER(
          "Generator::HandlePeriodExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unsupported stmt type.");
    }
  }

  switch (expr->GetRightExpr()->GetStatementType()) {
    case Ast::Statement::StatementType::kFunc: {
      std::size_t return_value_index = global_memory_.Add(1);
      std::size_t return_value_reference_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                              return_value_reference_index,
                              return_value_index));
      std::vector<std::size_t> args;
      args.push_back(HandleExpr(expr->GetLeftExpr(), code));
      args.push_back(global_memory_.AddString(
          *dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetName()));
      args.push_back(
          dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs().size() + 1);
      args.push_back(return_value_reference_index);
      for (std::size_t i = 0;
           i < dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs().size();
           i++) {
        args.push_back(HandleExpr(
            dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs()[i], code));
      }
      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, args));
      break;
    }
    case Ast::Statement::StatementType::kIdentifier: {
      std::size_t return_value_index = global_memory_.Add(1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_value_index,
                   HandleExpr(expr->GetLeftExpr(), code),
                   global_memory_.AddString(
                       *dynamic_cast<IdentifierNode*>(expr->GetRightExpr()))));
      return return_value_index;
    }
    default:
      EXIT_COMPILER(
          "Generator::HandlePeriodExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unsupported expr.");
      break;
  }
  return 0;
}

void Generator::HandleStmt(Ast::Statement* stmt, std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStmt(Ast::Statement*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  switch (stmt->GetStatementType()) {
    case Ast::Statement::StatementType::kImport:
      // HandleImport(dynamic_cast<Ast::Import*>(stmt));
      break;

    case Ast::Statement::StatementType::kBreak:
      HandleBreakStmt(code);
      break;

    case Ast::Statement::StatementType::kCompound:
      HandleCompoundStmt(dynamic_cast<Ast::Compound*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kExpr:
      HandleExpr(dynamic_cast<ExprNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kUnary:
      HandleUnaryExpr(dynamic_cast<UnaryNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kBinary:
      HandleBinaryExpr(dynamic_cast<BinaryNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kIf:
      HandleIfStmt(dynamic_cast<IfNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kWhile:
      HandleWhileStmt(dynamic_cast<WhileNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kDowhile:
      HandleDowhileStmt(dynamic_cast<DowhileNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kFor:
      HandleForStmt(dynamic_cast<ForNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kFunctionDeclaration:
      HandleFuncDecl(dynamic_cast<Ast::Function*>(stmt));
      break;

    case Ast::Statement::StatementType::kVariable:
      HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kArrayDeclaration:
      HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kClass:
      HandleClassDecl(dynamic_cast<Ast::Class*>(stmt));
      break;

    case Ast::Statement::StatementType::kFunc:
      HandleFuncInvoke(dynamic_cast<FuncNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kReturn:
      HandleReturn(dynamic_cast<ReturnNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kLabel:
      HandleLabel(dynamic_cast<LabelNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kGoto:
      HandleGoto(dynamic_cast<GotoNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kSwitch:
      HandleSwitchStmt(dynamic_cast<SwitchNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kStmt:
      // std::cout << "STMT WARNING." << std::endl;
      break;

    default:
      EXIT_COMPILER(
          "Generator::HandleStmt(Ast::Statement*,std::vector<Bytecode>&)",
          "Unexpected code.");
      break;
  }
}

void Generator::HandleClassStmt(Ast::Statement* stmt,
                                std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleClassStmt(Ast::Statement*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  switch (stmt->GetStatementType()) {
    case Ast::Statement::StatementType::kCompound:
      HandleCompoundStmt(dynamic_cast<Ast::Compound*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kExpr:
      HandleExpr(dynamic_cast<ExprNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kUnary:
      HandleUnaryExpr(dynamic_cast<UnaryNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kBinary:
      HandleBinaryExpr(dynamic_cast<BinaryNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kIf:
      HandleIfStmt(dynamic_cast<IfNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kWhile:
      HandleWhileStmt(dynamic_cast<WhileNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kDowhile:
      HandleDowhileStmt(dynamic_cast<DowhileNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kFor:
      HandleForStmt(dynamic_cast<ForNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kFunctionDeclaration:
      HandleFuncDecl(dynamic_cast<Ast::Function*>(stmt));
      break;

    case Ast::Statement::StatementType::kVariable:
      HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kArrayDeclaration:
      HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kClass:
      HandleClassDecl(dynamic_cast<Ast::Class*>(stmt));
      break;

    case Ast::Statement::StatementType::kFunc:
      HandleFuncInvoke(dynamic_cast<FuncNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kReturn:
      HandleReturn(dynamic_cast<ReturnNode*>(stmt), code);
      break;

    case Ast::Statement::StatementType::kLabel:
      HandleLabel(dynamic_cast<LabelNode*>(stmt), global_code_);
      break;

    case Ast::Statement::StatementType::kGoto:
      HandleGoto(dynamic_cast<GotoNode*>(stmt), global_code_);
      break;

    case Ast::Statement::StatementType::kStmt:
      // std::cout << "STMT WARNING." << std::endl;
      break;

    default:
      EXIT_COMPILER(
          "Generator::HandleClassStmt(Ast::Statement*,std::vector<Bytecode>&"
          ")",
          "Unexpected code.");
      break;
  }
}

void Generator::HandleBreakStmt(std::vector<Bytecode>& code) {
  std::size_t index = 0;
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64tWithoutValue(index)));
  // code.push_back(Bytecode(_AQVM_OPERATOR_GOTO,1,loop_break_index_.back()));

  loop_break_index_.push_back(index);
}

void Generator::HandleSwitchStmt(SwitchNode* stmt,
                                 std::vector<Bytecode>& code) {
  EXIT_COMPILER(
      "Generator::HandleSwitchStmt(SwitchNode*,std::vector<Bytecode>&)",
      "The switch statement is not yet supported.");
}

void Generator::HandleReturn(ReturnNode* stmt, std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER("Generator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
                  "stmt is nullptr.");

  if (stmt->GetExpr() == nullptr) {
    exit_index_.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  } else {
    std::size_t return_value = HandleExpr(stmt->GetExpr(), code);

    bool is_find = false;
    auto return_iterator = var_decl_map_.find("#!return");
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      return_iterator = var_decl_map_.find(current_scope_[i] + "#" +
                                           static_cast<std::string>("!return"));
      if (return_iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER(
          "Generator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
          "Not found identifier define.");

    is_find = false;
    auto return_reference_iterator = var_decl_map_.find("#!return_reference");
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      return_reference_iterator =
          var_decl_map_.find(current_scope_[i] + "#" +
                             static_cast<std::string>("!return_reference"));
      if (return_reference_iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER(
          "Generator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
          "Not found identifier define.");

    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2,
                            return_iterator->second.second, return_value));
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2,
                            return_reference_iterator->second.second,
                            return_iterator->second.second));
    exit_index_.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  }
}

void Generator::HandleCompoundStmt(Ast::Compound* stmt,
                                   std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleCompoundStmt(Ast::Compound*,std::vector<"
        "Bytecode>&)",
        "stmt is nullptr.");

  for (std::size_t i = 0; i < stmt->GetStatements().size(); i++) {
    HandleStmt(stmt->GetStatements()[i], code);
  }
}

void Generator::HandleIfStmt(IfNode* stmt, std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER("Generator::HandleIfStmt(IfNode*,std::vector<Bytecode>&)",
                  "stmt is nullptr.");

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  // Need true branch and false branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t true_location = code.size();
  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();

  std::size_t goto_location = code.size();
  // Need exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  std::size_t false_location = code.size();
  if (stmt->GetElseBody() != nullptr) {
    current_scope_.push_back(current_scope_.back() + "@@" +
                             std::to_string(++undefined_count_));
    HandleStmt(stmt->GetElseBody(), code);
    current_scope_.pop_back();
  }
  std::size_t exit_branch = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  std::vector<std::size_t> goto_args;
  if_args.push_back(condition_index);
  if_args.push_back(true_location);
  if_args.push_back(false_location);
  code[if_location].SetArgs(if_args);

  goto_args.push_back(global_memory_.AddUint64t(exit_branch));
  code[goto_location].SetArgs(goto_args);
}

void Generator::HandleWhileStmt(WhileNode* stmt, std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleWhileStmt(WhileNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  loop_break_index_.push_back(-1);

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  // Need body branch and exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t body_location = code.size();

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64t(start_location)));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();
}

void Generator::HandleDowhileStmt(DowhileNode* stmt,
                                  std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleDowhileStmt(DowhileNode*,std::vector<"
        "Bytecode>&)",
        "stmt is nullptr.");

  loop_break_index_.push_back(-1);

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::size_t body_location = code.size();

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();
}
void Generator::HandleForStmt(ForNode* stmt, std::vector<Bytecode>& code) {
  if (stmt == nullptr)
    EXIT_COMPILER(
        "Generator::HandleWhileStmt(WhileNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  loop_break_index_.push_back(-1);

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));

  HandleStmt(stmt->GetStart(), code);

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  // Need body branch and exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t body_location = code.size();

  HandleStmt(stmt->GetBody(), code);
  HandleExpr(stmt->GetEnd(), code);
  current_scope_.pop_back();
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64t(start_location)));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();
}

std::size_t Generator::HandleFuncInvoke(FuncNode* func,
                                        std::vector<Bytecode>& code) {
  if (func == nullptr)
    EXIT_COMPILER(
        "Generator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func is nullptr.");

  ExprNode* func_name_node = func->GetName();
  if (func_name_node == nullptr)
    EXIT_COMPILER(
        "Generator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func_name_node is nullptr.");
  std::string func_name = static_cast<std::string>(*func_name_node);
  std::vector<ExprNode*> args = func->GetArgs();

  for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
    auto iterator = func_decl_map_.find(func_name);
    if (i != -1)
      iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
    if (iterator != func_decl_map_.end()) {
      func_name = func_name;
      if (i != -1) func_name = current_scope_[i] + "." + func_name;
      break;
    }
    if (i == -1)
      EXIT_COMPILER(
          "Generator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&"
          ")",
          "Function not found.");
  }

  std::vector<std::size_t> vm_args;

  std::size_t func_name_index = global_memory_.AddString(func_name);

  vm_args.push_back(2);
  vm_args.push_back(func_name_index);
  vm_args.push_back(args.size() + 1);

  std::size_t return_value_index = global_memory_.Add(1);
  std::size_t return_value_reference_index = global_memory_.Add(1);
  code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, return_value_reference_index,
                          return_value_index));
  vm_args.push_back(return_value_reference_index);

  for (std::size_t i = 0; i < args.size(); i++) {
    vm_args.push_back(HandleExpr(args[i], code));
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, vm_args));

  return return_value_index;
}

std::size_t Generator::HandleClassFuncInvoke(FuncNode* func,
                                             std::vector<Bytecode>& code) {
  if (func == nullptr)
    EXIT_COMPILER(
        "Generator::HandleClassFuncInvoke(FuncNode*,std::vector<"
        "Bytecode>&)",
        "func is nullptr.");

  ExprNode* func_name_node = func->GetName();
  if (func_name_node == nullptr)
    EXIT_COMPILER(
        "Generator::HandleClassFuncInvoke(FuncNode*,std::vector<"
        "Bytecode>&)",
        "func_name_node is nullptr.");
  std::string func_name = static_cast<std::string>(*func_name_node);
  std::vector<ExprNode*> args = func->GetArgs();
  for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
    auto iterator = func_decl_map_.find(func_name);
    if (i != -1)
      iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
    if (iterator != func_decl_map_.end()) {
      func_name = func_name;
      if (i != -1) func_name = current_scope_[i] + "." + func_name;
      break;
    }
    if (i == -1)
      EXIT_COMPILER(
          "Generator::HandleClassFuncInvoke(FuncNode*,std::vector<"
          "Bytecode>&)",
          "Function not found.");
  }

  std::vector<std::size_t> vm_args;

  std::size_t func_name_index = global_memory_.AddString(func_name);

  vm_args.push_back(2);
  vm_args.push_back(func_name_index);
  vm_args.push_back(args.size() + 1);

  std::size_t return_value_index = global_memory_.Add(1);
  std::size_t return_value_ptr_index = global_memory_.Add(1);
  std::size_t return_value_reference_index = global_memory_.Add(1);
  code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, return_value_reference_index,
                          return_value_index));
  vm_args.push_back(return_value_reference_index);

  for (std::size_t i = 0; i < args.size(); i++) {
    vm_args.push_back(HandleExpr(args[i], code));
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, vm_args));

  return return_value_index;
}

void Generator::HandleLabel(LabelNode* label, std::vector<Bytecode>& code) {
  if (label == nullptr)
    EXIT_COMPILER("Generator::HandleLabel(LabelNode*,std::vector<Bytecode>&)",
                  "label is nullptr.");

  std::string label_name =
      current_scope_.back() + "$" + std::string(label->GetLabel());

  if (label_map_.find(label_name) != label_map_.end())
    EXIT_COMPILER("Generator::HandleLabel(LabelNode*,std::vector<Bytecode>&)",
                  "Has found same name label.");
  label_map_.emplace(label_name, code.size());
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
}

void Generator::HandleGoto(GotoNode* label, std::vector<Bytecode>& code) {
  if (label == nullptr)
    EXIT_COMPILER("Generator::HandleGoto(GotoNode*,std::vector<Bytecode>&)",
                  "label is nullptr.");

  std::string label_name = std::string(label->GetLabel());

  for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
    auto iterator = label_map_.find(current_scope_[i] + "$" + label_name);
    if (iterator != label_map_.end()) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                              global_memory_.AddUint64t(iterator->second)));
      return;
    }
    if (i == current_func_index_) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
      goto_map_.push_back(
          std::pair<std::string, std::size_t>(label_name, code.size() - 1));
      return;
    }
  }
}

void Generator::HandleStartGoto(GotoNode* label, std::vector<Bytecode>& code) {
  if (label == nullptr)
    EXIT_COMPILER(
        "Generator::HandleStartGoto(GotoNode*,std::vector<Bytecode>&)",
        "label is nullptr.");

  std::string label_name = std::string(label->GetLabel());

  for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
    auto iterator = label_map_.find(current_scope_[i] + "$" + label_name);
    if (iterator != label_map_.end()) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                              global_memory_.AddUint64t(iterator->second)));
      return;
    }
    if (i == current_func_index_) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
      start_goto_map_.push_back(
          std::pair<std::string, std::size_t>(label_name, code.size() - 1));
      return;
    }
  }
}

std::size_t Generator::GetClassIndex(ExprNode* expr,
                                     std::vector<Bytecode>& code) {
  if (expr == nullptr)
    EXIT_COMPILER("Generator::GetClassIndex(ExprNode*,std::vector<Bytecode>&)",
                  "expr is nullptr.");

  switch (expr->GetStatementType()) {
    case Ast::Statement::StatementType::kIdentifier: {
      std::size_t index = 0;
      if (current_class_ != nullptr &&
          current_class_->GetVar(
              (std::string)(*dynamic_cast<IdentifierNode*>(expr)), index)) {
        std::size_t return_index = global_memory_.Add(1);
        code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_index, 0,
                                global_memory_.AddString((std::string)(
                                    *dynamic_cast<IdentifierNode*>(expr)))));
        return return_index;
      }

      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(
            current_scope_[i] + "#" +
            static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
        if (iterator != var_decl_map_.end()) {
          return iterator->second.second;
        }
      }
      EXIT_COMPILER("Generator::GetClassIndex(ExprNode*)", "Not found.");
      break;
    }

    case Ast::Statement::StatementType::kValue: {
      std::size_t vm_type = dynamic_cast<ValueNode*>(expr)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = dynamic_cast<ValueNode*>(expr)->GetByteValue();
          return global_memory_.AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = dynamic_cast<ValueNode*>(expr)->GetLongValue();
          return global_memory_.AddLong(value);
        }

        case 0x03: {
          double value = dynamic_cast<ValueNode*>(expr)->GetDoubleValue();
          return global_memory_.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = dynamic_cast<ValueNode*>(expr)->GetUInt64Value();
          return global_memory_.AddUint64t(value);
        }

        case 0x05: {
          std::string value = dynamic_cast<ValueNode*>(expr)->GetStringValue();
          std::size_t str_index = global_memory_.AddString(value);
          return str_index;
        }

        default:
          EXIT_COMPILER(
              "Generator::GetClassIndex(ExprNode*,std::vector<Bytecode>"
              "&)",
              "Unexpected code.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunc:
      return HandleClassFuncInvoke(dynamic_cast<FuncNode*>(expr), code);

    default:
      return 0;
  }

  return 0;
}

std::size_t Generator::GetIndex(ExprNode* expr, std::vector<Bytecode>& code) {
  if (expr == nullptr)
    EXIT_COMPILER("Generator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
                  "expr is nullptr.");
  if (current_class_ != nullptr) {
    return GetClassIndex(expr, code);
  }

  switch (expr->GetStatementType()) {
    case Ast::Statement::StatementType::kIdentifier: {
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(
            current_scope_[i] + "#" +
            static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
        if (iterator != var_decl_map_.end()) {
          return iterator->second.second;
        }
      }
      EXIT_COMPILER("Generator::GetIndex(ExprNode*)", "Not found.");
      break;
    }

    case Ast::Statement::StatementType::kValue: {
      std::size_t vm_type = dynamic_cast<ValueNode*>(expr)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = dynamic_cast<ValueNode*>(expr)->GetByteValue();
          return global_memory_.AddByte(value);
          break;
        }

        case 0x02: {
          int64_t value = dynamic_cast<ValueNode*>(expr)->GetLongValue();
          return global_memory_.AddLong(value);
        }

        case 0x03: {
          double value = dynamic_cast<ValueNode*>(expr)->GetDoubleValue();
          return global_memory_.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = dynamic_cast<ValueNode*>(expr)->GetUInt64Value();
          return global_memory_.AddUint64t(value);
        }

        case 0x05: {
          std::string value = dynamic_cast<ValueNode*>(expr)->GetStringValue();
          std::size_t str_index = global_memory_.AddString(value);
          return str_index;
        }

        default:
          EXIT_COMPILER("Generator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
                        "Unexpected code.");
          break;
      }
    }

    case Ast::Statement::StatementType::kFunc:
      return HandleFuncInvoke(dynamic_cast<FuncNode*>(expr), code);

    default:
      return 0;
  }

  return 0;
}

std::size_t Generator::AddConstInt8t(int8_t value) {
  return global_memory_.AddByte(value);
}

int64_t Generator::SwapLong(int64_t x) {
  uint64_t ux = (uint64_t)x;
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  return (int64_t)ux;
}

double Generator::SwapDouble(double x) {
  uint64_t ux;
  memcpy(&ux, &x, sizeof(uint64_t));
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  double result;
  memcpy(&result, &ux, sizeof(double));
  return result;
}
uint64_t Generator::SwapUint64t(uint64_t x) {
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}

void Generator::InsertUint64ToCode(uint64_t value) {
  for (int i = 0; i < 8; ++i) {
    code_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }
}

std::size_t Generator::EncodeUleb128(std::size_t value,
                                     std::vector<uint8_t>& output) {
  std::size_t count = 0;
  do {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if (value != 0) {
      byte |= 0x80;
    }
    output.push_back(byte);
    count++;
  } while (value != 0);
  return count;
}

Type* Generator::GetExprType(ExprNode* expr) {
  if (expr == Ast::Statement::StatementType::kArray) {
    bool is_find = false;
    auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
    for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
      iterator = var_decl_map_.find(
          current_scope_[i] + "#" +
          static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
      if (iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Not found array.");

    ArrayDeclNode* array_decl = (ArrayDeclNode*)iterator->second.second;
    if (array_decl->GetVarType() == Type::TypeType::kArray) {
      return dynamic_cast<ArrayType*>(array_decl->GetVarType())->GetSubType();
    } else if (array_decl->GetVarType() == Type::TypeType::kPointer) {
      return dynamic_cast<PointerType*>(array_decl->GetVarType())->GetSubType();
    } else {
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
    }
  } else if (expr == Ast::Statement::StatementType::kArrayDecl) {
    return dynamic_cast<ArrayDeclNode*>(expr)->GetVarType();
  } else if (expr == Ast::Statement::StatementType::kIdentifier) {
    bool is_find = false;
    auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
    for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
      iterator = var_decl_map_.find(
          current_scope_[i] + "#" +
          static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
      if (iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Not found variable.");

    return iterator->second.first->GetVarType();
  } else if (expr == Ast::Statement::StatementType::kUnary) {
    switch (dynamic_cast<UnaryNode*>(expr)->GetOperator()) {
      case UnaryNode::Operator::kPostInc:
      case UnaryNode::Operator::kPostDec:
      case UnaryNode::Operator::kPreInc:
      case UnaryNode::Operator::kPreDec:
      case UnaryNode::Operator::kPlus:
      case UnaryNode::Operator::kMinus:
      case UnaryNode::Operator::kNot:
      case UnaryNode::Operator::kBitwiseNot:
        return GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
      case UnaryNode::Operator::kAddrOf: {
        PointerType* ptr = new PointerType();
        ptr->SetSubType(GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()));
        return ptr;
      }
      case UnaryNode::Operator::kDeref:
        if (GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()) ==
            Type::TypeType::kPointer) {
          return dynamic_cast<PointerType*>(
                     GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()))
              ->GetSubType();
        } else if (GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()) ==
                   Type::TypeType::kArray) {
          return dynamic_cast<ArrayType*>(
                     GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()))
              ->GetSubType();
        } else {
          EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
        }
      case UnaryNode::Operator::ARRAY:
        EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unexpected code.");
      default:
        EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unexpected code.");
    }
  } else if (expr == Ast::Statement::StatementType::kBinary) {
    Type* left = GetExprType(dynamic_cast<BinaryNode*>(expr)->GetLeftExpr());
    Type* right = GetExprType(dynamic_cast<BinaryNode*>(expr)->GetRightExpr());

    if (dynamic_cast<BinaryNode*>(expr)->GetOperator() ==
        BinaryNode::Operator::kDiv) {
      return Type::CreateDoubleType();
    }

    if (left == Type::TypeType::kConst)
      left = dynamic_cast<ConstType*>(left)->GetSubType();
    if (right == Type::TypeType::kConst)
      right = dynamic_cast<ConstType*>(right)->GetSubType();
    if (left == Type::TypeType::kReference)
      left = dynamic_cast<ReferenceType*>(left)->GetSubType();
    if (right == Type::TypeType::kReference)
      right = dynamic_cast<ReferenceType*>(right)->GetSubType();
    if (left == Type::TypeType::NONE || right == Type::TypeType::NONE)
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown Type.");

    if (left == Type::TypeType::kPointer || right == Type::TypeType::kPointer ||
        left == Type::TypeType::kArray || right == Type::TypeType::kArray) {
      if (left == Type::TypeType::kPointer || left == Type::TypeType::kArray)
        return left;
      if (right == Type::TypeType::kPointer || right == Type::TypeType::kArray)
        return right;
    }

    if (left == Type::TypeType::kClass) return left;
    if (right == Type::TypeType::kClass) return right;

    if (left->GetBaseType() == right->GetBaseType()) return left;

    if (left->GetSize() > right->GetSize()) {
      return left;
    } else if (left->GetSize() < right->GetSize()) {
      return right;
    } else {
      int left_priority = 0;
      int right_priority = 0;
      switch (left->GetBaseType()) {
        case Type::BaseType::kVoid:
          left_priority = 0;
          break;
        case Type::BaseType::kBool:
          left_priority = 1;
          break;
        case Type::BaseType::kChar:
          left_priority = 2;
          break;
        case Type::BaseType::kShort:
          left_priority = 3;
          break;
        case Type::BaseType::kInt:
          left_priority = 4;
          break;
        case Type::BaseType::kFloat:
          left_priority = 5;
          break;
        case Type::BaseType::kLong:
          left_priority = 6;
          break;
        case Type::BaseType::kDouble:
          left_priority = 7;
          break;
        case Type::BaseType::kString:
          left_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          left_priority = 9;
          break;
        case Type::BaseType::kPointer:
          left_priority = 10;
          break;
        case Type::BaseType::kArray:
          left_priority = 11;
          break;
        case Type::BaseType::kEnum:
          left_priority = 12;
          break;
        case Type::BaseType::kUnion:
          left_priority = 13;
          break;
        case Type::BaseType::kStruct:
          left_priority = 14;
          break;
        case Type::BaseType::kClass:
          left_priority = 15;
          break;
        case Type::BaseType::kFunction:
          left_priority = 16;
          break;
        case Type::BaseType::kAuto:
          left_priority = 17;
          break;
        default:
          EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
      }
      switch (right->GetBaseType()) {
        case Type::BaseType::kVoid:
          right_priority = 0;
          break;
        case Type::BaseType::kBool:
          right_priority = 1;
          break;
        case Type::BaseType::kChar:
          right_priority = 2;
          break;
        case Type::BaseType::kShort:
          right_priority = 3;
          break;
        case Type::BaseType::kInt:
          right_priority = 4;
          break;
        case Type::BaseType::kFloat:
          right_priority = 5;
          break;
        case Type::BaseType::kLong:
          right_priority = 6;
          break;
        case Type::BaseType::kDouble:
          right_priority = 7;
          break;
        case Type::BaseType::kString:
          right_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          right_priority = 9;
          break;
        case Type::BaseType::kPointer:
          right_priority = 10;
          break;
        case Type::BaseType::kArray:
          right_priority = 11;
          break;
        case Type::BaseType::kEnum:
          right_priority = 12;
          break;
        case Type::BaseType::kUnion:
          right_priority = 13;
          break;
        case Type::BaseType::kStruct:
          right_priority = 14;
          break;
        case Type::BaseType::kClass:
          right_priority = 15;
          break;
        case Type::BaseType::kFunction:
          right_priority = 16;
          break;
        case Type::BaseType::kAuto:
          right_priority = 17;
          break;
        default:
          EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
      }
      if (left_priority > right_priority) return left;
      if (left_priority < right_priority) return right;
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unexpected code.");
    }

  } else if (expr == Ast::Statement::StatementType::kFunc) {
    return dynamic_cast<Ast::Function*>(expr)->GetReturnType();
  } else if (expr == Ast::Statement::StatementType::kVarDecl) {
    return dynamic_cast<VarDeclNode*>(expr)->GetVarType();
  } else if (expr == Ast::Statement::StatementType::kValue) {
    return dynamic_cast<ValueNode*>(expr)->GetValueType();
  } else if (expr == Ast::Statement::StatementType::kConditional) {
    Type* true_expr =
        GetExprType(dynamic_cast<ConditionalNode*>(expr)->GetTrueExpr());
    Type* false_expr =
        GetExprType(dynamic_cast<ConditionalNode*>(expr)->GetFalseExpr());

    if (true_expr == Type::TypeType::kConst)
      true_expr = dynamic_cast<ConstType*>(true_expr)->GetSubType();
    if (false_expr == Type::TypeType::kConst)
      false_expr = dynamic_cast<ConstType*>(false_expr)->GetSubType();
    if (true_expr == Type::TypeType::kReference)
      true_expr = dynamic_cast<ReferenceType*>(true_expr)->GetSubType();
    if (false_expr == Type::TypeType::kReference)
      false_expr = dynamic_cast<ReferenceType*>(false_expr)->GetSubType();
    if (true_expr == Type::TypeType::NONE || false_expr == Type::TypeType::NONE)
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown Type.");

    if (true_expr == Type::TypeType::kPointer ||
        false_expr == Type::TypeType::kPointer ||
        true_expr == Type::TypeType::kArray ||
        false_expr == Type::TypeType::kArray) {
      if (true_expr == Type::TypeType::kPointer ||
          true_expr == Type::TypeType::kArray)
        return true_expr;
      if (false_expr == Type::TypeType::kPointer ||
          false_expr == Type::TypeType::kArray)
        return false_expr;
    }

    if (true_expr == Type::TypeType::kClass) return true_expr;
    if (false_expr == Type::TypeType::kClass) return false_expr;

    if (true_expr->GetBaseType() == false_expr->GetBaseType()) return true_expr;

    if (true_expr->GetSize() > false_expr->GetSize()) {
      return true_expr;
    } else if (true_expr->GetSize() < false_expr->GetSize()) {
      return false_expr;
    } else {
      int true_expr_priority = 0;
      int false_expr_priority = 0;
      switch (true_expr->GetBaseType()) {
        case Type::BaseType::kVoid:
          true_expr_priority = 0;
          break;
        case Type::BaseType::kBool:
          true_expr_priority = 1;
          break;
        case Type::BaseType::kChar:
          true_expr_priority = 2;
          break;
        case Type::BaseType::kShort:
          true_expr_priority = 3;
          break;
        case Type::BaseType::kInt:
          true_expr_priority = 4;
          break;
        case Type::BaseType::kFloat:
          true_expr_priority = 5;
          break;
        case Type::BaseType::kLong:
          true_expr_priority = 6;
          break;
        case Type::BaseType::kDouble:
          true_expr_priority = 7;
          break;
        case Type::BaseType::kString:
          true_expr_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          true_expr_priority = 9;
          break;
        case Type::BaseType::kPointer:
          true_expr_priority = 10;
          break;
        case Type::BaseType::kArray:
          true_expr_priority = 11;
          break;
        case Type::BaseType::kEnum:
          true_expr_priority = 12;
          break;
        case Type::BaseType::kUnion:
          true_expr_priority = 13;
          break;
        case Type::BaseType::kStruct:
          true_expr_priority = 14;
          break;
        case Type::BaseType::kClass:
          true_expr_priority = 15;
          break;
        case Type::BaseType::kFunction:
          true_expr_priority = 16;
          break;
        case Type::BaseType::kAuto:
          true_expr_priority = 17;
          break;
        default:
          EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
      }
      switch (false_expr->GetBaseType()) {
        case Type::BaseType::kVoid:
          false_expr_priority = 0;
          break;
        case Type::BaseType::kBool:
          false_expr_priority = 1;
          break;
        case Type::BaseType::kChar:
          false_expr_priority = 2;
          break;
        case Type::BaseType::kShort:
          false_expr_priority = 3;
          break;
        case Type::BaseType::kInt:
          false_expr_priority = 4;
          break;
        case Type::BaseType::kFloat:
          false_expr_priority = 5;
          break;
        case Type::BaseType::kLong:
          false_expr_priority = 6;
          break;
        case Type::BaseType::kDouble:
          false_expr_priority = 7;
          break;
        case Type::BaseType::kString:
          false_expr_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          false_expr_priority = 7;
          break;
        case Type::BaseType::kPointer:
          false_expr_priority = 10;
          break;
        case Type::BaseType::kArray:
          false_expr_priority = 11;
          break;
        case Type::BaseType::kEnum:
          false_expr_priority = 12;
          break;
        case Type::BaseType::kUnion:
          false_expr_priority = 13;
          break;
        case Type::BaseType::kStruct:
          false_expr_priority = 14;
          break;
        case Type::BaseType::kClass:
          false_expr_priority = 15;
          break;
        case Type::BaseType::kFunction:
          false_expr_priority = 16;
          break;
        case Type::BaseType::kAuto:
          false_expr_priority = 17;
          break;
        default:
          EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unknown type.");
      }
      if (true_expr_priority > false_expr_priority) return true_expr;
      if (true_expr_priority < false_expr_priority) return false_expr;
      EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unexpected code.");
    }
  } else {
    EXIT_COMPILER("Generator::GetExprType(ExprNode*)", "Unsupport type.");
  }
  return nullptr;
}
std::string Generator::GetExprTypeString(ExprNode* expr) {
  Type* type = GetExprType(expr);
  if (type == nullptr)
    EXIT_COMPILER("Generator::GetExprTypeString(ExprNode*)",
                  "type is nullptr.");
  if (type == Type::TypeType::NONE)
    EXIT_COMPILER("Generator::GetExprTypeString(ExprNode*)", "Unknown type.");
  if (type == Type::TypeType::kConst) return *dynamic_cast<ConstType*>(type);
  if (type == Type::TypeType::kPointer)
    return *dynamic_cast<PointerType*>(type);
  if (type == Type::TypeType::kArray) return *dynamic_cast<ArrayType*>(type);
  if (type == Type::TypeType::kReference)
    return *dynamic_cast<ReferenceType*>(type);
  if (type == Type::TypeType::kClass) return *dynamic_cast<ClassType*>(type);
  if (type == Type::TypeType::kBase) return *type;

  EXIT_COMPILER("Generator::GetExprTypeString(ExprNode*)", "Unexpected code.");

  return std::string();
}

bool Generator::IsDereferenced(ExprNode* expr) {
  if (expr == Ast::Statement::StatementType::kUnary) {
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kDeref) {
      return true;
    }
  } else if (expr == Ast::Statement::StatementType::kArray) {
    return true;
  }
  return false;
}
}  // namespace Generator
}  // namespace Compiler
}  // namespace Aq