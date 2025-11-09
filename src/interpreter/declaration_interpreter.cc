// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "interpreter/declaration_interpreter.h"

#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "aq.h"
#include "ast/ast.h"
#include "ast/type.h"
#include "interpreter/bytecode.h"
#include "interpreter/expression_interpreter.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "interpreter/operator.h"
#include "interpreter/statement_interpreter.h"
#include "logging/logging.h"
#include "operator.h"
#include "parser/parser.h"

namespace Aq {
namespace Interpreter {
std::unordered_map<std::string, Interpreter*> imports_map;
std::unordered_set<std::string> currently_importing;

// Helper function to resolve import path relative to source file
std::string ResolveImportPath(const std::string& source_file_path, 
                               const std::string& import_location) {
  namespace fs = std::filesystem;
  
  try {
    // Get the directory of the source file
    fs::path source_path(source_file_path);
    fs::path source_dir = source_path.parent_path();
    
    // If source_dir is empty, use current directory
    if (source_dir.empty()) {
      source_dir = fs::current_path();
    }
    
    // Resolve the import path relative to source directory
    fs::path import_path = source_dir / import_location;
    
    // Get canonical path (resolves .., ., and symlinks)
    fs::path canonical = fs::canonical(import_path);
    
    return canonical.string();
  } catch (const fs::filesystem_error& e) {
    // If canonical fails (e.g., file doesn't exist), return the best we can do
    fs::path source_path(source_file_path);
    fs::path source_dir = source_path.parent_path();
    if (source_dir.empty()) {
      source_dir = fs::current_path();
    }
    fs::path import_path = source_dir / import_location;
    return import_path.lexically_normal().string();
  }
}

void HandleImport(Interpreter& interpreter, Ast::Import* statement) {
  if (statement == nullptr) INTERNAL_ERROR("statement is nullptr.");

  // Gets the reference of context.
  auto& main_class = interpreter.main_class;
  auto& memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& init_code = interpreter.init_code;
  auto& classes = interpreter.classes;
  auto& functions = interpreter.functions;

  // Gets the information from the import statement.
  std::string location = statement->GetImportLocation();
  std::string alias = statement->GetAlias();
  
  // Resolve the import path relative to the source file
  std::string resolved_location = ResolveImportPath(interpreter.source_file_path, location);

  // Generates bytecode if the bytecode isn't generated yet.
  if (imports_map.find(resolved_location) == imports_map.end()) {
    GenerateBytecode(resolved_location, resolved_location);
  }
  
  // If the import is still being generated (circular import), skip the rest
  // The import will be available once the initial import completes
  if (imports_map.find(resolved_location) == imports_map.end()) {
    LOGGING_INFO("Skipping setup for circular import: '" + resolved_location + "' is still being generated.");
    return;
  }

  // Check if this import has already been processed in this interpreter
  if (interpreter.imported_aliases.find(alias) != interpreter.imported_aliases.end()) {
    // Already processed, skip (this can happen because we call HandleImport from
    // both PreProcessImport and the main loop in interpreter.cc)
    LOGGING_INFO("Import '" + alias + "' already processed, skipping.");
    return;
  }
  
  // Track this alias as used in the current interpreter
  interpreter.imported_aliases.insert(alias);

  // Register the imported module's classes into the main interpreter
  Interpreter* imported_interpreter = imports_map[resolved_location];
  std::string class_name = "~" + resolved_location + "bc~.!__start";
  
  // Store the mapping from alias to the imported module's class name
  // This allows us to resolve cross-file class references like "test2.TEST_CLASS"
  interpreter.import_alias_to_class_name[alias] = class_name;
  
  // Merge the imported interpreter's global memory into the main interpreter's memory
  // and build a mapping from old indices to new indices
  std::unordered_map<std::size_t, std::size_t> index_map;
  auto& imported_memory = imported_interpreter->global_memory->GetMemory();
  for (std::size_t i = 0; i < imported_memory.size(); i++) {
    std::size_t new_index;
    const Object& obj = imported_memory[i];
    
    // Create appropriate copy based on object type
    switch (obj.type) {
      case 0x00:  // Auto/uninitialized
        new_index = memory->Add(1);
        break;
      case 0x01:  // Byte
        new_index = memory->AddByte(obj.data.byte_data);
        break;
      case 0x02:  // Long/Int
        new_index = memory->AddLong(obj.data.int_data);
        break;
      case 0x03:  // Double/Float
        new_index = memory->AddDouble(obj.data.float_data);
        break;
      case 0x04:  // Uint64
        new_index = memory->AddUint64t(obj.data.uint64t_data);
        break;
      case 0x05:  // String
        if (obj.data.string_data != nullptr) {
          new_index = memory->AddString(*obj.data.string_data);
        } else {
          new_index = memory->Add(1);
        }
        break;
      case 0x07:  // Reference - handle later
      case 0x08:  // Array
      case 0x09:  // Class
      default:
        // For complex types, just allocate space for now
        new_index = memory->Add(1);
        memory->GetMemory()[new_index] = obj;
        break;
    }
    
    index_map[i] = new_index;
  }
  
  // Helper lambda to remap indices in a function
  std::size_t imported_memory_size = imported_memory.size();
  auto remap_function = [&index_map, imported_memory_size](Function& func) -> Function {
    // Remap parameter indices
    std::vector<std::size_t> new_params;
    for (std::size_t old_idx : func.GetParameters()) {
      auto it = index_map.find(old_idx);
      if (it != index_map.end()) {
        new_params.push_back(it->second);
      } else {
        // If index not in map, keep as is (shouldn't happen normally)
        new_params.push_back(old_idx);
      }
    }
    
    // Remap indices in bytecode carefully - only remap if index exists in map
    std::vector<Bytecode> new_code;
    for (const auto& bytecode : func.GetCode()) {
      Bytecode new_bytecode = bytecode;
      // Remap operands if they exist in the index map
      for (std::size_t i = 0; i < bytecode.arguments.size(); i++) {
        std::size_t old_idx = bytecode.arguments[i];
        // Only remap if this index was in the imported memory
        if (old_idx < imported_memory_size) {
          auto it = index_map.find(old_idx);
          if (it != index_map.end()) {
            new_bytecode.arguments[i] = it->second;
          }
        }
      }
      new_code.push_back(new_bytecode);
    }
    
    Function remapped_func(func.GetName(), new_params, new_code);
    if (func.IsVariadic()) remapped_func.EnableVariadic();
    return remapped_func;
  };
  
  // Copy the imported module's main class to the main interpreter with remapped indices
  if (imported_interpreter->classes.find(".!__start") != imported_interpreter->classes.end()) {
    // Create a new Class instead of copying to avoid shallow copy issues
    Class& imported_class = imported_interpreter->classes[".!__start"];
    if (imported_class.GetClassDeclaration() != nullptr) {
      classes[class_name].SetClass(imported_class.GetClassDeclaration());
    }
    classes[class_name].GetCode() = imported_class.GetCode();
    classes[class_name].SetName(class_name);
    
    // Deep copy and remap class members
    auto& original_members = imported_class.GetMembers()->GetMembers();
    for (auto& member_pair : original_members) {
      const std::string& member_name = member_pair.first;
      const Object& member_obj = member_pair.second;
      
      // Deep copy the member object
      Object new_member_obj = member_obj;
      // Clear constant_type flag to allow imported variables to be mutable
      // This is necessary because auto variables might have constant_type=true
      // in their original context but should be mutable when imported
      new_member_obj.constant_type = false;
      
      // Remap any memory references in the object
      if (member_obj.type == 0x07) {  // Reference type
        // Create a new reference with remapped index
        ObjectReference* new_ref = new ObjectReference();
        new_ref->is_class = member_obj.data.reference_data->is_class;
        
        if (new_ref->is_class) {
          new_ref->memory.class_memory = member_obj.data.reference_data->memory.class_memory;
          new_ref->index.variable_name = new std::string(*member_obj.data.reference_data->index.variable_name);
        } else {
          new_ref->memory.memory = member_obj.data.reference_data->memory.memory;
          std::size_t old_idx = member_obj.data.reference_data->index.index;
          if (old_idx < imported_memory_size && index_map.find(old_idx) != index_map.end()) {
            new_ref->index.index = index_map[old_idx];
          } else {
            new_ref->index.index = old_idx;
          }
        }
        
        new_member_obj.data.reference_data = new_ref;
      } else if (member_obj.type == 0x05) {  // String type
        // Deep copy string
        if (member_obj.data.string_data != nullptr) {
          new_member_obj.data.string_data = new std::string(*member_obj.data.string_data);
        }
      } else if (member_obj.type == 0x06) {  // Array type
        // Keep array reference but increment refcount
        if (member_obj.data.array_data != nullptr) {
          member_obj.data.array_data->AddReferenceCount();
        }
      } else if (member_obj.type == 0x09) {  // Class type
        // Keep class reference but increment refcount
        if (member_obj.data.class_data != nullptr) {
          member_obj.data.class_data->AddReferenceCount();
        }
      }
      
      classes[class_name].GetMembers()->GetMembers()[member_name] = new_member_obj;
    }
    
    // Update the @name in the class members to match the registered name
    classes[class_name].GetMembers()->AddString("@name", class_name);
    
    // Transform method names and remap their memory indices
    auto& imported_methods = imported_class.GetMethods();
    std::unordered_map<std::string, std::vector<Function>> transformed_methods;
    
    for (auto& method_pair : imported_methods) {
      std::string original_name = method_pair.first;
      std::string new_name = original_name;
      
      // Remove scope prefixes
      // Try .!__start. prefix first
      std::string prefix1 = ".!__start.";
      if (original_name.find(prefix1) == 0) {
        new_name = original_name.substr(prefix1.length());
      }
      // Try just . prefix
      else if (original_name.length() > 0 && original_name[0] == '.' && 
               original_name != ".!__init" && original_name != ".!__start") {
        new_name = original_name.substr(1);
      }
      
      // Remap all function overloads for this method
      std::vector<Function> remapped_overloads;
      for (auto& func : method_pair.second) {
        remapped_overloads.push_back(remap_function(func));
      }
      
      transformed_methods[new_name] = remapped_overloads;
    }
    
    classes[class_name].GetMethods() = transformed_methods;
  }
  
  // Copy all other classes from the imported module and register them in the functions map
  for (auto& imported_class_pair : imported_interpreter->classes) {
    std::string imported_class_name = imported_class_pair.first;
    
    // Skip the main class as it's already handled above
    if (imported_class_name == ".!__start") continue;
    
    // Transform the class name to include the module prefix
    // Original name: ".TEST_CLASS" or ".!__start.SomeClass"
    // New name: "~path~.!__start.TEST_CLASS" or "~path~.!__start.SomeClass"
    std::string new_class_name;
    if (imported_class_name[0] == '.') {
      // Remove leading dot and add module prefix
      new_class_name = class_name + imported_class_name;
    } else {
      new_class_name = class_name + "." + imported_class_name;
    }
    
    LOGGING_INFO("Registering imported class: '" + imported_class_name + "' as '" + new_class_name + "'");
    
    // Create a new Class instead of copying to avoid shallow copy issues
    Class& imported_class = imported_class_pair.second;
    if (imported_class.GetClassDeclaration() != nullptr) {
      classes[new_class_name].SetClass(imported_class.GetClassDeclaration());
    }
    classes[new_class_name].GetCode() = imported_class.GetCode();
    classes[new_class_name].SetName(new_class_name);
    
    // Deep copy and remap class members
    auto& original_members = imported_class.GetMembers()->GetMembers();
    for (auto& member_pair : original_members) {
      const std::string& member_name = member_pair.first;
      const Object& member_obj = member_pair.second;
      
      // Deep copy the member object
      Object new_member_obj = member_obj;
      // Clear constant_type flag to allow imported variables to be mutable
      // This is necessary because auto variables might have constant_type=true
      // in their original context but should be mutable when imported
      new_member_obj.constant_type = false;
      
      // Remap any memory references in the object
      if (member_obj.type == 0x07) {  // Reference type
        // Create a new reference with remapped index
        ObjectReference* new_ref = new ObjectReference();
        new_ref->is_class = member_obj.data.reference_data->is_class;
        
        if (new_ref->is_class) {
          new_ref->memory.class_memory = member_obj.data.reference_data->memory.class_memory;
          new_ref->index.variable_name = new std::string(*member_obj.data.reference_data->index.variable_name);
        } else {
          new_ref->memory.memory = member_obj.data.reference_data->memory.memory;
          std::size_t old_idx = member_obj.data.reference_data->index.index;
          if (old_idx < imported_memory_size && index_map.find(old_idx) != index_map.end()) {
            new_ref->index.index = index_map[old_idx];
          } else {
            new_ref->index.index = old_idx;
          }
        }
        
        new_member_obj.data.reference_data = new_ref;
      } else if (member_obj.type == 0x05) {  // String type
        // Deep copy string
        if (member_obj.data.string_data != nullptr) {
          new_member_obj.data.string_data = new std::string(*member_obj.data.string_data);
        }
      } else if (member_obj.type == 0x06) {  // Array type
        // Keep array reference but increment refcount
        if (member_obj.data.array_data != nullptr) {
          member_obj.data.array_data->AddReferenceCount();
        }
      } else if (member_obj.type == 0x09) {  // Class type
        // Keep class reference but increment refcount
        if (member_obj.data.class_data != nullptr) {
          member_obj.data.class_data->AddReferenceCount();
        }
      }
      
      classes[new_class_name].GetMembers()->GetMembers()[member_name] = new_member_obj;
    }
    
    // Update the @name in the class members to match the registered name
    classes[new_class_name].GetMembers()->AddString("@name", new_class_name);
    
    // Transform method names and remap their memory indices (same as for main class)
    auto& imported_methods_2 = imported_class.GetMethods();
    std::unordered_map<std::string, std::vector<Function>> transformed_methods;
    
    for (auto& method_pair : imported_methods_2) {
      std::string original_name = method_pair.first;
      std::string new_method_name = original_name;
      
      // Remove scope prefixes
      // Try .!__start. prefix first
      std::string prefix1 = ".!__start.";
      if (original_name.find(prefix1) == 0) {
        new_method_name = original_name.substr(prefix1.length());
      }
      // Try .TEST_CLASS. prefix (class-specific prefix)
      else if (original_name.find(imported_class_name + ".") == 0) {
        new_method_name = original_name.substr(imported_class_name.length() + 1);
      }
      // Try just . prefix
      else if (original_name.length() > 0 && original_name[0] == '.' && 
               original_name != ".!__init" && original_name != ".!__start") {
        new_method_name = original_name.substr(1);
      }
      
      // Remap all function overloads for this method
      std::vector<Function> remapped_overloads;
      for (auto& func : method_pair.second) {
        remapped_overloads.push_back(remap_function(func));
      }
      
      transformed_methods[new_method_name] = remapped_overloads;
    }
    
    classes[new_class_name].GetMethods() = transformed_methods;
    
    // Register in functions map so it can be found during class resolution
    functions[new_class_name].push_back(Function());
  }

  // Gets index from import preprocessing.
  std::size_t index = variables["#" + alias];

  // Initialize the import bytecode directly using NEW (no need for LOAD_MEMBER).
  init_code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {index, memory->AddUint64t(0),
                memory->AddString(class_name)}});
}

