// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_AQVM_AQVM_H_
#define AQ_AQVM_AQVM_H_

// Starts a virtual machine with the given bytecode file name. Returns 0 for
// success, other values for errors.
int Aqvm_StartVm(const char* FileName);

#endif