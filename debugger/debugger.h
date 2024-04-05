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

  /// \fn Debugger
  /// \brief Output a debug message.
  /// \param level Level Type. The debug message's level.
  /// \param location `const char*` Type. The debug message's location.
  /// \param debug_code `const char*` Type. The debug message's code.
  /// \param debug_message `const char*` Type. The debug message's content.
  /// \param other_info `const char*` Type. Optional. The default value is
  /// `nullptr`. The debug message's other information.
  /// \note Output a debug message. If `other_info` has no content, `nullptr`
  /// can be filled in. Others must be filled in with strings and cannot be
  /// `nullptr`.
  Debugger(Level level, const char* location, const char* debug_code,
           const char* debug_message, const char* other_info = nullptr);
  ~Debugger();

  Debugger(const Debugger&) = default;
  Debugger(Debugger&&) = default;
  Debugger& operator=(const Debugger&) = default;
  Debugger& operator=(Debugger&&) = default;

 private:
  /// \brief The debug happened time.
  time_t timestamp_ = 0;

  /// \brief The debug message's level.
  Level level_;

  /// \brief The debug happened location.
  const char* location_;

  /// \brief The debug message's code.
  const char* debug_code_;

  /// \brief The debug message's content.
  const char* debug_message_;

  /// \brief The debug message's other information. Default value is `nullptr`.
  const char* other_info_ = nullptr;

  /// \brief The debug happened errno.
  int errno_ = 0;
  /// \brief The debug happened errno message.
  const char* errno_message_;

  /// \fn OutputMessage
  /// \brief Output a debug message.
  void OutputMessage() const;

  /// \fn GetTimeString
  /// \brief Get the current time string. Based on ISO 8601 standard.
  /// \return The current time string. std::string Type.
  std::string GetTimeString() const;
};
}  // namespace Aq

#endif