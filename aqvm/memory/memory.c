// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "aqvm/memory/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aqvm/base/logging/logging.h"
#include "aqvm/memory/types.h"

int AqvmMemory_CheckMemoryConditions() {
  int warning_count = 0;
  if (sizeof(aqbyte) != 1) {
    AqvmBaseLogging_OutputLog(
        "WARNING", "AqvmMemory_CheckMemoryConditions_ByteLengthWarning",
        "The length requirement for the byte type does not conform to the type "
        "definition.",
        NULL);
    ++warning_count;
  }
  if (sizeof(aqint) != 4) {
    AqvmBaseLogging_OutputLog(
        "WARNING", "AqvmMemory_CheckMemoryConditions_IntLengthWarning",
        "The length requirement for the int type does not conform to the type "
        "definition.",
        NULL);
    ++warning_count;
  }
  if (sizeof(aqlong) != 8) {
    AqvmBaseLogging_OutputLog(
        "WARNING", "AqvmMemory_CheckMemoryConditions_LongLengthWarning",
        "The length requirement for the long type does not conform to the type "
        "definition.",
        NULL);
    ++warning_count;
  }
  if (sizeof(aqfloat) != 4) {
    AqvmBaseLogging_OutputLog(
        "WARNING", "AqvmMemory_CheckMemoryConditions_FloatLengthWarning",
        "The length requirement for the float type does not conform to the "
        "type definition.",
        NULL);
    ++warning_count;
  }
  if (sizeof(aqdouble) != 8) {
    AqvmBaseLogging_OutputLog(
        "WARNING", "AqvmMemory_CheckMemoryConditions_DoubleLengthWarning",
        "The length requirement for the double type does not conform to the "
        "type definition.",
        NULL);
    ++warning_count;
  }

  if (warning_count == 0) {
    AqvmBaseLogging_OutputLog("INFO",
                              "AqvmMemory_CheckMemoryConditions_CheckNormal",
                              "No memory conditions warning.", NULL);
  }

  return warning_count;
}

struct AqvmMemory_Memory* AqvmMemory_InitializeMemory(void* data, void* type,
                                                      size_t size) {
  AqvmBaseLogging_OutputLog("INFO", "AqvmMemory_InitializeMemory_Start",
                            "Memory initialization started.", NULL);

  struct AqvmMemory_Memory* memory_ptr =
      (struct AqvmMemory_Memory*)malloc(sizeof(struct AqvmMemory_Memory));
  if (memory_ptr == NULL) {
    AqvmBaseLogging_OutputLog(
        "ERROR", "AqvmMemory_InitializeMemory_MemoryAllocationFailure",
        "Failed to allocate memory.", NULL);
    return NULL;
  }

  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void AqvmMemory_FreeMemory(struct AqvmMemory_Memory* memory_ptr) {
  AqvmBaseLogging_OutputLog("INFO", "AqvmMemory_FreeMemory_Start",
                            "Memory deallocation started.", NULL);

  free(memory_ptr);
}

int AqvmMemory_SetType(const struct AqvmMemory_Memory* memory, size_t index,
                       uint8_t type) {
  if (memory == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_SetType_NullMemoryPointer",
                              "The memory pointer is NULL.", NULL);
    return -1;
  }
  if (memory->type == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_SetType_NullTypePointer",
                              "The type pointer is NULL.", NULL);
    return -2;
  }
  if (index > memory->size) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_SetType_OutOfMemoryRange",
                              "The index is out of memory range.", NULL);
    return -3;
  }
  if (type > 0x0F) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_SetType_OutOfTypeRange",
                              "The type is out of range.", NULL);
    return -4;
  }

  // Sets the type of the data at |index| bytes in memory.
  // Since Aqvm stores type data occupying 4 bits and uint8_t occupying 8 bits,
  // each uint8_t type location stores two type data. The storage locations
  // (high 4 bits, low 4 bits) are set according to the parity of |index|. Even
  // numbers are stored in the high bits of (|index| / 2) and odd numbers are
  // stored in the low bits of (|index| / 2).
  if (index % 2 != 0) {
    memory->type[index / 2] = (memory->type[index / 2] & 0xF0) | type;
  } else {
    memory->type[index / 2] = (memory->type[index / 2] & 0x0F) | (type << 4);
  }

  return 0;
}

uint8_t AqvmMemory_GetType(struct AqvmMemory_Memory* memory, size_t index) {
  if (memory == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_GetType_NullMemoryPointer",
                              "The memory pointer is NULL.", NULL);
    return 0x11;
  }
  if (memory->type == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_GetType_NullTypePointer",
                              "The type pointer is NULL.", NULL);
    return 0x12;
  }
  if (index > memory->size) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_GetType_OutOfMemoryRange",
                              "The index is out of memory range.", NULL);
    return 0x13;
  }

  // Gets the type of the data at |index| bytes in memory.
  // Since Aqvm stores type data occupying 4 bits and uint8_t occupying 8 bits,
  // each uint8_t type location stores two type data. The storage locations
  // (high 4 bits, low 4 bits) are set according to the parity of |index|. Even
  // numbers are stored in the high bits of (|index| / 2) and odd numbers are
  // stored in the low bits of (|index| / 2).
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

int AqvmMemory_WriteData(struct AqvmMemory_Memory* memory, size_t index,
                         void* data_ptr, size_t size) {
  if (memory == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_WriteData_NullMemoryPointer",
                              "The memory pointer is NULL.", NULL);
    return -1;
  }
  if (memory->type == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_WriteData_NullTypePointer",
                              "The type pointer is NULL.", NULL);
    return -2;
  }
  if (index > memory->size) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_WriteData_OutOfMemoryRange",
                              "The index is out of memory range.", NULL);
    return -3;
  }
  if (data_ptr == NULL) {
    AqvmBaseLogging_OutputLog("ERROR", "AqvmMemory_WriteData_NullDataPointer",
                              "The data pointer is NULL.", NULL);
    return -4;
  }

  // Since void* does not have a specific size, pointer moves need to be
  // converted before moving.
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}