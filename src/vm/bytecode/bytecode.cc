// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "vm/bytecode/bytecode.h"

#include <cstdint>

#include "vm/memory/memory.h"
#include "vm/utils/utils.h"

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
  if (*(char*)location == '.') location += 1;

  // Gets the function name.
  std::string function_name(location);
  while (*(char*)location != '\0') location += 1;
  location += 1;

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

  // Sets the arguments.
  std::size_t arguments_size = 0;
  location += DecodeUleb128((uint8_t*)location, &arguments_size);
  function.arguments.resize(arguments_size);
  for (size_t i = 0; i < arguments_size; i++)
    location += DecodeUleb128((uint8_t*)location, &function.arguments[i]);

  // Sets the instructions.
  std::size_t instructions_size = 0;
  location += DecodeUleb128((uint8_t*)location, &instructions_size);

  instructions.resize(instructions_size);

  for (size_t i = 0; i < instructions_size; i++) {
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
        LOGGING_ERROR("Invalid operator.");
    }
  }

  return location;
}

char* AddClass(char* location,
               std::unordered_map<std::string, Bytecode::Class> classes,
               std::shared_ptr<Memory::Memory> memory) {
  // Gets the class name.
  std::string class_name(location);
  while (*(char*)location != '\0') location += 1;
  location += 1;

  // Gets the members size.
  std::size_t members_size = 0;
  location += DecodeUleb128((uint8_t*)location, &members_size);

  // Sets the members type.
  for (size_t i = 0; i < members_size; i++) {
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
          Aq::Vm::LOGGING_ERROR("Unsupported data type.");
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
        builtin_functions) {
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
                      run_code[i].arguments[2]);
        break;
      case Operator::Operator::ARRAY:
        Operator::ARRAY(heap, run_code[i].arguments[0],
                        run_code[i].arguments[1], run_code[i].arguments[2]);
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
        Operator::INVOKE(heap, builtin_functions, run_code[i].arguments);
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
        Operator::INVOKE_METHOD(heap, builtin_functions, run_code[i].arguments);
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

int InvokeCustomFunction(const char* name, size_t args_size,
                         size_t return_value, size_t* args) {
  TRACE_FUNCTION;
  FuncInfo func_info = GetCustomFunction(name, args, args_size);
  if (args_size < func_info.args_size) {
    EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
            "Invalid args_size.");
  }

  if (func_info.va_flag) {
    uint8_t* type = calloc(1, sizeof(uintptr_t));
    type[0] = 0x04;
    object_table[return_value].type = type;
    object_table[return_value].data.uint64t_data =
        args_size - func_info.args_size;

    NEW(func_info.args[func_info.args_size], return_value, 0x00);

    if (object_table[func_info.args[func_info.args_size]].type == NULL ||
        object_table[func_info.args[func_info.args_size]].type[0] != 0x06)
      EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
              "Invalid va_arg array.");

    for (size_t i = 0; i < args_size - func_info.args_size; i++) {
      object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .type = object_table[args[func_info.args_size - 1 + i]].type;

      object_table[func_info.args[func_info.args_size]]
          .data.ptr_data[i + 1]
          .data = object_table[args[func_info.args_size - 1 + i]].data;
    }
  }

  object_table[func_info.args[0]] = object_table[return_value];
  struct Object* return_object = object_table + func_info.args[0];
  func_info.args++;
  args_size--;
  for (size_t i = 0; i < func_info.args_size - 1; i++) {
    if (object_table[func_info.args[i]].const_type &&
        object_table[func_info.args[i]].type[0] == 0x07 &&
        object_table[func_info.args[i]].type[1] != 0x08) {
      object_table[func_info.args[i]].data.reference_data =
          object_table + args[i];
    } else if (object_table[func_info.args[i]].const_type &&
               object_table[func_info.args[i]].type[0] == 0x07 &&
               object_table[func_info.args[i]].type[1] == 0x08) {
      object_table[func_info.args[i]].type =
          object_table[func_info.args[i]].type + 1;
      object_table[func_info.args[i]].data.const_data = object_table + args[i];
    } else if (object_table[func_info.args[i]].const_type &&
               object_table[func_info.args[i]].type[0] == 0x08) {
      object_table[func_info.args[i]].data.const_data = object_table + args[i];
    } else {
      EQUAL(func_info.args[i], args[i]);
    }
  }
  struct Bytecode* run_code = func_info.commands;
  for (size_t i = 0; i < func_info.commands_size; i++) {
    switch (run_code[i].operator) {
      case 0x00:
        NOP();
        break;
      case 0x01:
        LOAD(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x02:
        STORE(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x03:
        NEW(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x04:
        ARRAY(run_code[i].arguments[0], run_code[i].arguments[1],
              run_code[i].arguments[2]);
        break;
      case 0x05:
        PTR(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x06:
        ADD(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x07:
        SUB(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x08:
        MUL(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x09:
        DIV(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x0A:
        REM(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x0B:
        NEG(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x0C:
        SHL(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x0D:
        SHR(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x0E:
        REFER(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x0F:
        i = IF(run_code[i].arguments[0], run_code[i].arguments[1],
               run_code[i].arguments[2]);
        i--;
        break;
      case 0x10:
        AND(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x11:
        OR(run_code[i].arguments[0], run_code[i].arguments[1],
           run_code[i].arguments[2]);
        break;
      case 0x12:
        XOR(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2]);
        break;
      case 0x13:
        CMP(run_code[i].arguments[0], run_code[i].arguments[1],
            run_code[i].arguments[2], run_code[i].arguments[3]);
        break;
      case 0x14:
        INVOKE(run_code[i].arguments);
        break;
      case 0x15:
        EQUAL(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x16:
        i = GOTO(run_code[i].arguments[0]);
        i--;
        break;
      case 0x17:
        LOAD_CONST(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x18:
        CONVERT(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x19:
        _CONST(run_code[i].arguments[0], run_code[i].arguments[1]);
        break;
      case 0x1A:
        INVOKE_METHOD(run_code[i].arguments);
        break;
      case 0x1B:
        LOAD_MEMBER(run_code[i].arguments[0], run_code[i].arguments[1],
                    run_code[i].arguments[2]);
        break;
      case 0xFF:
        WIDE();
        break;
      default:
        EXIT_VM("InvokeCustomFunction(const char*,size_t,size_t,size_t*)",
                "Invalid operator.");
        break;
    }
  }

  return 0;
}

void* AddBytecodeFileClass(const char* name, struct Memory* memory,
                           void* location) {
  char* class_name =
      calloc(strlen(name) + strlen((char*)location) + 1, sizeof(char));
  AddFreePtr(class_name);
  snprintf(class_name, strlen(name) + strlen((char*)location) + 1, "%s%s", name,
           (char*)location);

  struct ClassList* table = &class_table[hash(class_name)];
  if (table == NULL)
    EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
            "table is NULL.");
  while (table->next != NULL) {
    table = table->next;
  }
  table->class.name = class_name;
  while (*(char*)location != '\0') {
    location = (void*)((uintptr_t)location + 1);
  }
  location = (void*)((uintptr_t)location + 1);

  size_t object_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);
  table->class.members_size = object_size;
  table->class.members =
      (struct Object*)calloc(object_size, sizeof(struct Object));
  if (table->class.members == NULL)
    EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
            "calloc failed.");
  AddFreePtr(table->class.members);

  for (size_t i = 0; i < object_size; i++) {
    struct ClassVarInfoList* var_info =
        &table->class.var_info_table[hash(location)];
    if (var_info == NULL)
      EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
              "var info table is NULL.");
    while (var_info->next != NULL) {
      var_info = var_info->next;
    }
    var_info->name = location;
    var_info->index = i;
    var_info->next =
        (struct ClassVarInfoList*)calloc(1, sizeof(struct ClassVarInfoList));
    AddFreePtr(var_info->next);
    while (*(char*)location != '\0') {
      location = (void*)((uintptr_t)location + 1);
    }
    location = (void*)((uintptr_t)location + 1);

    table->class.members[i].type = location;
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
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          location = (void*)((uintptr_t)location + 1);
          break;

        default:
          EXIT_VM("AddBytecodeFileClass(const char*,struct Memory*,void*)",
                  "Unsupported type.");
          break;
      }
    }
    if (table->class.members[i].type[0] != 0x00)
      table->class.members[i].const_type = true;
    location = (void*)((uintptr_t)location + 1);
  }

  size_t method_size =
      is_big_endian ? *(uint64_t*)location : SwapUint64t(*(uint64_t*)location);
  location = (void*)((uintptr_t)location + 8);

  for (size_t i = 0; i < method_size; i++) {
    location = AddClassMethodFromOutside(location, table->class.methods);
  }

  table->next = (struct ClassList*)calloc(1, sizeof(struct ClassList));
  AddFreePtr(table->next);

  table->class.memory = memory;

  return location;
}

void HandleBytecodeFile(const char* name, void* bytecode_file, size_t size) {
  TRACE_FUNCTION;
  if (((char*)bytecode_file)[0] != 0x41 || ((char*)bytecode_file)[1] != 0x51 ||
      ((char*)bytecode_file)[2] != 0x42 || ((char*)bytecode_file)[3] != 0x43) {
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "Invalid bytecode file.");
  }

  if (((char*)bytecode_file)[4] != 0x00 || ((char*)bytecode_file)[5] != 0x00 ||
      ((char*)bytecode_file)[6] != 0x00 || ((char*)bytecode_file)[7] != 0x03) {
    EXIT_VM(
        "HandleBytecodeFile(const char*,const char*,size_t)",
        "This bytecode version is not supported, please check for updates.");
  }

  void* bytecode_end = (void*)((uintptr_t)bytecode_file + size);

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  struct Memory* memory = calloc(1, sizeof(struct Memory));

  memory->const_object_table_size =
      is_big_endian ? *(uint64_t*)bytecode_file
                    : SwapUint64t(*(uint64_t*)bytecode_file);

  memory->const_object_table = (struct Object*)malloc(
      memory->const_object_table_size * sizeof(struct Object));

  if (memory->const_object_table == NULL)
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "const_object_table malloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < memory->const_object_table_size; i++) {
    memory->const_object_table[i].type = bytecode_file;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
    switch (memory->const_object_table[i].type[0]) {
      case 0x01:
        memory->const_object_table[i].data.byte_data = *(int8_t*)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
        break;

      case 0x02:
        memory->const_object_table[i].data.long_data = *(int64_t*)bytecode_file;
        memory->const_object_table[i].data.long_data =
            is_big_endian
                ? memory->const_object_table[i].data.long_data
                : SwapLong(memory->const_object_table[i].data.long_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x03:
        memory->const_object_table[i].data.double_data =
            *(double*)bytecode_file;
        memory->const_object_table[i].data.double_data =
            is_big_endian
                ? const_object_table[i].data.double_data
                : SwapDouble(memory->const_object_table[i].data.double_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x04:
        memory->const_object_table[i].data.uint64t_data =
            *(uint64_t*)bytecode_file;
        memory->const_object_table[i].data.uint64t_data =
            is_big_endian
                ? const_object_table[i].data.uint64t_data
                : SwapUint64t(memory->const_object_table[i].data.uint64t_data);
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      case 0x05: {
        size_t str_size = 0;
        bytecode_file = (void*)((uintptr_t)bytecode_file +
                                DecodeUleb128(bytecode_file, &str_size));
        memory->const_object_table[i].data.string_data = bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + str_size);
        break;
      }

      case 0x06:
        memory->const_object_table[i].data.ptr_data = *(void**)bytecode_file;
        bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
        break;

      default:
        EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
                "Unknown type.");
        break;
    }
  }

  memory->object_table_size = is_big_endian
                                  ? *(uint64_t*)bytecode_file
                                  : SwapUint64t(*(uint64_t*)bytecode_file);

  memory->object_table =
      (struct Object*)calloc(memory->object_table_size, sizeof(struct Object));

  if (memory->object_table == NULL)
    EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
            "object_table calloc failed.");

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  for (size_t i = 0; i < memory->object_table_size; i++) {
    memory->object_table[i].type = bytecode_file;
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
          is_type_end = true;
          break;

        case 0x06:
        case 0x07:
        case 0x08:
          bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
          break;

        default:
          EXIT_VM("HandleBytecodeFile(const char*,const char*,size_t)",
                  "Unsupported type.");
          break;
      }
    }
    if (memory->object_table[i].type[0] != 0x00)
      memory->object_table[i].const_type = true;
    bytecode_file = (void*)((uintptr_t)bytecode_file + 1);
  }

  while (bytecode_file < bytecode_end) {
    bytecode_file = AddBytecodeFileClass(name, memory, bytecode_file);
  }
}

void AddBytecodeFile(const char* file) {
  file++;
  const char* filename_start = file;
  while (*file != '~') {
    file++;
  }
  char* file_name = calloc(file - filename_start + 1, sizeof(char));
  memcpy(file_name, filename_start, file - filename_start);

  FILE* bytecode = fopen(file_name, "rb");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", file_name);
    EXIT_VM("AddBytecodeFile(const char*,size_t)", "Could not open file.");
  }
  fseek(bytecode, 0, SEEK_END);
  size_t bytecode_size = ftell(bytecode);
  void* bytecode_file = malloc(bytecode_size);
  void* bytecode_begin = bytecode_file;
  void* bytecode_end = (void*)((uintptr_t)bytecode_file + bytecode_size);
  fseek(bytecode, 0, SEEK_SET);
  fread(bytecode_file, 1, bytecode_size, bytecode);
  fclose(bytecode);

  const unsigned int class_hash = hash(file_name);
  struct BytecodeFileList* current_bytecode_file_table =
      &bytecode_file_table[class_hash];
  bool is_exist = false;
  while (current_bytecode_file_table->next != NULL) {
    if (strcmp(current_bytecode_file_table->name, file_name) == 0) {
      is_exist = true;
      break;
    }
    current_bytecode_file_table = current_bytecode_file_table->next;
  }

  if (!is_exist) {
    current_file_count++;
    current_bytecode_file_table->next = malloc(sizeof(struct BytecodeFileList));
    current_bytecode_file_table->name = file_name;
    char* name = calloc(strlen(file_name) + 3, sizeof(char));
    snprintf(name, strlen(file_name) + 3, "~%s~", file_name);

    HandleBytecodeFile(name, bytecode_file, bytecode_size);
  }
}
}  // namespace Bytecode
}  // namespace Vm
}  // namespace Aq