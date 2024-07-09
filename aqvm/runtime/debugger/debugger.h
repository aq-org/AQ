// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_
#define AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_

#include <stdint.h>

// Outputs |report| with time, errno and so on to printing to the console or
// other devices and writing to a log file. No return.
// |type|, |code|, and |message| are necessary and shouldn't be set to NULL in
// common. But |other_info| can be set to NULL if it is not needed.
// In general, |type| should be "ERROR", "WARNING" or "INFO". |code| should be a
// full function name plus a concise description of the error, separated by
// underscores (e.g., AqvmRuntimeDebugger_OutputReport_TestInfo). |message|
// should be a detailed and accurate description. |other_info| on the other hand
// should be an additional information to the current report (e.g. system
// information).
// NOTICE: If you need to use the function, please use json format. The output
// is json format. For example, AqvmRuntimeDebugger_OutputReport("\"type\"",
// "\"code\"", "\"message\"", "\"other_info\"");
void AqvmRuntimeDebugger_OutputReport(const char* type, const char* code,
                                      const char* message,
                                      const char* other_info);

#endif