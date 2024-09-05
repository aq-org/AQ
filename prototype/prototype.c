// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Memory* memory;

#define GET_SIZE(x)  \
  ((x) == 0x00   ? 0 \
   : (x) == 0x01 ? 1 \
   : (x) == 0x02 ? 4 \
   : (x) == 0x03 ? 8 \
   : (x) == 0x04 ? 4 \
   : (x) == 0x02 ? 8 \
                 : 0)

#define GET_TYPE(type_code, ptr)          \
  ((type_code) == 0x01   ? (int8_t*)(ptr) \
   : (type_code) == 0x02 ? (int*)(ptr)    \
   : (type_code) == 0x03 ? (long*)(ptr)   \
   : (type_code) == 0x04 ? (float*)(ptr)  \
   : (type_code) == 0x05 ? (double*)(ptr) \
                         : (int8_t*)(ptr))

struct Memory {
  uint8_t* type;
  void* data;
  size_t size;
};

struct Memory* InitializeMemory(void* data, void* type, size_t size) {
  struct Memory* memory_ptr = (struct Memory*)malloc(sizeof(struct Memory));

  memory_ptr->data = data;
  memory_ptr->type = type;
  memory_ptr->size = size;

  return memory_ptr;
}

void FreeMemory(struct Memory* memory_ptr) { free(memory_ptr); }