void HandleFunctionDeclaration(Interpreter& interpreter,
                               Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& current_scope = interpreter.context.function_context->current_scope;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  auto parameters = statement->GetParameters();
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  std::string function_name =
      scopes[scopes.size() - 2] + "." + statement->GetFunctionName();

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function parameters and return value.
  std::vector<std::size_t> parameters_index;
  HandleReturnVariableInHandlingFunction(interpreter, declaration, scope_name,
                                         parameters_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  // Handles function body.
  HandleStatement(interpreter, declaration->GetFunctionBody(), code);

  // Handles jump statements.
  HandleReturnInHandlingFunction(interpreter, code);
  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddFunctionIntoList(interpreter, declaration, function_name, parameters_index,
                      code);

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassFunctionDeclaration(Interpreter& interpreter,
                                    Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;
  auto& label_map = interpreter.context.function_context->label_map;
  auto& scopes = interpreter.context.scopes;
  auto& current_scope = interpreter.context.function_context->current_scope;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  std::vector<Bytecode> code;

  // Handles the class constructor if this function is a constructor function.
  if (std::string(current_class->GetClassDeclaration()->GetClassName()) ==
      name) {
    HandleClassConstructor(interpreter, declaration);
    return;
  }

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  // Records the creation of the function.
  current_class->GetMethods()[name].push_back(Function());

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  // Handles function parameters and return value.
  std::vector<std::size_t> parameters_index;
  HandleReturnVariableInHandlingFunction(interpreter, declaration, scope_name,
                                         parameters_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  // Handles function body.
  HandleClassStatement(interpreter, declaration->GetFunctionBody(), code);

  // Handles jump statements.
  HandleReturnInHandlingFunction(interpreter, code);
  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddClassFunctionIntoList(interpreter, declaration, parameters_index, code);

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassConstructor(Interpreter& interpreter,
                            Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto parameters_index =
      HandleFactoryFunctionInHandlingConstructor(interpreter, declaration);

  HandleConstructorFunctionInHandlingConstructor(interpreter, declaration,
                                                 parameters_index);
}

void HandleClassDeclaration(Interpreter& interpreter, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.classes;
  auto& current_class = interpreter.context.current_class;
  auto& functions = interpreter.functions;

  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());

  // Adds the class name into scopes.
  scopes.push_back(class_name);

  // Check if there are any errors in the preprocessing.
  if (classes.find(class_name) == classes.end())
    INTERNAL_ERROR("Not found class declaration: " + class_name);
  current_class = &classes[class_name];

  // Adds the special variable into class memory.
  current_class->GetMembers()->AddString("@name", class_name);

  HandleSubClassesInHandlingClass(interpreter, declaration);

  // Restores function context null caused by possible sub classes generation.
  current_class = &classes[class_name];

  HandleStaticMembersInHandlingClass(interpreter, declaration);

  HandleClassMembersInHandlingClass(interpreter, declaration);

  HandleMethodsInHandlingClass(interpreter, declaration);

  // Adds the void default constructor if the class doesn't have a constructor.
  if (functions.find(class_name) == functions.end())
    AddVoidConstructorInHandlingClass(interpreter, declaration);

  // Destroys temporary context.
  if (scopes.size() <= 0) {
    LOGGING_WARNING(
        "Scopes size is 0, this may cause unexpected behavior. "
        "Please check the class declaration.");
  } else {
    scopes.pop_back();
  }
  current_class = nullptr;
}

std::size_t HandleVariableDeclaration(Interpreter& interpreter,
                                      Ast::Variable* declaration,
                                      std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& variables = interpreter.context.variables;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  uint8_t vm_type = declaration->GetVariableType()->GetVmType();
  uint8_t return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name =
      scopes.back() + "#" + declaration->GetVariableName();

  // For class types with initialization, use auto type to avoid
  // pre-allocating uninitialized class memory.
  uint8_t alloc_type = vm_type;
  if (category == Ast::Type::TypeCategory::kClass &&
      declaration->GetVariableValue()[0] != nullptr) {
    alloc_type = 0x00;  // Use auto type for class variables with initialization
  }

  std::size_t variable_index = memory->AddWithType(alloc_type);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index = HandleExpression(
        interpreter, declaration->GetVariableValue()[0], code, 0);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      code.push_back(
          Bytecode{_AQVM_OPERATOR_REFER, {variable_index, value_index}});
    } else {
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {variable_index, value_index}});
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");

  } else if (category == Ast::Type::TypeCategory::kClass) {
    HandleClassInHandlingVariable(interpreter, declaration, variable_index,
                                  code);
  }

  variables[variable_name] = variable_index;
  return variable_index;
}

