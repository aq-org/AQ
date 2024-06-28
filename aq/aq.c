// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include <stdio.h>

#include "aqvm/aqvm.h"

#include "aqvm/runtime/debugger/debugger.h"

int main(int argc, char *argv[]) {
  // TODO(Aqvm): Finish this function after completing AQVM development.
  if(Aqvm_InitVm() != 0){
    return -1;
  }

  AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){0, NULL, NULL, NULL});

  return 0;
}