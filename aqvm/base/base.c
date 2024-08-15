// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/base/base.h"

#include "aqvm/base/logging/logging.h"
#include "aqvm/base/io/io.h"
#include "aqvm/base/threading/threading.h"
#include "aqvm/base/time/time.h"

int AqvmBase_InitilizeBase() {
  if (AqvmBaseIo_InitializeIo() != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
}

int AqvmBase_CloseBase() {
  if (AqvmBaseIo_CloseIo() != 0) {
    // TODO(logging)
    return -1;
  }
  return 0;
}