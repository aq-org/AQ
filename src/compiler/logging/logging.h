// Copyright 2025 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LOGGING_LOGGING_H_
#define AQ_COMPILER_LOGGING_LOGGING_H_

#include <string>

namespace Aq {
namespace Compiler {
namespace Logging {
#define LOGGING_ERROR(message) Logging::ERROR(__FUNCTION__, message)
#define LOGGING_WARNING(message) Logging::WARNING(__FUNCTION__, message)
#define LOGGING_INFO(message) Logging::INFO(__FUNCTION__, message)

// Reports an error and exits the program.
void ERROR(std::string func_name, std::string message);

// Reports a warning. But does not exit the program.
void WARNING(std::string func_name, std::string message);

// Reports an informational message. Does not exit the program.
void INFO(std::string func_name, std::string message);
}  // namespace Logging
}  // namespace Compiler
}  // namespace Aq

#endif