std::size_t HandleGlobalVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration,
                                            std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& start_class = interpreter.main_class;
  auto& variables = interpreter.context.variables;
  auto memory = start_class.GetMembers();
  auto& global_memory = interpreter.global_memory;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  uint8_t vm_type = declaration->GetVariableType()->GetVmType();
  uint8_t return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name = declaration->GetVariableName();

  std::size_t reference_index = global_memory->AddWithType(return_type);

  memory->AddReference(variable_name, global_memory, reference_index);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index = HandleExpression(
        interpreter, declaration->GetVariableValue()[0], code, reference_index);

    // Avoids the void class information.
    if (category == Ast::Type::TypeCategory::kClass)
      HandleClassInHandlingVariableWithValue(interpreter, declaration,
                                             reference_index, code);

    if (value_index != reference_index)
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {reference_index, value_index}});

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");

  } else if (category == Ast::Type::TypeCategory::kClass) {
    HandleClassInHandlingVariable(interpreter, declaration, reference_index,
                                  code);
  }

  variables[variable_name] = reference_index;

  return reference_index;
}

std::size_t HandleStaticVariableDeclaration(Interpreter& interpreter,
                                            Ast::Variable* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& global_code = interpreter.global_code;
  auto& variables = interpreter.context.variables;

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  uint8_t vm_type = declaration->GetVariableType()->GetVmType();
  uint8_t return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name =
      scopes.back() + "." + declaration->GetVariableName();

  // For class types with initialization, use auto type to avoid
  // pre-allocating uninitialized class memory.
  uint8_t alloc_type = vm_type;
  if (category == Ast::Type::TypeCategory::kClass &&
      declaration->GetVariableValue()[0] != nullptr) {
    alloc_type = 0x00;  // Use auto type for class variables with initialization
  }
  
  std::size_t variable_index = global_memory->AddWithType(alloc_type);

  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index = HandleExpression(
        interpreter, declaration->GetVariableValue()[0], global_code, 0);

    // If the variable is a reference type, it needs to be handled
    // specially.
    if (category == Ast::Type::TypeCategory::kReference) {
      global_code.push_back(
          Bytecode{_AQVM_OPERATOR_REFER, {variable_index, value_index}});
    } else {
      global_code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {variable_index, value_index}});
    }

  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");
  } else if (category == Ast::Type::TypeCategory::kClass) {
    // If the variable is a class type without initialization, it needs to be
    // created and default-initialized.
    HandleClassInHandlingVariable(interpreter, declaration, variable_index,
                                  global_code);
  }

  variables[variable_name] = variable_index;
  return variable_index;
}

