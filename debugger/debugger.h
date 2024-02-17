// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_DEBUGGER_DEBUGGER_H_
#define AQ_DEBUGGER_DEBUGGER_H_

#include <cstring>
#include <ctime>
#include <string>

namespace Aq {
class Debugger {
 public:
  enum class Level { ERROR = 0, WARNING = 1, INFO = 2 };

  // Output a debug message. If |other_info| has no content, nullptr can be
  // filled in. Others must be filled in with strings and cannot be nullptr.
  Debugger(Level level, const char* location, const char* debug_code,
           const char* debug_message, const char* other_info = nullptr)
      : errno_(errno),
        errno_message_(std::strerror(errno)),
        timestamp_(time(NULL)),
        level_(level),
        location_(location),
        debug_code_(debug_code),
        debug_message_(debug_message),
        other_info_(other_info) {
    OutputMessage();
  };
  ~Debugger() = default;

  Debugger(const Debugger&) = delete;
  Debugger(Debugger&&) = delete;
  Debugger& operator=(const Debugger&) = delete;
  Debugger& operator=(Debugger&&) = delete;

 private:
  time_t timestamp_ = 0;
  Level level_;
  const char* location_;
  const char* debug_code_;
  const char* debug_message_;
  const char* other_info_ = nullptr;

  int errno_ = 0;
  const char* errno_message_;

  // Output a debug message.
  void OutputMessage() const;

  // Get the current time string. Based on ISO 8601 standard.
  std::string GetTimeString() const;
};
}  // namespace Aq

#endif