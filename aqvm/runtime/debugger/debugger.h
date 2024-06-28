// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_
#define AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_

#include <stdint.h>

// The struct stores information about the debug report. |type| is the type of
// the report. In |type|, 0 is info, 1 is warning, and 2 is error. |code| is a
// pointer to the debug code. |message| is a pointer to the error message.
// |other_info| is a pointer to the other information and it can be set to NULL
// if it is not needed.
struct AqvmRuntimeDebugger_DebugReport {
  uint8_t type;
  char* code;
  char* message;
  char* other_info;
};

// Outputs |report| by printing to the console or other devices and writing to a
// log file. No return.
void AqvmRuntimeDebugger_OutputReport(
    struct AqvmRuntimeDebugger_DebugReport report);

// Gets the current time in a string format and stores it in |return_time|.
// |return_time| stores the current time in a string format based on ISO 8601
// standard. No return.
void AqvmRuntimeDebugger_GetCurrentTime(char* return_time);

// Formats |report|. If code, message, or other_info are NULL, The NULL pointers
// in them will be set to |null_str|. |type| will be formatted to "INFO",
// "WARNING", "ERROR", etc. No return.
void AqvmRuntimeDebugger_FormatReport(
    struct AqvmRuntimeDebugger_DebugReport* report, char** type,
    const char* null_str);

#endif