std::size_t HandleClassVariableDeclaration(Interpreter& interpreter,
                                           Ast::Variable* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& start_class = interpreter.main_class;
  auto& global_memory = interpreter.global_memory;
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  // For non const types, |return_type| is equivalent to |vm_type|, but for
  // const types, |vm_type| is the internal variable type excluding the const
  // wrapper, and |return_type| is the final returned variable type.
  uint8_t vm_type = declaration->GetVariableType()->GetVmType();
  uint8_t return_type = vm_type;

  auto category = declaration->GetVariableType()->GetTypeCategory();

  // Deletes const flag if the variable type contain const.
  // if (category == Ast::Type::TypeCategory::kConst)
  //   vm_type.erase(vm_type.begin());

  std::string variable_name = declaration->GetVariableName();

  memory->AddWithType(variable_name, return_type);

  std::size_t reference_index =
      global_memory->AddReference(memory, variable_name);
  /*code.push_back(
      Bytecode{_AQVM_OPERATOR_LOAD_MEMBER,
               {reference_index, 0,
     global_memory->AddString(variable_name)}});*/

  // If the variable is a class type, it needs to be handled specially.
  if (category == Ast::Type::TypeCategory::kClass) {
    // Create a temporary reference index for use during initialization
    std::size_t temp_reference_index = global_memory->Add(1);
    code.push_back(
        Bytecode{_AQVM_OPERATOR_LOAD_MEMBER,
                 {temp_reference_index, 0,
                  global_memory->AddString(variable_name)}});
    HandleClassInHandlingVariable(interpreter, declaration, temp_reference_index,
                                  code);
  }
  // If the variable value isn't nullptr, it means that the variable is
  // initialized.
  if (declaration->GetVariableValue()[0] != nullptr) {
    std::size_t value_index = HandleExpression(
        interpreter, declaration->GetVariableValue()[0], code, 0);

    // Generate bytecode to load the member reference and assign the value
    std::size_t temp_reference_index = global_memory->Add(1);
    code.push_back(
        Bytecode{_AQVM_OPERATOR_LOAD_MEMBER,
                 {temp_reference_index, 0,
                  global_memory->AddString(variable_name)}});
    code.push_back(
        Bytecode{_AQVM_OPERATOR_EQUAL, {temp_reference_index, value_index}});
  } else if (category == Ast::Type::TypeCategory::kReference) {
    // If the variable is a reference type and not initialized, it will meet
    // undefined behavior.
    LOGGING_WARNING(
        "The reference variable declaration without initialization is "
        "deprecated.");
  }

  return reference_index;
}
std::size_t HandleArrayDeclaration(Interpreter& interpreter,
                                   Ast::ArrayDeclaration* declaration,
                                   std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& scopes = interpreter.context.scopes;

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name =
      scopes.back() + "#" + declaration->GetVariableName();

  // Adds the array index and the type index.
  std::size_t array_index = global_memory->AddWithType(array_type->GetVmType());
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType() != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {array_index, global_memory->AddByte(1), array_type_index}});

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    code.push_back(
        Bytecode{_AQVM_OPERATOR_ARRAY,
                 {current_index, array_index, global_memory->AddUint64t(0)}});
    code.push_back(
        Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                 {current_index, global_memory->AddString("@constructor"),
                  global_memory->Add(1)}});
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->Add(1);
      code.push_back(
          Bytecode{_AQVM_OPERATOR_ARRAY,
                   {current_index, array_index, global_memory->AddUint64t(i)}});

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code, 0);
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {current_index, value_index}});
    }
  }

  variables[variable_name] = array_index;
  return array_index;
}

std::size_t HandleGlobalArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration,
                                         std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& start_class = interpreter.main_class;
  auto& scopes = interpreter.context.scopes;
  auto memory = start_class.GetMembers();

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name = declaration->GetVariableName();

  // Adds the array index and the type index.
  std::size_t array_index = global_memory->AddWithType(array_type->GetVmType());
  memory->AddReference(variable_name, global_memory, array_index);
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType() != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {array_index, global_memory->AddByte(1), array_type_index}});

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    code.push_back(
        Bytecode{_AQVM_OPERATOR_ARRAY,
                 {current_index, array_index, global_memory->AddUint64t(0)}});
    code.push_back(
        Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                 {current_index, global_memory->AddString("@constructor"),
                  global_memory->Add(1)}});
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    std::size_t current_index = global_memory->Add(1);
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      code.push_back(
          Bytecode{_AQVM_OPERATOR_ARRAY,
                   {current_index, array_index, global_memory->AddUint64t(i)}});

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code, 0);
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {current_index, value_index}});
    }
  }

  variables[variable_name] = array_index;

  return array_index;
}

