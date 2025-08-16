// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_LOGGING_LOGGING_H_
#define AQ_LOGGING_LOGGING_H_

#include <iostream>
#include <string>

namespace Aq {
namespace Logging {
#define LOGGING_ERROR(message) Logging::ERROR(__FUNCTION__, message)
#define LOGGING_WARNING(message) Logging::WARNING(__FUNCTION__, message)
#define LOGGING_INFO(message) Logging::INFO(__FUNCTION__, message)

#define INTERNAL_ERROR(message) \
  Logging::ERROR(std::string("[[[INTERNAL ERROR]]]") + __FUNCTION__, message)

// Reports an error and exits the program.
inline void ERROR(std::string func_name, std::string message) {
  std::cerr << "[ERROR] " << func_name << ": " << message << std::endl;
  exit(EXIT_FAILURE);
}

// Reports a warning. But does not exit the program.
inline void WARNING(std::string func_name, std::string message) {
  std::cerr << "[WARNING] " << func_name << ": " << message << std::endl;
}

// Reports an informational message. Does not exit the program.
inline void INFO(std::string func_name, std::string message) {
  //std::cerr << "[INFO] " << func_name << ": " << message << std::endl;
}
}  // namespace Logging
}  // namespace Aq

#endif