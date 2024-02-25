// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aq/aq.h"

#include "compiler/compiler.h"

namespace Aq {
int Start(int argc, char *argv[]) {
  // TODO(Aq::Start): For testing purposes only, modifications will be made after other
  // components have been developed.
  Compiler compiler(argv[1]);

  return 0;
}
}  // namespace Aq

int main(int argc, char *argv[]) { return Aq::Start(argc, argv); }