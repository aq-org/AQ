// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/bytecode/bytecode.h"

#include <cstdint>
#include <memory>
#include <string>

#include "vm/bytecode/bytecode.h"
#include "vm/logging/logging.h"
#include "vm/memory/memory.h"
#include "vm/operator/operator.h"
#include "vm/utils/utils.h"
#include "vm/vm.h"

namespace Aq {
namespace Vm {
namespace Bytecode {
char* Get1Parament(char* ptr, size_t* first) {
  ptr += DecodeUleb128((uint8_t*)ptr, first);
  return ptr;
}

char* Get2Parament(char* ptr, size_t* first) {
  ptr += DecodeUleb128((uint8_t*)ptr, first);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 1);
  return ptr;
}

char* Get3Parament(char* ptr, size_t* first) {
  ptr += DecodeUleb128((uint8_t*)ptr, first);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 1);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 2);
  return ptr;
}

char* Get4Parament(char* ptr, size_t* first) {
  ptr += DecodeUleb128((uint8_t*)ptr, first);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 1);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 2);
  ptr += DecodeUleb128((uint8_t*)ptr, first + 3);
  return ptr;
}

std::vector<std::size_t> GetUnknownCountParament(char*& ptr) {
  std::vector<std::size_t> arguments;

  std::size_t function_name = 0;
  std::size_t arguments_size = 0;
  ptr += DecodeUleb128((uint8_t*)ptr, &function_name);
  ptr += DecodeUleb128((uint8_t*)ptr, &arguments_size);

  arguments.resize(arguments_size + 1);
  arguments[0] = function_name;

  for (size_t i = 1; i < arguments_size + 1; i++) {
    ptr += DecodeUleb128((uint8_t*)ptr, arguments.data() + i);
  }

  return arguments;
}

std::vector<std::size_t> GetUnknownCountParamentForClass(char*& ptr) {
  std::vector<std::size_t> arguments;

  std::size_t class_index = 0;
  std::size_t function_name = 0;
  std::size_t arguments_size = 0;
  ptr += DecodeUleb128((uint8_t*)ptr, &class_index);
  ptr += DecodeUleb128((uint8_t*)ptr, &function_name);
  ptr += DecodeUleb128((uint8_t*)ptr, &arguments_size);

  arguments.resize(arguments_size + 2);
  arguments[0] = class_index;
  arguments[1] = function_name;

  for (size_t i = 2; i < arguments_size + 2; i++) {
    ptr += DecodeUleb128((uint8_t*)ptr, arguments.data() + i);
  }

  return arguments;
}

