// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/memory/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aqvm/memory/types.h"
#include "aqvm/runtime/debugger/debugger.h"

int AqvmMemory_CheckMemoryConditions() {
  int warning_count = 0;
  if (sizeof(aqint) != 4) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_IntLengthWarning",
        "The length requirement for the int type does not conform to the type "
        "definition.",
        NULL});
    ++warning_count;
  }
  if (sizeof(aqlong) != 8) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_LongLengthWarning",
        "The length requirement for the long type does not conform to the type "
        "definition.",
        NULL});
    ++warning_count;
  }
  if (sizeof(aqfloat) != 4) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_FloatLengthWarning",
        "The length requirement for the float type does not conform to the "
        "type definition.",
        NULL});
    ++warning_count;
  }
  if (sizeof(aqdouble) != 4) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_DoubleLengthWarning",
        "The length requirement for the double type does not conform to the "
        "type definition.",
        NULL});
    ++warning_count;
  }
  if (sizeof(aqchar) != 1) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_CharLengthWarning",
        "The length requirement for the char type does not conform to the type "
        "definition.",
        NULL});
    ++warning_count;
  }
  if (sizeof(aqbool) != 1) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryCheckMemoryConditions_BoolLengthWarning",
        "The length requirement for the bool type does not conform to the type "
        "definition.",
        NULL});
    ++warning_count;
  }

  return warning_count;
}

struct AqvmMemory_Memory* AqvmMemory_CreateMemory(void* data, void* type,
                                                  size_t size) {
  struct AqvmMemory_Memory* memory_ptr =
      (struct AqvmMemory_Memory*)malloc(sizeof(struct AqvmMemory_Memory));
  if (memory_ptr == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        2, "AqvmMemoryCreateMemory_MemoryAllocationFailure",
        "Failed to allocate memory.", NULL});
    return NULL;
  }

  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void AqvmMemory_FreeMemory(struct AqvmMemory_Memory* memory_ptr) {
  free(memory_ptr);
}

int AqvmMemory_SetType(const struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type) {
  if (memory == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemorySetType_NullMemoryPointer", "The memory pointer is NULL.",
        NULL});
    return -1;
  }

  if (memory->type == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemorySetType_NullTypePointer", "The type pointer is NULL.",
        NULL});
    return -2;
  }

  if (index > memory->size) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemorySetType_OutOfMemoryRange",
        "The index is out of memory range.", NULL});
    return -3;
  }

  if (type > 0x0F) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemorySetType_OutOfTypeRange", "The type is out of range.",
        NULL});
    return -4;
  }

  if (index % 2 != 0) {
    memory->type[index / 2] = (memory->type[index / 2] & 0xF0) | type;
  } else {
    memory->type[index / 2] = (memory->type[index / 2] & 0x0F) | (type << 4);
  }

  return 0;
}

uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index) {
  if (memory == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryGetType_NullMemoryPointer", "The memory pointer is NULL.",
        NULL});
    return 0x11;
  }

  if (memory->type == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryGetType_NullTypePointer", "The type pointer is NULL.",
        NULL});
    return 0x12;
  }

  if (index > memory->size) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryGetType_OutOfMemoryRange",
        "The index is out of memory range.", NULL});
    return 0x13;
  }

  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

int AqvmMemory_WriteData(struct AqvmMemory_Memory* memory, size_t index,
                         void* data_ptr, size_t size) {
  if (memory == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryWriteData_NullMemoryPointer",
        "The memory pointer is NULL.", NULL});
    return -1;
  }

  if (memory->type == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryWriteData_NullTypePointer", "The type pointer is NULL.",
        NULL});
    return -2;
  }

  if (index > memory->size) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryWriteData_OutOfMemoryRange",
        "The index is out of memory range.", NULL});
    return -3;
  }

  if (data_ptr == NULL) {
    AqvmRuntimeDebugger_OutputReport((struct AqvmRuntimeDebugger_DebugReport){
        1, "AqvmMemoryWriteData_NullDataPointer", "The data pointer is NULL.", NULL});
    return -4;
  }

  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);
  return 0;
}