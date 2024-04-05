/// Copyright 2024 AQ authors, All Rights Reserved.
/// This program is licensed under the AQ License. You can find the AQ license in
/// the root directory.

#include "compiler/linked_list/linked_list.h"

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"
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
Compiler::LinkedList<DataType>::Iterator::Iterator(Node* node) : node_(node) {}
template <typename DataType>
Compiler::LinkedList<DataType>::Iterator::~Iterator() = default;
template <typename DataType>
DataType& Compiler::LinkedList<DataType>::Iterator::operator*() const {
  return node_->data;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator&
Compiler::LinkedList<DataType>::Iterator::operator++() {
  node_ = node_->location.second;
  return *this;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator&
Compiler::LinkedList<DataType>::Iterator::operator--() {
  node_ = node_->location.first;
  return *this;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::Iterator::operator++(int) {
  Iterator temp(*this);
  ++(*this);
  return temp;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::Iterator::operator--(int) {
  Iterator temp(*this);
  --(*this);
  return temp;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator&
Compiler::LinkedList<DataType>::Iterator::operator+=(std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    if (node_ == nullptr) {
      Debugger error(Debugger::Level::ERROR,
                     "Aq::Compiler::LinkedList::Iterator::operator+=",
                     "operator+=_IndexError", "Index out of range.", nullptr);
    }
    node_ = node_->location.second;
  }
  return *this;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator&
Compiler::LinkedList<DataType>::Iterator::operator-=(std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    node_ = node_->location.first;
  }
  return *this;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::Iterator::operator+(std::size_t n) const {
  Iterator new_iterator(*this);
  new_iterator += n;
  return new_iterator;
}
template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::Iterator::operator-(std::size_t n) const {
  Iterator new_iterator(*this);
  new_iterator -= n;
  return new_iterator;
}
template <typename DataType>
bool Compiler::LinkedList<DataType>::Iterator::operator==(
    const Iterator& other) const {
  return *this->node_ == other.node_;
}
template <typename DataType>
bool Compiler::LinkedList<DataType>::Iterator::operator!=(
    const Iterator& other) const {
  return !(*this->node_ == other->node_);
}

template <typename DataType>
void Compiler::LinkedList<DataType>::Insert(Iterator prev_node,
                                            DataType new_data) {
  if (prev_node.node_ == nullptr) {
    head_ = new Node(nullptr, head_, new_data);
    if (!head_) {
      Debugger error(Debugger::Level::ERROR, "Aq::Compiler::LinkedList::Insert",
                     "Insert_NewNodeError", "New node out of memory occurred.",
                     nullptr);
    }
  } else {
    prev_node.node_->location.second =
        new Node(prev_node.node_, prev_node->node_->location.second, new_data);
    if (!prev_node.node_->location.second) {
      Debugger error(Debugger::Level::ERROR, "Aq::Compiler::LinkedList::Insert",
                     "Insert_NewNodeError", "New node out of memory occurred.",
                     nullptr);
    }
  }
}

template <typename DataType>
void Compiler::LinkedList<DataType>::Remove(Iterator* delete_node) {
  if (delete_node->node_ == nullptr) {
    return;
  }
  if (delete_node->node_->location.first == nullptr) {
    head_ = delete_node->node_->location.second;
  } else {
    delete_node->node_->location.first->location.second =
        delete_node->node_->location.second;
  }
  if (delete_node->node_->location.second == nullptr) {
    tail_ = delete_node->node_->location.first;
  } else {
    delete_node->node_->location.second->location.first =
        delete_node->node_->location.first;
  }
  delete delete_node->node_;
}

template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::Begin() const {
  return Iterator(head_);
}

template <typename DataType>
typename Compiler::LinkedList<DataType>::Iterator
Compiler::LinkedList<DataType>::End() const {
  return Iterator(tail_);
}

}  // namespace Aq
