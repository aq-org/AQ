// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/aqvm.h"

#include "aqvm/base/logging.h"
#include "aqvm/memory/memory.h"

int Aqvm_StartVm(const char* FileName) {
  // TODO(Aqvm): Finish this function after completing AQVM development.
  AqvmBase_OutputLog("\"INFO\"", "\"Aqvm_StartVm_Start\"",
                     "\"Initializing Aqvm has been started.\"", NULL);

  if (AqvmMemory_CheckMemoryConditions() != 0) {
    AqvmBase_OutputLog("\"ERROR\"",
                       "\"Aqvm_StartVm_CheckMemoryConditionsError\"",
                       "\"Checking memory conditions met error.\"", NULL);
    return -1;
  }

  return 0;
}