int SetType(const struct Memory* memory, size_t index, uint8_t type) {
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

int WriteData(struct Memory* memory, size_t index, void* data_ptr,
              size_t size) {
  memcpy((void*)((uintptr_t)memory->data + index), data_ptr, size);

  return 0;
}

uint8_t GetType(const struct Memory* memory, size_t index) {
  if (index % 2 != 0) {
    return memory->type[index / 2] & 0x0F;
  } else {
    return (memory->type[index / 2] & 0xF0) >> 4;
  }
}

void* Get1Parament(void* ptr, size_t* first) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get2Parament(void* ptr, size_t* first, size_t* second) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get3Parament(void* ptr, size_t* first, size_t* second, size_t* third) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *third = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

void* Get4Parament(void* ptr, size_t* first, size_t* second, size_t* third,
                   size_t* fourth) {
  int state = 0;
  int size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *first = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *second = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *third = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  state = 0;
  size = 0;
  while (state == 0) {
    if (*(size_t*)ptr < 255) {
      *fourth = 255 * size + *(size_t*)ptr;
      state = 1;
    }
    ptr = (void*)((uintptr_t)ptr + 1);
    ++size;
  }
  return ptr;
}

int NOP() { return 0; }
int LOAD(size_t ptr, size_t operand) {
  WriteData(memory, operand, (void*)((uintptr_t)memory->data + ptr),
            GET_SIZE(GetType(memory, operand)));
  return 0;
}
int STORE(size_t ptr, size_t operand) {
  memcpy(*((void**)((uintptr_t)memory->data + ptr)),
         (void*)((uintptr_t)memory->data + operand),
         GET_SIZE(GetType(memory, operand)));
  return 0;
}
int NEW(size_t ptr, size_t size) {
  void* data = malloc((size_t)(*(GET_TYPE(
      GetType(memory, size), (void*)((uintptr_t)memory->data + size)))));
  WriteData(memory, ptr, &data, GET_SIZE(GetType(memory, size)));
  return 0;
}
int FREE(size_t ptr) {
  free(*((void**)((uintptr_t)memory->data + ptr)));
  return 0;
}
int SIZE() { return 0; }
int ADD(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) +
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int SUB(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) -
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int MUL(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) *
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int DIV(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) /
      (*(GET_TYPE(GetType(memory, operand2),
                  (void*)((uintptr_t)memory->data + operand2))));
  return 0;
}
int REM(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) %
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int NEG(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      -*(GET_TYPE(GetType(memory, operand1),
                  (void*)((uintptr_t)memory->data + operand1)));
  return 0;
}
int SHL(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1)))
      << *(GET_TYPE(GetType(memory, operand2),
                    (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int SHR(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) >>
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int SAR(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) >>
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int IF() {}
int AND(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) &
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int OR(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) |
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int XOR(size_t result, size_t operand1, size_t operand2) {
  *(GET_TYPE(GetType(memory, result),
             (void*)((uintptr_t)memory->data + result))) =
      *(GET_TYPE(GetType(memory, operand1),
                 (void*)((uintptr_t)memory->data + operand1))) ^
      *(GET_TYPE(GetType(memory, operand2),
                 (void*)((uintptr_t)memory->data + operand2)));
  return 0;
}
int CMP(size_t result, size_t opcode, size_t operand1, size_t operand2) {
  switch (*(GET_TYPE(GetType(memory, opcode),
                     (void*)((uintptr_t)memory->data + opcode)))) {
    case 0x00:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) ==
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    case 0x01:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) !=
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    case 0x02:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) <
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    case 0x03:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) <=
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    case 0x04:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) >
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    case 0x05:
      *(GET_TYPE(GetType(memory, result),
                 (void*)((uintptr_t)memory->data + result))) =
          *(GET_TYPE(GetType(memory, operand1),
                     (void*)((uintptr_t)memory->data + operand1))) >=
          *(GET_TYPE(GetType(memory, operand2),
                     (void*)((uintptr_t)memory->data + operand2)));
      break;
    default:
      break;
  }
  return 0;
}
int INVOKE() {}
int RETURN() {}
int GOTO() {}
int THROW() {}
int WIDE() {}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  FILE* bytecode = fopen(argv[1], "r");
  if (bytecode == NULL) {
    printf("Error: Could not open file %s\n", argv[1]);
    return -2;
  }

  fseek(bytecode, 0, SEEK_END);
  size_t bytecode_size = ftell(bytecode);
  void* bytecode_file = malloc(bytecode_size);
  void* bytecode_begin = bytecode_file;
  void* bytecode_end = (void*)((uintptr_t)bytecode_file + bytecode_size);
  fread(bytecode_file, 1, bytecode_size, bytecode);
  fseek(bytecode, 0, SEEK_SET);
  fclose(bytecode);

  if (((char*)bytecode_file)[0] != 0x41 || ((char*)bytecode_file)[1] != 0x51 ||
      ((char*)bytecode_file)[2] != 0x42 || ((char*)bytecode_file)[3] != 0x43) {
    printf("Error: Invalid bytecode file\n");
    return -3;
  }

  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);

  size_t memory_size = *(size_t*)bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + 8);
  void* type = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size / 2);
  void* data = bytecode_file;
  bytecode_file = (void*)((uintptr_t)bytecode_file + memory_size);
  memory = InitializeMemory(data, type, memory_size);
  size_t first, second, result, operand1, operand2, opcode;
  while (bytecode_file < bytecode_end) {
    switch (*(uint8_t*)bytecode_file) {
      case 0x00:
        NOP();
        break;
      case 0x01:
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        LOAD(first, second);
        break;
      case 0x02:
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        STORE(first, second);
        break;
      case 0x03:
        bytecode_file = Get2Parament(bytecode_file, &first, &second);
        NEW(first, second);
        break;
      case 0x04:
        bytecode_file = Get1Parament(bytecode_file, &first);
        FREE(first);
        break;
      case 0x05:
        SIZE();
        break;
      case 0x06:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        ADD(result, operand1, operand2);
        break;
      case 0x07:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SUB(result, operand1, operand2);
        break;
      case 0x08:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        MUL(result, operand1, operand2);
        break;
      case 0x09:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        DIV(result, operand1, operand2);
        break;
      case 0x0A:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        REM(result, operand1, operand2);
        break;
      case 0x0B:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        NEG(result, operand1, operand2);
        break;
      case 0x0C:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SHL(result, operand1, operand2);
        break;
      case 0x0D:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SHR(result, operand1, operand2);
        break;
      case 0x0E:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        SAR(result, operand1, operand2);
        break;
      case 0x0F:
        IF();
        break;
      case 0x10:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        AND(result, operand1, operand2);
        break;
      case 0x11:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        OR(result, operand1, operand2);
        break;
      case 0x12:
        bytecode_file =
            Get3Parament(bytecode_file, &result, &operand1, &operand2);
        XOR(result, operand1, operand2);
        break;
      case 0x13:
        bytecode_file =
            Get4Parament(bytecode_file, &result, &opcode, &operand1, &operand2);
        CMP(result, opcode, operand1, operand2);
        break;
      case 0x14:
        INVOKE();
        break;
      case 0x15:
        RETURN();
        break;
      case 0x16:
        GOTO();
        break;
      case 0x17:
        THROW();
        break;
      case 0xFF:
        WIDE();
        break;
      default:
        break;
    }
  }

  printf("Program finished");
  FreeMemory(memory);
  free(bytecode_begin);
  return 0;
}