// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/logging/logging.h"

#include <iostream>
#include <string>

namespace Aq {
namespace Compiler {
namespace Logging {
void ERROR(std::string func_name, std::string message) {
  std::cerr << "[ERROR] " << func_name << ": " << message << std::endl;
  exit(EXIT_FAILURE);
}

void WARNING(std::string func_name, std::string message) {
  std::cerr << "[WARNING] " << func_name << ": " << message << std::endl;
}

void INFO(std::string func_name, std::string message) {
  std::cerr << "[INFO] " << func_name << ": " << message << std::endl;
}
}  // namespace Logging
}  // namespace Compiler
}  // namespace Aq