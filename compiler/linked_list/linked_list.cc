// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/linked_list/linked_list.h"

#include "debugger/debugger.h"

namespace Aq {
template <typename DataType>
Compiler::LinkedList<DataType>::LinkedList() = default;
template <typename DataType>
Compiler::LinkedList<DataType>::~LinkedList() {
  Node* current_node = head_;
  while (current_node != nullptr) {
    Node* next_node = current_node->location.second;
    Remove(current_node);
    current_node = next_node;
  }
  head_ = tail_ = nullptr;
}

template <typename DataType>
void Compiler::LinkedList<DataType>::Insert(Node* prev_node,
                                            DataType new_data) {
  Node* new_node = new Node;
  if (new_node == nullptr) {
    Debugger error_info(
        Debugger::Level::ERROR, "Aq::Compiler::Lexer::LinkedList::Insert",
        "Insert_MemoryError", "Memory allocation failed.", nullptr);
    return;
  }

  new_node->data = new_data;

  if (prev_node == nullptr) {
    new_node->location.first = nullptr;
    if (head_ == nullptr) {
      new_node->location.second = nullptr;
      tail_ = new_node;
    } else {
      new_node->location.second = head_;
    }
    head_ = new_node;
    return;
  }

  new_node->location.second = prev_node->location.second;
  new_node->location.first = prev_node;
  prev_node->location.second = new_node;
}

template <typename DataType>
void Compiler::LinkedList<DataType>::Remove(Node* delete_node) {
  if (delete_node == nullptr) {
    Debugger error_info(
        Debugger::Level::ERROR, "Aq::Compiler::Lexer::LinkedList::Remove",
        "Delete_RemoveError", "Try to remove a nullptr.", nullptr);
    return;
  }
  delete delete_node;
}

}  // namespace Aq
