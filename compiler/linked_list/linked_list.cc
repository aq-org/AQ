// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/linked_list/linked_list.h"

#include "debugger/debugger.h"

namespace Aq {
template <typename DataType>
Compiler::LinkedList<DataType>::LinkedList() : head_(nullptr), tail_(nullptr) {}
template <typename DataType>
Compiler::LinkedList<DataType>::~LinkedList() {
  while (head_ != nullptr) {
    Node* temp = head_;
    head_ = head_->location.second;
    delete temp;
  }
}

template <typename DataType>
Compiler::LinkedList<DataType>::Node::Node(Node* prev, Node* next,
                                           DataType data)
    : location(prev, next), data(data) {}

template <typename DataType>
Compiler::LinkedList<DataType>::Iterator::Iterator(Node* node) : node_(node) {}
template <typename DataType>
Compiler::LinkedList<DataType>::Iterator::~Iterator() = default;

template <typename DataType>
void Compiler::LinkedList<DataType>::Insert(Iterator* prev_node,
                                            DataType new_data) {
  Node* new_node;
  try {
    if (prev_node == nullptr) {
      head_ = new Node(nullptr, head_, new_data);
    } else {
      prev_node->node_->location.second = new Node(
          prev_node->node_, prev_node->node_->location.second, new_data);
    }
  } catch (std::bad_alloc& e) {
    throw Debugger(Debugger::Level::ERROR, "Aq::Compiler::LinkedList::Insert",
                   "Insert_NewNodeError", "New node out of memory occurred.",
                   nullptr);
  }
}

template <typename DataType>
void Compiler::LinkedList<DataType>::Remove(Iterator* delete_node) {
  if (delete_node->node_ == nullptr) {
    return;
  }
  if (delete_node->node_->location.first != nullptr) {
    delete_node->node_->location.first->location.second =
        delete_node->node_->location.second;
  } else {
    head_ = delete_node->node_->location.second;
  }
  if (delete_node->node_->location.second != nullptr) {
    delete_node->node_->location.second->location.first =
        delete_node->node_->location.first;
  } else {
    tail_ = delete_node->node_->location.first;
  }
  delete delete_node->node_;
}

template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator*
Compiler::LinkedList<DataType>::Begin() const {
  if (head_ == nullptr) {
    return nullptr;
  }
  return Iterator(head_);
}

template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator*
Compiler::LinkedList<DataType>::End() const {
  if (head_ == nullptr) {
    return nullptr;
  }
  return Iterator(tail_);
}

}  // namespace Aq