char* AddClassMethod(char* location,
                     std::unordered_map<std::string, Function>& functions) {
  if (*location == '.') location += 1;

  // Gets the function name.
  std::string function_name(location);
  while (*location != '\0') location += 1;
  location += 1;

  LOGGING_INFO(function_name);
  // Gets the context of the function.
  auto& function = functions[function_name];
  auto& instructions = function.instructions;

  // Sets the variadic flag.
  if (*(uint8_t*)location == 0xFF) {
    location += 1;
    function.is_variadic = true;
  } else {
    function.is_variadic = false;
  }

  LOGGING_INFO("DEUBG");
  // Sets the arguments.
  std::size_t arguments_size = 0;
  location += DecodeUleb128((uint8_t*)location, &arguments_size);
  LOGGING_INFO(std::to_string(arguments_size));
  function.arguments.resize(arguments_size);
  for (size_t i = 0; i < arguments_size; i++)
    location += DecodeUleb128((uint8_t*)location, &function.arguments[i]);

  LOGGING_INFO("DEUBG");

  // Sets the instructions.
  std::size_t instructions_size = 0;
  location += DecodeUleb128((uint8_t*)location, &instructions_size);
  instructions.resize(instructions_size);

  LOGGING_INFO("DEUBG");
  for (size_t i = 0; i < instructions_size; i++) {
    LOGGING_INFO("0000");
    instructions[i].oper = static_cast<Operator::Operator>(*(uint8_t*)location);
    location += 1;
    switch (instructions[i].oper) {
      case Operator::Operator::NOP:
        break;

      case Operator::Operator::LOAD:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::STORE:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::NEW:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::ARRAY:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::PTR:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::ADD:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::SUB:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::MUL:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::DIV:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::REM:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::NEG:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::SHL:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::SHR:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::REFER:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::AND:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::OR:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::XOR:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::IF:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::CMP:
        instructions[i].arguments.resize(4);
        location = Get4Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::GOTO:
        instructions[i].arguments.resize(1);
        location = Get1Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::INVOKE:
        instructions[i].arguments = GetUnknownCountParament(location);
        break;

      case Operator::Operator::EQUAL:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::LOAD_CONST:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::CONVERT:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::CONST:
        instructions[i].arguments.resize(2);
        location = Get2Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::INVOKE_METHOD:
        instructions[i].arguments = GetUnknownCountParamentForClass(location);
        break;

      case Operator::Operator::LOAD_MEMBER:
        instructions[i].arguments.resize(3);
        location = Get3Parament(location, instructions[i].arguments.data());
        break;

      case Operator::Operator::WIDE:
        break;

      default:
        LOGGING_ERROR("Invalid operator. " +
                      std::to_string(static_cast<int>(instructions[i].oper)));
    }
  }

  return location;
}

char* AddClass(char* location,
               std::unordered_map<std::string, Bytecode::Class> classes,
               std::shared_ptr<Memory::Memory> memory) {
  // Gets the class name.
  std::string class_name(location);
  while (*location != '\0') location += 1;
  location += 1;

  // Gets the members size.
  std::size_t members_size = 0;
  location += DecodeUleb128((uint8_t*)location, &members_size);
  classes[class_name].members.resize(members_size);

  // Sets the members type.
  for (size_t i = 0; i < members_size; i++) {
    // Gets the member name.
    const char* name_location = location;
    while (*location != '\0') location += 1;
    location += 1;
    std::string member_name(location, location - name_location);
    classes[class_name].variables[member_name] = i;

    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)location) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x09:
        case 0x0A:
          classes[class_name].members[i].type.push_back(*(uint8_t*)location);
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          classes[class_name].members[i].type.push_back(*(uint8_t*)location);
          location += 1;
          break;

        default:
          LOGGING_ERROR("Unsupported data type " +
                        std::to_string(*(uint8_t*)location));
          break;
      }
    }
    if (classes[class_name].members[i].type[0] != 0x00)
      classes[class_name].members[i].const_type = true;
    location += 1;
  }

  // Gets the methods size.
  std::size_t methods_size = 0;
  location += DecodeUleb128((uint8_t*)location, &methods_size);

  // Sets the class methods.
  for (size_t i = 0; i < methods_size; i++)
    location = AddClassMethod(location, classes[class_name].functions);

  // Sets the memory.
  classes[class_name].memory = memory;

  return location;
}

