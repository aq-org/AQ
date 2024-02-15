// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "debugger/debugger.h"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>

namespace Aq {
Debugger::Debugger(Level level, const char* location, const char* debug_code,
             const char* debug_message, const char* other_info) {
  errno_ = errno;
  errno_message_ = std::strerror(errno_);
  errno = 0;

  timestamp_ = std::time(nullptr);
  level_ = level;
  location_ = location;
  debug_code_ = debug_code;
  debug_message_ = debug_message;
  other_info_ = other_info;
  OutputMessage();
}
Debugger::~Debugger() = default;

void Debugger::OutputMessage() {
  std::string time_string = "Time:\"" + GetTimeString() + "\"";
  std::string level_string;
  switch (level_) {
    case ERROR:
      level_string = "Level:\"ERROR\"";
      break;
    case WARNING:
      level_string = "Level:\"WARNING\"";
      break;
    case INFO:
      level_string = "Level:\"INFO\"";
      break;
    default:
      level_string = "Level:NULL";
  }
  std::string location_string = "Location:\"" + std::string(location_) + "\"";
  std::string code_string = "Code:\"" + std::string(debug_code_) + "\"";
  std::string message_string =
      "Message:\"" + std::string(debug_message_) + "\"";
  std::string errno_string = "{Errno:" + std::to_string(errno_) +
                             ",Errno_Message:\"" + std::string(errno_message_) +
                             "\"}";
  std::string other_info_string;
  if (other_info_ != nullptr) {
    other_info_string = ",Other_Info:{" + std::string(other_info_) + "}";
  }

  std::cerr << "{" << time_string << "," << level_string << ","
            << location_string << "," << code_string << "," << message_string
            << "," << errno_string << other_info_string << "}" << std::endl;
}

std::string Debugger::GetTimeString() {
  if (timestamp_ != -1) {
    std::tm local_time_info = *std::localtime(&timestamp_);
    std::tm utc_time_info = *std::gmtime(&timestamp_);

    std::stringstream date_time;
    char date_time_buffer[20];
    std::strftime(date_time_buffer, 20, "%Y-%m-%dT%H:%M:%S", &local_time_info);
    date_time << date_time_buffer;

    int offset =
        std::difftime(mktime(&local_time_info), mktime(&utc_time_info));

    char time_zone_offset_buffer[12];
    if (offset != 0) {
      snprintf(time_zone_offset_buffer, 12, "%+03d:%02d",
               std::abs(offset / 3600), std::abs((offset % 3600) / 60));
    } else {
      time_zone_offset_buffer[0] = 'Z';
      time_zone_offset_buffer[1] = '\0';
    }

    date_time << time_zone_offset_buffer;
    return "Time:\"" + date_time.str() + "\"";
  } else {
    int time_errno = errno;
    std::string error_message =
        time_errno != 0
            ? "{Errno:" + std::to_string(time_errno) +
                  ", Errno_Message:" + std::strerror(time_errno) + "}"
            : "Errno:null";
    return "Time:[\"Failed to get time.\"," + error_message + "]";
  }
}
}  // namespace Aq
