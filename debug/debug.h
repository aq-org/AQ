// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_DEBUG_DEBUG_H_
#define AQ_DEBUG_DEBUG_H_

namespace Aq {
class Debug {
 public:
  enum Level { ERROR = 0, WARNING = 1, INFO = 2 };

  Debug(Level level, char* location, char* debug_code, char* debug_message,
        char* other_info);
  ~Debug();

  Debug(const Debug&) = delete;
  Debug(Debug&&) = delete;
  Debug& operator=(const Debug&) = delete;
  Debug& operator=(Debug&&) = delete;

 private:
  time_t timestamp_;
  Level level_;
  char* location_;
  char* debug_code_;
  char* debug_message_;
  char* other_info_;

  int errno_;
  char* errno_message_;

  void OutputMessage();
  std::string GetTimeString();
};
}  // namespace Aq

#endif