std::size_t HandleStaticArrayDeclaration(Interpreter& interpreter,
                                         Ast::ArrayDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;
  auto& scopes = interpreter.context.scopes;
  auto& global_code = interpreter.global_code;

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name =
      scopes.back() + "." + declaration->GetVariableName();

  // Adds the array index and the type index.
  std::size_t array_index = global_memory->AddWithType(array_type->GetVmType());
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType() != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  global_code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {array_index, global_memory->AddByte(1), array_type_index}});

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    global_code.push_back(
        Bytecode{_AQVM_OPERATOR_ARRAY,
                 {current_index, array_index, global_memory->AddUint64t(0)}});
    global_code.push_back(
        Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                 {current_index, global_memory->AddString("@constructor"),
                  global_memory->Add(1)}});
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    std::size_t current_index = global_memory->Add(1);
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      global_code.push_back(
          Bytecode{_AQVM_OPERATOR_ARRAY,
                   {current_index, array_index, global_memory->AddUint64t(i)}});

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], global_code, 0);
      global_code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {current_index, value_index}});
    }
  }

  variables[variable_name] = array_index;
  return array_index;
}

std::size_t HandleClassArrayDeclaration(Interpreter& interpreter,
                                        Ast::ArrayDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& global_memory = interpreter.global_memory;
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  // Handles the array type.
  Ast::ArrayType* array_type =
      dynamic_cast<Ast::ArrayType*>(declaration->GetVariableType());
  if (array_type == nullptr) INTERNAL_ERROR("array_type is nullptr.");
  if (array_type->GetTypeCategory() == Ast::Type::TypeCategory::kConst)
    LOGGING_ERROR("const array not support.");

  std::string variable_name = declaration->GetVariableName();

  // Adds the array index and the type index.
  memory->AddWithType(variable_name, array_type->GetVmType());
  std::size_t array_index = global_memory->Add(1);
  code.push_back(
      Bytecode{_AQVM_OPERATOR_LOAD_MEMBER,
               {3, array_index, 0, global_memory->AddString(variable_name)}});
  std::size_t array_type_index = 0;

  // Gets the sub type of the array type and its category.
  Ast::Type* sub_type = array_type->GetSubType();
  auto sub_type_category = sub_type->GetTypeCategory();

  // If the sub type is a class type, it needs to be handled specially.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::string class_name = GetClassNameString(
        interpreter, dynamic_cast<Ast::ClassType*>(sub_type));
    array_type_index = global_memory->AddString(class_name);

  } else {
    // If the vm type of the sub type isn't 0x00 (auto type), it means that
    // the sub type is a primitive type, so we can add it into the global
    // memory.
    if (sub_type->GetVmType() != 0x00)
      array_type_index = global_memory->AddWithType(sub_type->GetVmType());
  }

  // Handles the array creation bytecode.
  // The array is created with the size of 1, and the type of the sub type.
  // This means that regardless of the size of the array definition, it is
  // actually determined based on the actual number of initialization lists
  // given.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {3, array_index, global_memory->AddByte(1), array_type_index}});

  // If the sub type is a class type, it needs to be handled specially. Because
  // the default generated class index is considered an initialized value
  // because it is smaller than the array size, it will not be automatically
  // initialized when the ARRAY operator is called.
  if (sub_type_category == Ast::Type::TypeCategory::kClass) {
    std::size_t current_index = global_memory->Add(1);
    code.push_back(Bytecode{
        _AQVM_OPERATOR_ARRAY,
        {3, current_index, array_index, global_memory->AddUint64t(0)}});
    code.push_back(
        Bytecode{_AQVM_OPERATOR_INVOKE_METHOD,
                 {3, current_index, global_memory->AddString("@constructor"),
                  global_memory->Add(1)}});
  }

  // Handles the array initialization with the initialization lists.
  if (!declaration->GetVariableValue().empty()) {
    for (std::size_t i = 0; i < declaration->GetVariableValue().size(); i++) {
      // Gets the corresponding array index reference.
      std::size_t current_index = global_memory->Add(1);
      code.push_back(Bytecode{
          _AQVM_OPERATOR_ARRAY,
          {3, current_index, array_index, global_memory->AddUint64t(i)}});

      // Gets the value of the initialization list and assigns value to
      // corresponding index.
      std::size_t value_index = HandleExpression(
          interpreter, declaration->GetVariableValue()[i], code, 0);
      code.push_back(
          Bytecode{_AQVM_OPERATOR_EQUAL, {2, current_index, value_index}});
    }
  }

  return array_index;
}

