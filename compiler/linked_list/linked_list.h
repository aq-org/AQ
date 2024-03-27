// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_
#define AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"

namespace Aq {

template <typename ValueType>
class Compiler::LinkedList {
 public:
  struct Node {
    Pair<Node*, Node*> location;
    ValueType data;
  };

  LinkedList();
  ~LinkedList();

  /*
  LinkedList& operator=(const LinkedList& other);

  allocator_type GetAllocator() const;
  ValueType* Front();
  const ValueType* Front() const;
  ValueType* Back();
  const ValueType* Back() const;
  iterator Begin();
  const_iterator Begin() const;
  iterator End();
  const_iterator End() const;
  bool Empty() const;
  size_type Size() const;
  void Clear();
  iterator Insert(iterator pos, const ValueType& value);
  template <class... Args>
  iterator Emplace(iterator pos, Args&&... args);
  iterator Erase(iterator pos);
  void PushFront(const ValueType& value);
  void PushBack(const ValueType& value);
  void PopFront();
  void PopBack();
  void Merge(MyList& other);
  void Splice(iterator pos, MyList& other, iterator first, iterator last);
  void Remove(const ValueType& value);
  template <class UnaryPredicate>
  void RemoveIf(UnaryPredicate pred);
  void Reverse();
  void Unique();
  template <class Compare>
  void Sort(Compare comp);
  */

  // Wait development.

 private:
  Node* head_;
};

}  // namespace Aq

#endif  // AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_