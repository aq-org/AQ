// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_
#define AQ_AQVM_RUNTIME_DEBUGGER_DEBUGGER_H_

#include <stdarg.h>

// Reports an error. It needs |error_code| and |error_message| and outputs them
// with other information. No return.
void AqvmRuntimeDebugger_ReportError(const char* error_code,
                                     const char* error_message, ...);

// Reports a warning. It needs |warning_code| and |warning_message| and outputs
// them with other information. No return.
void AqvmRuntimeDebugger_ReportWarning(const char* warning_code,
                                       const char* warning_message, ...);

// Reports an info. It needs |info_code| and |info_message| and outputs them
// with other information. No return.
void AqvmRuntimeDebugger_ReportInfo(const char* info_code,
                                    const char* info_message, ...);

// Outputs |debugg_message|. No return.
void AqvmRuntimeDebugger_Output(const char* debug_message);

#endif