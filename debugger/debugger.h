// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_DEBUGGER_DEBUGGER_H_
#define AQ_DEBUGGER_DEBUGGER_H_

#include <ctime>
#include <string>

namespace Aq {
class Debugger {
 public:
  enum Level { ERROR = 0, WARNING = 1, INFO = 2 };

  Debugger(Level level, const char* location, const char* debug_code,
        const char* debug_message, const char* other_info);
  ~Debugger();

  Debugger(const Debugger&) = delete;
  Debugger(Debugger&&) = delete;
  Debugger& operator=(const Debugger&) = delete;
  Debugger& operator=(Debugger&&) = delete;

 private:
  time_t timestamp_;
  Level level_;
  const char* location_;
  const char* debug_code_;
  const char* debug_message_;
  const char* other_info_;

  int errno_;
  const char* errno_message_;

  void OutputMessage();
  std::string GetTimeString();
};
}  // namespace Aq

#endif