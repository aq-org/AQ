// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include <stdio.h>

#include "aqvm/aqvm.h"
#include "aqvm/base/logging/logging.h"

int main(int argc, char *argv[]) {
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
    return -1;
  }

  if (Aqvm_StartVm(argv[1]) != 0) {
    AqvmBaseLogging_OutputLog("ERROR", "main_InitVmError",
                              "Starting Aqvm met error.", NULL);
    return -1;
  }

  AqvmBaseLogging_OutputLog("INFO", "main_End", "Aq main program has ended.",
                            NULL);
  return 0;
}