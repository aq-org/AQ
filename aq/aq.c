// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include <stdio.h>

#include "aqvm/aqvm.h"
#include "aqvm/base/logging/logging.h"

int main(int argc, char *argv[]) {
if(Aq_InitilizeAq() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "main_InitilizeAqError",
                              "Initializing Aq met error.", NULL);
    return -1;
  }

  // TODO(Aqvm): Finish this function after completing AQVM development.
  AqvmBaseLogging_OutputLog("INFO", "main_Start",
                            "Aq main program has been started.", NULL);

  // TODO(Aq): Before the official release, remove the judgment logic for
  // command line arguments and design a dedicated component to parse command
  // line arguments.
  if (argc < 2) {
    AqvmBaseLogging_OutputLog("ERROR", "main_ArgsError",
                              "Please provide a file name as an argument.",
                              NULL);
    return -2;
  }

  if (Aqvm_StartVm(argv[1]) != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "main_InitVmError",
                              "Starting Aqvm met error.", NULL);
    return -1;
  }

  AqvmBaseLogging_OutputLog("INFO", "main_End", "Aq main program has ended.",
                            NULL);

  if(Aq_CloseAq() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "main_CloseAqError",
                              "Closing Aq met error.", NULL);
    return -1;    
  }

  return 0;
}

int Aq_InitilizeAq() {
  if (Aqvm_InitilizeVm() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "Aq_InitilizeAq_InitilizeVmError",
                              "Initializing Aqvm met error.", NULL);
    return -1;
  }

  AqvmBaseLogging_OutputLog("INFO", "Aq_InitilizeAq_InitilizeVmNormal",
                            "Initializing Aqvm normal.", NULL);
  return 0;
}

int Aq_CloseAq() {
  if (Aqvm_CloseVm() != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "Aq_CloseAq_CloseVmError",
                              "Closing Aqvm met error.", NULL);
    return -1;
  }

  AqvmBaseLogging_OutputLog("INFO", "Aq_CloseAq_CloseVmNormal",
                            "Closing Aqvm normal.", NULL);
  return 0;
}