// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/aqvm.h"

#include "aqvm/base/base.h"
#include "aqvm/base/logging/logging.h"
#include "aqvm/memory/memory.h"

int Aqvm_InitilizeVm() {
  if (AqvmBase_InitilizeBase() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "Aqvm_StartVm_InitilizeBaseError",
                              "Initializing base error.", NULL);
    return -1;
  }

  AqvmBaseLogging_OutputLog("INFO", "Aqvm_StartVm_InitilizeBaseNormal",
                            "Initializing base normal.", NULL);
  return 0;
}

int Aqvm_CloseVm() {
  if (AqvmBase_CloseBase() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "Aqvm_StartVm_CloseBaseError",
                              "Closing base error.", NULL);
    return -3;
  }

  AqvmBaseLogging_OutputLog("INFO", "Aqvm_StartVm_CloseBaseNoraml",
                            "Closing base normal.", NULL);
  return 0;
}

int Aqvm_StartVm(const char* FileName) {
  // TODO(Aqvm): Finish this function after completing AQVM development.
  AqvmBaseLogging_OutputLog("INFO", "Aqvm_StartVm_Start",
                            "Initializing Aqvm has been started.", NULL);

  if (AqvmMemory_CheckMemoryConditions() != 0) {
    AqvmBaseLogging_OutputLog("ERROR",
                              "Aqvm_StartVm_CheckMemoryConditionsError",
                              "Checking memory conditions met error.", NULL);
    return -2;
  }

  return 0;
}