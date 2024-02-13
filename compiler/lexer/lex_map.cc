// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/lex_map.h"

#include <cstddef>

#include "debug/debug.h"

namespace Aq {
namespace Compiler {
template <typename T>
LexMap<T>::LexMap(std::size_t init_capacity) {}
template <typename T>
LexMap<T>::~LexMap() {}

template <typename T>
LexMap<T>::PairList::~PairList() {
  while (head_ptr_ != nullptr) {
    Node* temp = head_ptr_;
    head_ptr_ = head_ptr_->next;
    delete temp;
  }
}

template <typename T>
unsigned int LexMap<T>::Hash(char* key) {
  unsigned int hash = 5381;
  while (*key) {
    // hash = hash * 33 + character
    hash = ((hash << 5) + hash) + (*key++);
  }
  return hash;
}

template <typename T>
void LexMap<T>::Insert(char* key, T value) {
  unsigned int hash = Hash(key);
  if (hash > capacity_) {
    if (Resize() == -1) {
      Debug error_info(
          Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::Insert",
          "Insert_MemoryError", "Memory allocation failed.", nullptr);
      return -1;
    }
  }
  Pair pair;
  pair.key = key;
  pair.value = value;
  pair_list_[hash].Append(pair);
}

template <typename T>
T* LexMap<T>::Find(char* key) {
  unsigned int hash = Hash(key);
  if (hash > capacity_) {
    return nullptr;
  }
  // TODO
}

template <typename T>
int LexMap<T>::Resize() {
  PairList* temp = pair_list_;
  pair_list_ = new PairList[capacity_ * 1.5];
  if (!pair_list_) {
    Debug error_info(Debug::Level::ERROR, "Aq::Compiler::Lexer::LexMap::Resize",
                     "Resize_MemoryError", "Memory allocation failed.",
                     nullptr);
    return -1;
  }
  for (int i = 0; i < capacity_; i++) {
    pair_list_[i] = temp[i];
  }
  return 0;
}
}  // namespace Compiler
}  // namespace Aq
