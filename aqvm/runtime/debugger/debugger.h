// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_
#define AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_

#include <stdint.h>

// The struct stores information about the debug report. |type| is the type of
// the report. In |type|, 0 is INFO, 1 is WARNING, and 2 is ERROR.
// |type|, |code|, and |message| are necessary and shouldn't be set to NULL in
// common. |other_info| is a pointer to the other information and it can be set
// to NULL if it is not needed.
// NOTICE: If you need to use it, please use json format.
struct AqvmRuntimeDebugger_DebugReport {
  uint8_t type;
  char* code;
  char* message;
  char* other_info;
};

// Outputs |report| with time, errno and so on to printing to the console or
// other devices and writing to a log file. No return.
// NOTICE: The output is json format.
void AqvmRuntimeDebugger_OutputReport(
    struct AqvmRuntimeDebugger_DebugReport report);

// INTERNAL USE ONLY.
// Gets the current time in a string format based on ISO 8601 standard and
// stores it in |return_time|. No return.
// |return_time| length is at least 28 bytes.
// NOTICE: The output is json format.
void AqvmRuntimeDebugger_GetCurrentTime(char* return_time);

// INTERNAL USE ONLY.
// Formats |report| and returns type string accroding to the type in |report|.
// If code, message, or other_info are NULL, They will be set to the string
// "NULL".
// NOTICE: The output is json format.
const char* AqvmRuntimeDebugger_FormatReport(
    struct AqvmRuntimeDebugger_DebugReport* report);

#endif