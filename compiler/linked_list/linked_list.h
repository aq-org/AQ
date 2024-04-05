// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_
#define AQ_COMPILER_LINKED_LIST_LINKED_LIST_H_

#include <cstddef>

#include "compiler/compiler.h"
#include "compiler/pair/pair.h"
#include "debugger/debugger.h"

namespace Aq {
/// \class LinkedList
/// \brief linked list
/// \tparam DataType
template <typename DataType>
class Compiler::LinkedList {
 public:
  LinkedList();
  ~LinkedList();

  LinkedList(const LinkedList&) = delete;
  LinkedList(LinkedList&&) noexcept = delete;
  LinkedList& operator=(const LinkedList&) = delete;
  LinkedList& operator=(LinkedList&&) noexcept = delete;

  /// \struct Node
  /// \brief linked list node
  /// \warning Node shouldn't be used by other classes.
  struct Node {
    /// \brief First is prev node, second is next node.
    Pair<Node*, Node*> location;

    /// \brief The data of the node.
    DataType data;
  };

  /// \class Iterator
  /// \brief The iterator type of the linked list.
  class Iterator {
   public:
    /// \fn Iterator
    /// @brief Creates and initialize a linked list iterator.
    /// @param node Node* Type
    Iterator(Node* node);
    ~Iterator();

    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) noexcept = default;
    Iterator& operator=(const Iterator&) = default;
    Iterator& operator=(Iterator&&) noexcept = default;

    DataType& operator*() const;
    Iterator& operator++();
    Iterator& operator--();
    Iterator operator++(int);
    Iterator operator--(int);
    Iterator& operator+=(std::size_t n);
    Iterator& operator-=(std::size_t n);
    Iterator operator+(std::size_t n) const;
    Iterator operator-(std::size_t n) const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    /// @brief The node of the iterator. The default value is `nullptr`.
    Node* node_ = nullptr;
  };

  /// \fn Insert
  /// \brief Inserts `new_data` after `prev_node`. If `prev_node` is `nullptr`,
  /// `new_data` will be inserted at the beginning of the list.
  /// \param prev_node Iterator Type
  /// \param new_data DataType Type
  void Insert(Iterator prev_node, DataType new_data);

  /// \fn Remove
  /// \brief Removes `delete_node` from the list.
  /// \param delete_node Iterator Type
  void Remove(Iterator* delete_node);

  /// \fn Clear
  /// \brief Removes all nodes from the list.
  void Clear();

  /// \fn Size
  /// \brief Returns the number of elements in the linked list.
  /// \return the beginning iterator of the linked list. Iterator Type.
  Iterator Begin() const;

  /// \fn End
  /// \return the end iterator of the linked list.
  /// \return the beginning iterator of the linked list. Iterator Type.
  Iterator End() const;

 private:
  /// @brief The head node pointer of the linked list.
  Node* head_ = nullptr;

  /// @brief The tail node pointer of the linked list.
  Node* tail_ = nullptr;
};

}  // namespace Aq

#endif