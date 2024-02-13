// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#ifndef AQ_COMPILER_LEXER_LEX_MAP_H_
#define AQ_COMPILER_LEXER_LEX_MAP_H_

#include <cstddef>

namespace Aq {
namespace Compiler {
template <typename T>
class LexMap {
 public:
  LexMap(std::size_t init_capacity = 4294967295);
  ~LexMap();

  void Insert(char* key, T value);
  T* Find(char* key);

 private:
  struct Pair {
    char* key;
    T* value;
  };
  class PairList {
   public:
    PairList() : head_ptr_(nullptr){};
    ~PairList();

    // Prepend a new pair to the list.
    void Prepend(Pair value) {
      Node* new_node = new Node(value);
      new_node->next = head_ptr_;
      head_ptr_ = new_node;
    }

    // Append a new pair to the list. It is not recommended to use it when
    // dealing with large amounts of data.
    void Append(Pair value) {
      if (head_ptr_ == nullptr) {
        head_ptr_ = new Node(value);
      } else {
        Node* temp = head_ptr_;
        while (temp->next != nullptr) {
          temp = temp->next;
        }
        temp->next = new Node(value);
      }
    }

   private:
    struct Node {
      Pair* data;
      Node* next;
      Node(Pair* pair) : data(pair), next(nullptr){};
    };

    Node* head_ptr_;
  };

  std::size_t capacity_;
  PairList* pair_list_;

  static unsigned int Hash(char* key);

  int Resize();
};
}  // namespace Compiler
}  // namespace Aq

#endif