int InvokeClassFunction(
    std::vector<Memory::Object>& heap, size_t class_index,
    std::string function_name, std::vector<size_t> arguments,
    std::string& current_bytecode_file,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::shared_ptr<Memory::Memory>& memory,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>
        builtin_functions,
    bool is_big_endian) {
  auto class_name_object = GetObjectData(heap, class_index);
  if (class_name_object.empty() || class_name_object[0].type[0] != 0x05)
    LOGGING_ERROR("Invalid class name object.");

  // Retrieves the uninitialized class name and wait for the next step of
  // processing.
  std::string class_name = std::get<std::string>(class_name_object[0].data);

  // Records the old current bytecode file to recover the status after the
  // processing.
  std::string old_current_bytecode_file = current_bytecode_file;

  // If the class name starts with '~', it indicates that the class is
  // defined in other bytecode file.
  if (class_name[0] == '~') {
    std::string file_name = class_name;
    std::size_t start_pos = file_name.find('~');
    std::size_t end_pos = file_name.find('~', start_pos + 1);

    // Gets the class name and file name.
    if (start_pos != std::string::npos && end_pos != std::string::npos &&
        start_pos < end_pos) {
      class_name = file_name.substr(end_pos + 1);
      file_name = file_name.substr(start_pos + 1, end_pos - start_pos - 1);
    } else {
      INTERNAL_ERROR("Bytecode file name is invalid.");
    }

    current_bytecode_file = file_name;

  } else {
    // If the class name without a bytecode file name, we assume it is a class
    // from current bytecode file.
    current_bytecode_file = "";
  }

  if (classes.find(class_name) == classes.end())
    LOGGING_ERROR("Class not found: " + class_name);

  if (classes[class_name].functions.find(function_name) ==
      classes[class_name].functions.end())
    LOGGING_ERROR("Function not found: " + function_name);

  auto function = classes[class_name].functions[function_name];

  if (arguments.size() < function.arguments.size())
    LOGGING_ERROR("Invalid args_size.");

  // Sets the variadic information.
  if (function.is_variadic) {
    std::vector<Memory::Object> variadic_list;
    for (size_t i = 0; i < arguments.size() - function.arguments.size() + 1;
         i++) {
      // Copys data from arguments to target.
      variadic_list[i].type =
          heap[arguments[function.arguments.size() + i - 2]].type;
      variadic_list[i].data =
          heap[arguments[function.arguments.size() + i - 2]].data;
    }

    // Sets the target.
    classes[class_name].memory->heap[function.arguments.back()].type.push_back(
        0x06);
    classes[class_name].memory->heap[function.arguments.back()].data =
        variadic_list;
  }

  // Processes the arguments including the return value (at the arguments index
  // 0).
  auto& class_heap = classes[class_name].memory->heap;
  for (size_t i = 0; i < function.arguments.size(); i++) {
    // Handles the argument which is the reference of non-const variable.
    if (class_heap[function.arguments[i]].const_type &&
        class_heap[function.arguments[i]].type[0] == 0x07 &&
        class_heap[function.arguments[i]].type[1] != 0x08) {
      class_heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

      // Handles the argument which is the reference of const variable.
    } else if (class_heap[function.arguments[i]].const_type &&
               class_heap[function.arguments[i]].type[0] == 0x07 &&
               class_heap[function.arguments[i]].type[1] == 0x08) {
      // Deletes the const type from the type vector and sets the data.
      class_heap[function.arguments[i]].type.erase(
          class_heap[function.arguments[i]].type.begin(),
          class_heap[function.arguments[i]].type.begin() + 1);

      class_heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

      // Handles the argument which is the const object.
    } else if (class_heap[function.arguments[i]].const_type &&
               class_heap[function.arguments[i]].type[0] == 0x08) {
      class_heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

    } else {
      // Handles other arguments which are not const or reference.
      Operator::CrossMemoryEqual(classes[class_name].memory->heap,
                                 function.arguments[i], heap, arguments[i]);
    }
  }

  // Stores the old heap and sets the new heap for the class.
  auto& old_heap = heap;
  heap = classes[class_name].memory->heap;
  auto& constant_pool = classes[class_name].memory->constant_pool;

  auto run_code = function.instructions;
  for (size_t i = 0; i < function.instructions.size(); i++) {
    switch (run_code[i].oper) {
      case Operator::Operator::NOP:
        Operator::NOP();
        break;
      case Operator::Operator::LOAD:
        Operator::LOAD(heap, run_code[i].arguments[0],
                       run_code[i].arguments[1]);
        break;
      case Operator::Operator::STORE:
        Operator::STORE(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::NEW:
        Operator::NEW(heap, bytecode_files, current_bytecode_file, classes,
                      run_code[i].arguments[0], run_code[i].arguments[1],
                      is_big_endian, run_code[i].arguments[2], memory,
                      builtin_functions);
        break;
      case Operator::Operator::ARRAY:
        Operator::ARRAY(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1], run_code[i].arguments[2],
                        classes, bytecode_files, builtin_functions,
                        current_bytecode_file, is_big_endian, memory);
        break;
      case Operator::Operator::PTR:
        Operator::PTR(heap, run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case Operator::Operator::ADD:
        Operator::ADD(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::SUB:
        Operator::SUB(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::MUL:
        Operator::MUL(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::DIV:
        Operator::DIV(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::REM:
        Operator::REM(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::NEG:
        Operator::NEG(heap, run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case Operator::Operator::SHL:
        Operator::SHL(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::SHR:
        Operator::SHR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::REFER:
        Operator::REFER(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::IF:
        i = Operator::IF(heap, run_code[i].arguments[0],
                         run_code[i].arguments[1], run_code[i].arguments[2]);
        i--;
        break;
      case Operator::Operator::AND:
        Operator::AND(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::OR:
        Operator::OR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                     run_code[i].arguments[2]);
        break;
      case Operator::Operator::XOR:
        Operator::XOR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::CMP:
        Operator::CMP(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2], run_code[i].arguments[3]);
        break;
      case Operator::Operator::INVOKE:
        Operator::INVOKE(heap, builtin_functions, run_code[i].arguments,
                         classes, bytecode_files, current_bytecode_file,
                         is_big_endian, memory);
        break;
      case Operator::Operator::EQUAL:
        Operator::EQUAL(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::GOTO:
        i = Operator::GOTO(heap, run_code[i].arguments[0]);
        i--;
        break;
      case Operator::Operator::LOAD_CONST:
        Operator::LOAD_CONST(heap, constant_pool, run_code[i].arguments[0],
                             run_code[i].arguments[1]);
        break;
      case Operator::Operator::CONVERT:
        Operator::CONVERT(heap, run_code[i].arguments[0],
                          run_code[i].arguments[1]);
        break;
      case Operator::Operator::CONST:
        Operator::_CONST(heap, run_code[i].arguments[0],
                         run_code[i].arguments[1]);
        break;
      case Operator::Operator::INVOKE_METHOD:
        Operator::INVOKE_METHOD(heap, current_bytecode_file, classes, memory,
                                bytecode_files, builtin_functions,
                                is_big_endian, run_code[i].arguments);
        break;
      case Operator::Operator::LOAD_MEMBER:
        if (run_code[i].arguments[1] == 0) {
          Operator::LOAD_MEMBER(heap, classes, run_code[i].arguments[0],
                                class_index, run_code[i].arguments[2]);
        } else {
          Operator::LOAD_MEMBER(heap, classes, run_code[i].arguments[0],
                                run_code[i].arguments[1],
                                run_code[i].arguments[2]);
        }
        break;
      case Operator::Operator::WIDE:
        Operator::WIDE();
        break;
      default:
        LOGGING_ERROR("Invalid operator.");
        break;
    }
  }

  // Recovers the old heap and the current bytecode file.
  heap = old_heap;
  current_bytecode_file = old_current_bytecode_file;

  return 0;
}

int InvokeCustomFunction(
    std::vector<Memory::Object>& heap, std::string name,
    std::vector<size_t> arguments,
    std::unordered_map<std::string, Bytecode::Class>& classes,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    std::unordered_map<std::string,
                       std::function<int(std::vector<std::size_t>)>>
        builtin_functions,
    std::string& current_bytecode_file, bool is_big_endian,
    std::shared_ptr<Memory::Memory>& memory) {
  if (classes.find(".!__start") == classes.end())
    INTERNAL_ERROR("Unexpected error. Not found main class.");

  // Gets the main class functions.
  auto& functions = classes[".!__start"].functions;

  // Gets the custom function.
  if (functions.find(name) == functions.end())
    LOGGING_ERROR("Function not found: " + name);
  auto function = functions[name];

  if (arguments.size() < function.arguments.size())
    LOGGING_ERROR("Invalid args_size.");

  // Sets the variadic information.
  if (function.is_variadic) {
    std::vector<Memory::Object> variadic_list;
    for (size_t i = 0; i < arguments.size() - function.arguments.size() + 1;
         i++) {
      // Copys data from arguments to target.
      variadic_list[i].type =
          heap[arguments[function.arguments.size() + i - 2]].type;
      variadic_list[i].data =
          heap[arguments[function.arguments.size() + i - 2]].data;
    }

    // Sets the target.
    heap[function.arguments.back()].type.push_back(0x06);
    heap[function.arguments.back()].data = variadic_list;
  }

  // Processes the arguments including the return value (at the arguments index
  // 0).
  for (size_t i = 0; i < function.arguments.size(); i++) {
    // Handles the argument which is the reference of non-const variable.
    if (heap[function.arguments[i]].const_type &&
        heap[function.arguments[i]].type[0] == 0x07 &&
        heap[function.arguments[i]].type[1] != 0x08) {
      heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

      // Handles the argument which is the reference of const variable.
    } else if (heap[function.arguments[i]].const_type &&
               heap[function.arguments[i]].type[0] == 0x07 &&
               heap[function.arguments[i]].type[1] == 0x08) {
      // Deletes the const type from the type vector and sets the data.
      heap[function.arguments[i]].type.erase(
          heap[function.arguments[i]].type.begin(),
          heap[function.arguments[i]].type.begin() + 1);

      heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

      // Handles the argument which is the const object.
    } else if (heap[function.arguments[i]].const_type &&
               heap[function.arguments[i]].type[0] == 0x08) {
      heap[function.arguments[i]].data =
          std::shared_ptr<Memory::Object>(&heap[arguments[i]], [](void*) {});

    } else {
      // Handles other arguments which are not const or reference.
      Operator::EQUAL(heap, function.arguments[i], arguments[i]);
    }
  }

  auto& constant_pool = classes[".!__start"].memory->constant_pool;

  auto run_code = function.instructions;
  for (size_t i = 0; i < function.instructions.size(); i++) {
    switch (run_code[i].oper) {
      case Operator::Operator::NOP:
        Operator::NOP();
        break;
      case Operator::Operator::LOAD:
        Operator::LOAD(heap, run_code[i].arguments[0],
                       run_code[i].arguments[1]);
        break;
      case Operator::Operator::STORE:
        Operator::STORE(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::NEW:
        Operator::NEW(heap, bytecode_files, current_bytecode_file, classes,
                      is_big_endian, run_code[i].arguments[0],
                      run_code[i].arguments[1], run_code[i].arguments[2],
                      memory, builtin_functions);
        break;
      case Operator::Operator::ARRAY:
        Operator::ARRAY(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1], run_code[i].arguments[2],
                        classes, bytecode_files, builtin_functions,
                        current_bytecode_file, is_big_endian, memory);
        break;
      case Operator::Operator::PTR:
        Operator::PTR(heap, run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case Operator::Operator::ADD:
        Operator::ADD(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::SUB:
        Operator::SUB(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::MUL:
        Operator::MUL(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::DIV:
        Operator::DIV(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::REM:
        Operator::REM(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::NEG:
        Operator::NEG(heap, run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case Operator::Operator::SHL:
        Operator::SHL(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::SHR:
        Operator::SHR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::REFER:
        Operator::REFER(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::IF:
        i = Operator::IF(heap, run_code[i].arguments[0],
                         run_code[i].arguments[1], run_code[i].arguments[2]);
        i--;
        break;
      case Operator::Operator::AND:
        Operator::AND(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::OR:
        Operator::OR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                     run_code[i].arguments[2]);
        break;
      case Operator::Operator::XOR:
        Operator::XOR(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::CMP:
        Operator::CMP(heap, run_code[i].arguments[0], run_code[i].arguments[1],
                      run_code[i].arguments[2], run_code[i].arguments[3]);
        break;
      case Operator::Operator::INVOKE:
        Operator::INVOKE(heap, builtin_functions, run_code[i].arguments,
                         classes, bytecode_files, current_bytecode_file,
                         is_big_endian, memory);
        break;
      case Operator::Operator::EQUAL:
        Operator::EQUAL(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1]);
        break;
      case Operator::Operator::GOTO:
        i = Operator::GOTO(heap, run_code[i].arguments[0]);
        i--;
        break;
      case Operator::Operator::LOAD_CONST:
        Operator::LOAD_CONST(heap, constant_pool, run_code[i].arguments[0],
                             run_code[i].arguments[1]);
        break;
      case Operator::Operator::CONVERT:
        Operator::CONVERT(heap, run_code[i].arguments[0],
                          run_code[i].arguments[1]);
        break;
      case Operator::Operator::CONST:
        Operator::_CONST(heap, run_code[i].arguments[0],
                         run_code[i].arguments[1]);
        break;
      case Operator::Operator::INVOKE_METHOD:
        Operator::INVOKE_METHOD(heap, current_bytecode_file, classes, memory,
                                bytecode_files, builtin_functions,
                                is_big_endian, run_code[i].arguments);
        break;
      case Operator::Operator::LOAD_MEMBER:
        if (run_code[i].arguments[1] == 0) {
          Operator::LOAD_MEMBER(heap, classes, run_code[i].arguments[0], 2,
                                run_code[i].arguments[2]);
        } else {
          Operator::LOAD_MEMBER(heap, classes, run_code[i].arguments[0],
                                run_code[i].arguments[1],
                                run_code[i].arguments[2]);
        }
        break;
      case Operator::Operator::WIDE:
        Operator::WIDE();
        break;
      default:
        LOGGING_ERROR("Invalid operator.");
        break;
    }
  }

  return 0;
}

char* AddBytecodeFileClass(
    std::string prefix_name, std::shared_ptr<Memory::Memory> memory,
    char* location, std::unordered_map<std::string, Bytecode::Class>& classes) {
  std::string class_name = prefix_name + std::string(location);

  // Skips the class name.
  while (*location != '\0') location += 1;
  location += 1;

  // Gets the members size.
  std::size_t members_size = 0;
  location += DecodeUleb128((uint8_t*)location, &members_size);
  classes[class_name].members.resize(members_size);

  // Handles the members.
  classes[class_name].members.resize(members_size);
  for (size_t i = 0; i < members_size; i++) {
    std::string member_name(location);

    // Skips the member name.
    while (*location != '\0') location += 1;
    location += 1;

    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)location) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x09:
        case 0x0A:
          classes[class_name].members[i].type.push_back(*(uint8_t*)location);
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          classes[class_name].members[i].type.push_back(*(uint8_t*)location);
          location += 1;
          break;

        default:
          LOGGING_ERROR("Unsupported data type.");
          break;
      }
    }
    if (classes[class_name].members[i].type[0] != 0x00)
      classes[class_name].members[i].const_type = true;
    location += 1;
  }

  // Gets the methods size.
  std::size_t methods_size = 0;
  location += DecodeUleb128((uint8_t*)location, &methods_size);

  for (size_t i = 0; i < methods_size; i++) {
    location = AddClassMethod(location, classes[class_name].functions);
  }

  classes[class_name].memory = memory;

  return location;
}

char* HandleBytecodeFile(
    std::string name, char* bytecode_file, size_t size, bool is_big_endian,
    std::unordered_map<std::string, Bytecode::Class>& classes) {
  if (bytecode_file[0] != 0x41 || bytecode_file[1] != 0x51 ||
      bytecode_file[2] != 0x42 || bytecode_file[3] != 0x43)
    LOGGING_ERROR("Invalid bytecode file.");

  if (bytecode_file[4] != 0x00 || bytecode_file[5] != 0x00 ||
      bytecode_file[6] != 0x00 || bytecode_file[7] != 0x03)
    LOGGING_ERROR(
        "This bytecode version is not supported, please check for updates.");

  char* bytecode_end = bytecode_file + size;

  // Skips the magic code and version.
  bytecode_file += 8;

  auto memory = std::make_shared<Memory::Memory>();

  // Gets the constant pool size.
  std::size_t constant_pool_size = 0;
  bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &constant_pool_size);

  // Handles the constant pool.
  for (size_t i = 0; i < constant_pool_size; i++) {
    // Sets the type of the constant.
    memory->constant_pool[i].type.push_back(*bytecode_file);
    bytecode_file += 1;

    // Sets the data of the constant.
    switch (memory->constant_pool[i].type[0]) {
      case 0x01:
        memory->constant_pool[i].data = *(int8_t*)bytecode_file;
        bytecode_file += 1;
        break;

      case 0x02:
        memory->constant_pool[i].data =
            is_big_endian ? *(int64_t*)bytecode_file
                          : SwapLong(*(int64_t*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x03:
        memory->constant_pool[i].data =
            is_big_endian ? *(double*)bytecode_file
                          : SwapDouble(*(double*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x04:
        memory->constant_pool[i].data =
            is_big_endian ? *(uint64_t*)bytecode_file
                          : SwapUint64t(*(uint64_t*)bytecode_file);
        bytecode_file += 8;
        break;

      case 0x05: {
        size_t str_size = 0;
        bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &str_size);
        memory->constant_pool[i].data = std::string(bytecode_file, str_size);
        break;
      }

      default:
        LOGGING_ERROR("Unknown type.");
        break;
    }
  }

  // Gets the heap size.
  std::size_t heap_size = 0;
  bytecode_file += DecodeUleb128((uint8_t*)bytecode_file, &heap_size);
  memory->heap.resize(heap_size);

  // Handles the heap type.
  for (size_t i = 0; i < heap_size; i++) {
    memory->heap[i].type.push_back(*bytecode_file);
    bool is_type_end = false;
    while (!is_type_end) {
      switch (*(uint8_t*)bytecode_file) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x09:
        case 0x0A:
          memory->heap[i].type.push_back(*(uint8_t*)bytecode_file);
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          memory->heap[i].type.push_back(*(uint8_t*)bytecode_file);
          bytecode_file += 1;
          break;

        default:
          LOGGING_ERROR("Unsupported data type.");
          break;
      }
    }
    if (memory->heap[i].type[0] != 0x00) memory->heap[i].const_type = true;
    bytecode_file += 1;
  }

  while (bytecode_file < bytecode_end) {
    bytecode_file = AddBytecodeFileClass(name, memory, bytecode_file, classes);
  }

  return bytecode_file;
}

void AddBytecodeFile(
    const char* file,
    std::unordered_map<std::string, Bytecode::BytecodeFile>& bytecode_files,
    bool is_big_endian,
    std::unordered_map<std::string, Bytecode::Class>& classes) {
  // Gets the filename. (Format: ~filename~class_name)
  const char* filename_start = ++file;
  while (*file != '~') file++;
  std::string file_name(filename_start, file - filename_start + 1);

  std::vector<char> code;
  ReadCodeFromFile(file_name.c_str(), code);

  if (bytecode_files.find(file_name) == bytecode_files.end()) {
    bytecode_files[file_name].name = file_name;
    HandleBytecodeFile("~" + file_name + "~", code.data(), code.size(),
                       is_big_endian, classes);
  }
}
}  // namespace Bytecode
}  // namespace Vm
}  // namespace Aq