void HandleFunctionArguments(Interpreter& interpreter,
                             Ast::FunctionDeclaration* declaration,
                             std::vector<std::size_t>& parameters_index,
                             std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& variables = interpreter.context.variables;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  auto parameters = statement->GetParameters();

  // Handles functions that only contain variable parameters.
  if (parameters.size() == 0 && statement->IsVariadic()) {
    std::size_t index = memory->Add(1);

    // Adds index into |parameters_index| and |variables|.
    parameters_index.push_back(index);
    variables[scopes.back() + "#args"] = index;
    return;
  }

  // Handles function parameters.
  for (std::size_t i = 0; i < parameters.size(); i++) {
    if (Ast::IsOfType<Ast::Variable>(parameters[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::Variable>(parameters[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index =
          HandleVariableDeclaration(interpreter, declaration, code);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[name] = index;

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(parameters[i])) {
      // Gets function declaration and name.
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(parameters[i]);
      std::string name = scopes.back() + "#" + declaration->GetVariableName();

      std::size_t index =
          HandleArrayDeclaration(interpreter, declaration, code);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[name] = index;

    } else {
      INTERNAL_ERROR(
          "Function parameters is not a variable or array declaration.");
    }

    // Handles variable parameters if have.
    if (i == parameters.size() - 1 && statement->IsVariadic()) {
      std::size_t index = memory->Add(1);

      // Adds index into |parameters_index| and |variables|.
      parameters_index.push_back(index);
      variables[scopes.back() + "#args"] = index;
    }
  }
}

void HandleReturnInHandlingFunction(Interpreter& interpreter,
                                    std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& exit_index = interpreter.context.function_context->exit_index;
  auto& memory = interpreter.global_memory;

  code.push_back(Bytecode{_AQVM_OPERATOR_NOP, {}});
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index.size(); i++) {
    code[exit_index[i]].arguments = {memory->AddUint64t(return_location)};
  }
}

void HandleGotoInHandlingFunction(Interpreter& interpreter,
                                  std::size_t current_scope,
                                  std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& goto_map = interpreter.context.function_context->goto_map;
  auto& memory = interpreter.global_memory;
  auto& scopes = interpreter.context.scopes;
  auto& label_map = interpreter.context.function_context->label_map;

  while (goto_map.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = scopes.size() - 1; i >= current_scope; i--) {
      auto iterator = label_map.find(scopes[i] + "$" + goto_map.back().first);
      if (iterator != label_map.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_scope) LOGGING_ERROR("Label not found.");
    }

    code[goto_map.back().second].arguments = {
        memory->AddUint64t(goto_location)};
    goto_map.pop_back();
  }
}

void AddFunctionIntoList(Interpreter& interpreter,
                         Ast::FunctionDeclaration* declaration,
                         std::string name,
                         std::vector<std::size_t> parameters_index,
                         std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& functions = interpreter.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();

  Function function(name, parameters_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  functions[name].push_back(function);
}

void AddClassFunctionIntoList(Interpreter& interpreter,
                              Ast::FunctionDeclaration* declaration,
                              std::vector<std::size_t>& parameters_index,
                              std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& methods = interpreter.context.current_class->GetMethods();

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();

  // Adds function into class function list.
  Function function(name, parameters_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  methods[name].push_back(function);
}

void AddClassConstructorFunctionIntoList(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index, std::vector<Bytecode>& code) {
  // Gets the reference of context.
  auto& methods = interpreter.context.current_class->GetMethods();

  Ast::Function* statement = declaration->GetFunctionStatement();

  // Adds function into class function list.
  Function function("@constructor", parameters_index, code);
  if (statement->IsVariadic()) function.EnableVariadic();
  methods["@constructor"].push_back(function);
}

void HandleReturnVariableInHandlingFunction(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::string scope_name, std::vector<std::size_t>& parameters_index) {
  // Gets the reference of context.
  auto& variables = interpreter.context.variables;
  auto& memory = interpreter.global_memory;

  uint8_t vm_type = declaration->GetReturnType()->GetVmType();
  variables[scope_name + "#!return"] = memory->AddWithType(vm_type);
  variables[scope_name + "#!return_reference"] = memory->Add(1);
  parameters_index.push_back(variables[scope_name + "#!return_reference"]);
}

std::vector<std::size_t> HandleFactoryFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& functions = interpreter.functions;

  Ast::Function* statement = declaration->GetFunctionStatement();
  std::string name = statement->GetFunctionName();
  auto parameters = statement->GetParameters();
  std::vector<Bytecode> code;

  // name = GetFunctionNameWithScope(interpreter, declaration);
  name = scopes.back();
  scopes.push_back(name);

  // std::string function_name =
  //     scopes[scopes.size() - 2] + "." + statement->GetFunctionName();

  // Records the creation of the function.
  functions[name].push_back(Function());

  // Handles the return value and parameters.
  std::vector<std::size_t> parameters_index;
  std::size_t return_index = memory->Add(1);
  parameters_index.push_back(return_index);
  HandleFunctionArguments(interpreter, declaration, parameters_index, code);

  // Builds the main part of the factory function.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {return_index, memory->AddUint64t(0), memory->AddString(name)}});
  std::vector<std::size_t> method_parameters = parameters_index;
  method_parameters.erase(method_parameters.begin());
  method_parameters.insert(
      method_parameters.begin(),
      {return_index, memory->AddString("@constructor"), memory->Add(1)});
  code.push_back(
      Bytecode{_AQVM_OPERATOR_INVOKE_METHOD, std::move(method_parameters)});

  AddFunctionIntoList(interpreter, declaration, name, parameters_index, code);

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;

  return parameters_index;
}

void HandleConstructorFunctionInHandlingConstructor(
    Interpreter& interpreter, Ast::FunctionDeclaration* declaration,
    std::vector<std::size_t>& parameters_index) {
  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& memory = interpreter.global_memory;
  auto& functions = interpreter.functions;
  auto& current_class = interpreter.context.current_class;
  auto& current_scope = interpreter.context.function_context->current_scope;

  // Creates the bytecode vector.
  std::vector<Bytecode> code;

  // Handles the function name with scopes.
  std::string scope_name = GetFunctionNameWithScope(interpreter, declaration);
  scopes.push_back(scope_name);
  current_scope = scopes.size() - 1;

  // Records the creation of the function.
  current_class->GetMethods()["@constructor"].push_back(Function());

  code.push_back(Bytecode{_AQVM_OPERATOR_NOP, {}});

  // Handles the class init code.
  for (std::size_t i = 0; i < current_class->GetCode().size(); i++) {
    code.push_back(current_class->GetCode()[i]);
  }

  // DEPRECATED: Keep this implementation for adaptation to previous versions.
  // In principle, it will no longer be used.
  if (declaration->GetFunctionBody() == nullptr) {
    LOGGING_WARNING(
        "The function statement without function body has been deprecated.");
    scopes.pop_back();
    return;
  }

  HandleClassStatement(interpreter, declaration->GetFunctionBody(), code);

  HandleReturnInHandlingFunction(interpreter, code);

  HandleGotoInHandlingFunction(interpreter, current_scope, code);

  AddClassConstructorFunctionIntoList(interpreter, declaration,
                                      parameters_index, code);

  // Destroys temporary context.
  scopes.pop_back();
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleSubClassesInHandlingClass(Interpreter& interpreter,
                                     Ast::Class* declaration) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& classes = interpreter.classes;

  for (std::size_t i = 0; i < declaration->GetSubClasses().size(); i++) {
    if (Ast::IsOfType<Ast::Class>(declaration->GetSubClasses()[i])) {
      // Handles the sub class declaration.
      auto sub_class = Ast::Cast<Ast::Class>(declaration->GetSubClasses()[i]);
      HandleClassDeclaration(interpreter, sub_class);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }

  // Restores class name.
  auto& current_class = interpreter.context.current_class;
  std::string class_name =
      scopes.back() + "." + std::string(declaration->GetClassName());
  current_class = &classes[class_name];
}

void HandleStaticMembersInHandlingClass(Interpreter& interpreter,
                                        Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetStaticMembers().size(); i++) {
    auto member = declaration->GetStaticMembers()[i]->GetStaticDeclaration();
    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the static variable declaration.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleStaticVariableDeclaration(interpreter, variable);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the static array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleStaticArrayDeclaration(interpreter, array);

    } else if (Ast::IsOfType<Ast::FunctionDeclaration>(member)) {
      // Handles the static function declaration.
      auto function = Ast::Cast<Ast::FunctionDeclaration>(member);
      HandleFunctionDeclaration(interpreter, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleClassMembersInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration) {
  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;
  auto memory = current_class->GetMembers();
  auto& code = current_class->GetCode();

  for (std::size_t i = 0; i < declaration->GetMembers().size(); i++) {
    auto member = dynamic_cast<Ast::Declaration*>(declaration->GetMembers()[i]);

    if (Ast::IsOfType<Ast::Variable>(member)) {
      // Handles the variable.
      auto variable = Ast::Cast<Ast::Variable>(member);
      HandleClassVariableDeclaration(interpreter, variable);

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(member)) {
      // Handles the array declaration.
      auto array = Ast::Cast<Ast::ArrayDeclaration>(member);
      HandleClassArrayDeclaration(interpreter, array);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void HandleMethodsInHandlingClass(Interpreter& interpreter,
                                  Ast::Class* declaration) {
  for (std::size_t i = 0; i < declaration->GetMethods().size(); i++) {
    if (Ast::IsOfType<Ast::FunctionDeclaration>(declaration->GetMethods()[i])) {
      // Handles the class function declaration.
      auto function =
          Ast::Cast<Ast::FunctionDeclaration>(declaration->GetMethods()[i]);
      HandleClassFunctionDeclaration(interpreter, function);

    } else {
      INTERNAL_ERROR("Unexpected code.");
    }
  }
}

void AddVoidConstructorInHandlingClass(Interpreter& interpreter,
                                       Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  auto parameters_index =
      HandleVoidFactoryFunctionInHandlingClass(interpreter, declaration);

  HandleVoidConstructorFunctionInHandlingClass(interpreter, declaration,
                                               parameters_index);
}
std::vector<std::size_t> HandleVoidFactoryFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Creates temporary context.
  interpreter.context.function_context = new FunctionContext();

  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& current_class = interpreter.context.current_class;
  auto& goto_map = interpreter.context.function_context->goto_map;
  auto& variables = interpreter.context.variables;
  auto& memory = interpreter.global_memory;
  auto& exit_index = interpreter.context.function_context->exit_index;

  // Handles the function name with scopes and the class name.
  std::string name = scopes.back();
  std::string class_name = declaration->GetClassName();

  // Handles the parameters.
  std::vector<std::size_t> parameters_index;
  std::size_t return_index = memory->Add(1);
  parameters_index.push_back(return_index);

  // Records the creation of the function.
  functions[name].push_back(Function());

  // Builds the main part of the factory function.
  std::vector<Bytecode> code;
  code.push_back(Bytecode{
      _AQVM_OPERATOR_NEW,
      {return_index, memory->AddUint64t(0), memory->AddString(class_name)}});
  code.push_back(Bytecode{
      _AQVM_OPERATOR_INVOKE_METHOD,
      {return_index, memory->AddString("@constructor"), memory->Add(1)}});

  // Adds the function into function list.
  Function factory(name, parameters_index, code);
  functions[name].push_back(factory);

  // Destroys temporary context.
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
  return parameters_index;
}

void HandleVoidConstructorFunctionInHandlingClass(
    Interpreter& interpreter, Ast::Class* declaration,
    std::vector<std::size_t>& parameters_index) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& current_class = interpreter.context.current_class;

  // Records the creation of the function.
  current_class->GetMethods()["@constructor"].push_back(Function());

  // Handles the class init.
  std::vector<Bytecode> code;
  for (std::size_t i = 0; i < current_class->GetCode().size(); i++) {
    code.push_back(current_class->GetCode()[i]);
  }

  // Adds function into class function list.
  Function constructor("@constructor", parameters_index, code);
  current_class->GetMethods()["@constructor"].push_back(constructor);

  // Destroys temporary context.
  delete interpreter.context.function_context;
  interpreter.context.function_context = nullptr;
}

void HandleClassInHandlingVariable(Interpreter& interpreter,
                                   Ast::Variable* declaration,
                                   std::size_t variable_index,
                                   std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& memory = interpreter.global_memory;
  // auto& global_code = interpreter.global_code;
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& variables = interpreter.context.variables;

  // Gets the class name, which is same as the function name.
  std::string name = std::string(
      *dynamic_cast<Ast::ClassType*>(declaration->GetVariableType()));
  
  // Check if the class name refers to an imported module (e.g., "test2.TEST_CLASS")
  std::size_t dot_pos = name.find('.');
  if (dot_pos != std::string::npos) {
    std::string potential_alias = name.substr(0, dot_pos);
    std::string class_in_module = name.substr(dot_pos + 1);
    
    // Check if potential_alias is an import alias by looking in the import map
    auto alias_it = interpreter.import_alias_to_class_name.find(potential_alias);
    if (alias_it != interpreter.import_alias_to_class_name.end()) {
      // This is an imported module access
      std::string import_class_name = alias_it->second;
      
      // The imported class name would be like "~path~.!__start"
      // We need to construct the full class name as "~path~.!__start.CLASS_NAME"
      name = import_class_name + "." + class_in_module;
      
      // Verify this class exists
      if (functions.find(name) == functions.end()) {
        LOGGING_ERROR("Class '" + class_in_module + "' not found in imported module '" + potential_alias + "'.");
      }
      
      // Successfully resolved imported class name
      goto class_found;
    }
  }
  
  // Original class resolution logic for non-imported classes
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    // Searching globally first and then conducting a search with scope is to
    // avoid the situation where the scope index is exceeded and -1 occurs.
    auto iterator = functions.find(name);

    // If the search scope is not the global scope, adds the scope name to the
    // class name.
    if (i != -1) iterator = functions.find(scopes[i] + "." + name);
    if (iterator != functions.end()) {
      // Searching globally first and then conducting a search with scope is to
      // avoid the situation where the scope index is exceeded and -1 occurs.
      name = name;

      // If the search scope is not the global scope, adds the scope name to the
      // class name.
      if (i != -1) name = scopes[i] + "." + name;
      break;
    }
    if (i == -1) LOGGING_ERROR("Class not found.");
  }
  
class_found:

  // Adds the class into global memory.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {variable_index, memory->AddByte(0), memory->AddString(name)}});

  // Classes without initialization requires default initialization.
  code.push_back(Bytecode{
      _AQVM_OPERATOR_INVOKE_METHOD,
      {variable_index, memory->AddString("@constructor"), memory->Add(1)}});
}

void HandleClassInHandlingVariableWithValue(Interpreter& interpreter,
                                            Ast::Variable* declaration,
                                            std::size_t variable_index,
                                            std::vector<Bytecode>& code) {
  if (declaration == nullptr) INTERNAL_ERROR("declaration is nullptr.");

  // Gets the reference of context.
  auto& memory = interpreter.global_memory;
  // auto& global_code = interpreter.global_code;
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& variables = interpreter.context.variables;

  // Gets the class name, which is same as the function name.
  std::string name = std::string(
      *dynamic_cast<Ast::ClassType*>(declaration->GetVariableType()));
  
  // Check if the class name refers to an imported module (e.g., "test2.TEST_CLASS")
  std::size_t dot_pos = name.find('.');
  if (dot_pos != std::string::npos) {
    std::string potential_alias = name.substr(0, dot_pos);
    std::string class_in_module = name.substr(dot_pos + 1);
    
    // Check if potential_alias is an import alias by looking in the import map
    auto alias_it = interpreter.import_alias_to_class_name.find(potential_alias);
    if (alias_it != interpreter.import_alias_to_class_name.end()) {
      // This is an imported module access
      std::string import_class_name = alias_it->second;
      
      // The imported class name would be like "~path~.!__start"
      // We need to construct the full class name as "~path~.!__start.CLASS_NAME"
      name = import_class_name + "." + class_in_module;
      
      // Verify this class exists
      if (functions.find(name) == functions.end()) {
        LOGGING_ERROR("Class '" + class_in_module + "' not found in imported module '" + potential_alias + "'.");
      }
      
      // Successfully resolved imported class name
      goto class_found;
    }
  }
  
  // Original class resolution logic for non-imported classes
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    // Searching globally first and then conducting a search with scope is to
    // avoid the situation where the scope index is exceeded and -1 occurs.
    auto iterator = functions.find(name);

    // If the search scope is not the global scope, adds the scope name to the
    // class name.
    if (i != -1) iterator = functions.find(scopes[i] + "." + name);
    if (iterator != functions.end()) {
      // Searching globally first and then conducting a search with scope is to
      // avoid the situation where the scope index is exceeded and -1 occurs.
      name = name;

      // If the search scope is not the global scope, adds the scope name to the
      // class name.
      if (i != -1) name = scopes[i] + "." + name;
      break;
    }
    if (i == -1) LOGGING_ERROR("Class not found.");
  }
  
class_found:

  // Adds the class into global memory.
  code.push_back(
      Bytecode{_AQVM_OPERATOR_NEW,
               {variable_index, memory->AddByte(0), memory->AddString(name)}});
}

std::string GetClassNameString(Interpreter& interpreter, Ast::ClassType* type) {
  // Gets the reference of context.
  auto& scopes = interpreter.context.scopes;
  auto& functions = interpreter.functions;
  auto& variables = interpreter.context.variables;
  auto& memory = interpreter.global_memory;

  std::string name = type->GetClassName();
  
  // Check if the class name refers to an imported module (e.g., "test2.TEST_CLASS")
  std::size_t dot_pos = name.find('.');
  if (dot_pos != std::string::npos) {
    std::string potential_alias = name.substr(0, dot_pos);
    std::string class_in_module = name.substr(dot_pos + 1);
    
    // Check if potential_alias is an import alias by looking in the import map
    auto alias_it = interpreter.import_alias_to_class_name.find(potential_alias);
    if (alias_it != interpreter.import_alias_to_class_name.end()) {
      // This is an imported module access
      std::string import_class_name = alias_it->second;
      
      // The imported class name would be like "~path~.!__start"
      // We need to construct the full class name as "~path~.!__start.CLASS_NAME"
      name = import_class_name + "." + class_in_module;
      
      // Verify this class exists
      if (functions.find(name) == functions.end()) {
        LOGGING_ERROR("Class '" + class_in_module + "' not found in imported module '" + potential_alias + "'.");
      }
      
      return name;
    }
  }
  
  // Original class resolution logic for non-imported classes
  for (int64_t i = scopes.size() - 1; i >= -1; i--) {
    // Searching globally first and then conducting a search with scope is to
    // avoid the situation where the scope index is exceeded and -1 occurs.
    auto iterator = functions.find(name);

    // If the search scope is not the global scope, adds the scope name to the
    // class name.
    if (i != -1) iterator = functions.find(scopes[i] + "." + name);
    if (iterator != functions.end()) {
      // Searching globally first and then conducting a search with scope is to
      // avoid the situation where the scope index is exceeded and -1 occurs.
      name = name;

      // If the search scope is not the global scope, adds the scope name to the
      // class name.
      if (i != -1) name = scopes[i] + "." + name;
      break;
    }
    if (i == -1) LOGGING_ERROR("Class not found.");
  }

  return name;
}

[[deprecated]] void GenerateBytecode(std::string import_file_path, std::string canonical_key) {
  // Check for circular imports - if already importing, skip to avoid infinite loop
  // but don't error, just return since it will be available once the initial import completes
  if (currently_importing.find(canonical_key) != currently_importing.end()) {
    LOGGING_INFO("Skipping circular import: '" + canonical_key + 
                  "' is already being imported.");
    return;
  }

  // Add to currently importing set
  currently_importing.insert(canonical_key);

  Interpreter* interpreter = new Interpreter();
  // Set the source file path for this interpreter so it can resolve its own imports
  interpreter->source_file_path = import_file_path;
  
  std::vector<char> code;
  Aq::ReadCodeFromFile(import_file_path.c_str(), code);

  std::vector<Aq::Token> token;
  Aq::LexCode(code, token);

  Aq::Ast::Compound* ast = Aq::Parser::Parse(token);
  if (ast == nullptr) LOGGING_ERROR("ast is nullptr.");

  interpreter->Generate(ast);

  LOGGING_INFO("Generate Bytecode SUCCESS!");

  imports_map.insert(std::make_pair(canonical_key, interpreter));

  // Remove from currently importing set after successful import
  currently_importing.erase(canonical_key);
}

std::string GetFunctionNameWithScope(Interpreter& interpreter,
                                     Ast::FunctionDeclaration* declaration) {
  // Gets the reference of context.

  auto& scopes = interpreter.context.scopes;

  // Gets the function statement and its parameters.
  Ast::Function* statement = declaration->GetFunctionStatement();
  std::vector<Ast::Expression*> parameters = statement->GetParameters();

  // Gets the function name with scopes.
  std::string scope_name = scopes.back() + "." + statement->GetFunctionName();

  // Adds the function parameters type into the scope name.
  for (std::size_t i = 0; i < parameters.size(); i++) {
    // Adds the argument type separator to the scope name.
    if (i == 0) {
      scope_name += "@";
    } else {
      scope_name += ",";
    }

    // Adds the argument type to the scope name.
    if (Ast::IsOfType<Ast::Variable>(parameters[i])) {
      auto declaration = Ast::Cast<Ast::Variable>(parameters[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else if (Ast::IsOfType<Ast::ArrayDeclaration>(parameters[i])) {
      auto declaration = Ast::Cast<Ast::ArrayDeclaration>(parameters[i]);
      scope_name += std::string(*declaration->GetVariableType());

    } else {
      LOGGING_ERROR(
          "Function parameters is not a variable or array declaration.");
    }

    // Handles variadic parameters.
    if (i == parameters.size() - 1 && statement->IsVariadic()) {
      scope_name += ",...";
    }
  }

  // Handles variadic parameters if the function is variadic and parameters size
  // is 0.
  if (parameters.size() == 0 && statement->IsVariadic()) {
    scope_name += "@...";
  }

  return scope_name;
}

}  // namespace Interpreter
}  // namespace Aq