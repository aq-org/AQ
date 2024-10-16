// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Aq {
template <typename T1, typename T2>
struct Pair {
  T1 first;
  T2 second;

  Pair(T1 _first, T2 _second);
};

template <typename T1, typename T2>
Pair<T1, T2>::Pair(T1 _first, T2 _second) : first(first), second(second) {}

template <typename DataType>
class LinkedList {
 public:
  LinkedList();
  ~LinkedList();

  LinkedList(const LinkedList&) = delete;
  LinkedList(LinkedList&&) noexcept = delete;
  LinkedList& operator=(const LinkedList&) = delete;
  LinkedList& operator=(LinkedList&&) noexcept = delete;

  struct Node {
    Pair<Node*, Node*> location;

    DataType data;
  };

  class Iterator {
   public:
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
    Node* node_ = nullptr;
  };

  void Insert(Iterator prev_node, DataType new_data);

  void Remove(Iterator* delete_node);

  void Clear();

  Iterator Begin() const;

  Iterator End() const;

 private:
  Node* head_ = nullptr;

  Node* tail_ = nullptr;
};

template <typename DataType>
LinkedList<DataType>::LinkedList() : head_(nullptr), tail_(nullptr) {}
template <typename DataType>
LinkedList<DataType>::~LinkedList() {
  while (head_ != nullptr) {
    Node* temp = head_;
    head_ = head_->location.second;
    delete temp;
  }
}

template <typename DataType>
LinkedList<DataType>::Iterator::Iterator(Node* node) : node_(node) {}
template <typename DataType>
LinkedList<DataType>::Iterator::~Iterator() = default;
template <typename DataType>
DataType& LinkedList<DataType>::Iterator::operator*() const {
  return node_->data;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator&
LinkedList<DataType>::Iterator::operator++() {
  node_ = node_->location.second;
  return *this;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator&
LinkedList<DataType>::Iterator::operator--() {
  node_ = node_->location.first;
  return *this;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator
LinkedList<DataType>::Iterator::operator++(int) {
  Iterator temp(*this);
  ++(*this);
  return temp;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator
LinkedList<DataType>::Iterator::operator--(int) {
  Iterator temp(*this);
  --(*this);
  return temp;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator&
LinkedList<DataType>::Iterator::operator+=(std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    if (node_ == nullptr) {
    }
    node_ = node_->location.second;
  }
  return *this;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator&
LinkedList<DataType>::Iterator::operator-=(std::size_t n) {
  for (std::size_t i = 0; i < n; ++i) {
    node_ = node_->location.first;
  }
  return *this;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator
LinkedList<DataType>::Iterator::operator+(std::size_t n) const {
  Iterator new_iterator(*this);
  new_iterator += n;
  return new_iterator;
}
template <typename DataType>
typename LinkedList<DataType>::Iterator
LinkedList<DataType>::Iterator::operator-(std::size_t n) const {
  Iterator new_iterator(*this);
  new_iterator -= n;
  return new_iterator;
}
template <typename DataType>
bool LinkedList<DataType>::Iterator::operator==(const Iterator& other) const {
  return *this->node_ == other.node_;
}
template <typename DataType>
bool LinkedList<DataType>::Iterator::operator!=(const Iterator& other) const {
  return !(*this->node_ == other->node_);
}

template <typename DataType>
void LinkedList<DataType>::Insert(Iterator prev_node, DataType new_data) {
  if (prev_node.node_ == nullptr) {
    head_ = new Node(nullptr, head_, new_data);
    if (!head_) {
    }
  } else {
    prev_node.node_->location.second =
        new Node(prev_node.node_, prev_node->node_->location.second, new_data);
    if (!prev_node.node_->location.second) {
    }
  }
}

template <typename DataType>
void LinkedList<DataType>::Remove(Iterator* delete_node) {
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
void LinkedList<DataType>::Clear() {
  while (head_ != nullptr) {
    Node* temp = head_;
    head_ = head_->location.second;
    delete temp;
  }
}

template <typename DataType>
typename LinkedList<DataType>::Iterator LinkedList<DataType>::Begin() const {
  return Iterator(head_);
}

template <typename DataType>
typename LinkedList<DataType>::Iterator LinkedList<DataType>::End() const {
  return Iterator(tail_);
}

template <typename ArrayType>
class DynArray {
 public:
  DynArray(std::size_t InitCapacity = 1);
  ~DynArray();

  DynArray(const DynArray&) = default;
  DynArray(DynArray&&) noexcept = default;
  DynArray& operator=(const DynArray&) = default;
  DynArray& operator=(DynArray&&) noexcept = default;

  ArrayType& operator[](std::size_t index);

  void Insert(ArrayType data);

  void Remove(std::size_t index);

  int Resize(std::size_t new_capacity = 0);

  std::size_t Size() const;

  void Clear();

  class Iterator {
   public:
    using iterator_category = std::random_access_iterator_tag;

    Iterator operator+(std::size_t size) const;
    Iterator& operator++();
    Iterator operator++(int);
    Iterator operator-(std::size_t size) const;
    std::size_t operator-(const Iterator& other) const;
    Iterator& operator--();
    Iterator operator--(int);
    Iterator& operator+=(std::size_t size);
    Iterator& operator-=(std::size_t size);
    ArrayType& operator*() const;
    ArrayType* operator->() const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;
    bool operator<(const Iterator& other) const;
    bool operator<=(const Iterator& other) const;
    bool operator>(const Iterator& other) const;
    bool operator>=(const Iterator& other) const;

   private:
    Iterator(DynArray<ArrayType>* array, std::size_t index = 0);

    ~Iterator();

    DynArray<ArrayType>* array_;

    std::size_t index_;
  };

  Iterator Begin();

  Iterator End();

 private:
  ArrayType* data_;

  std::size_t capacity_;

  std::size_t size_;
};

template <typename ArrayType>
DynArray<ArrayType>::DynArray(std::size_t InitCapacity) {
  data_ = new ArrayType[InitCapacity];
  if (data_ != nullptr) {
    capacity_ = InitCapacity;
  } else {
    capacity_ = 0;
  }
  size_ = 0;
}

template <typename ArrayType>
DynArray<ArrayType>::~DynArray() {
  delete[] data_;
}

template <typename ArrayType>
ArrayType& DynArray<ArrayType>::operator[](std::size_t index) {
  return data_[index];
}

template <typename ArrayType>
void DynArray<ArrayType>::Insert(ArrayType data) {
  if (capacity_ == 0 && Resize(1) == -1) {
    return;
  }
  if (size_ > capacity_) {
    return;
  }
  if (size_ == capacity_) {
    if (Resize(capacity_ * 2) != 0) {
      return;
    }
  }
  data_[size_] = data;
  ++size_;
}

template <typename ArrayType>
int DynArray<ArrayType>::Resize(std::size_t new_capacity) {
  if (new_capacity == 0) {
    ++capacity_;
  } else {
    capacity_ = new_capacity;
  }
  ArrayType* new_data = new ArrayType[capacity_];
  if (new_data == nullptr) {
    return -1;
  }
  for (std::size_t i = 0; i < size_ && i < capacity_; ++i) {
    new_data[i] = data_[i];
  }
  delete[] data_;
  data_ = new_data;
  return 0;
}

template <typename ArrayType>
std::size_t DynArray<ArrayType>::Size() const {
  return size_;
}

template <typename ArrayType>
void DynArray<ArrayType>::Remove(std::size_t index) {
  for (std::size_t i = index; i < size_ - 1; ++i) {
    data_[i] = data_[i + 1];
  }
  --size_;
}

template <typename ArrayType>
void DynArray<ArrayType>::Clear() {
  delete[] data_;
  data_ = nullptr;
  capacity_ = 0;
  size_ = 0;
}

template <typename ArrayType>
DynArray<ArrayType>::Iterator::Iterator(DynArray<ArrayType>* array,
                                        std::size_t index)
    : array_(array), index_(index) {
  if (index_ >= array_->size_) {
  }
}
template <typename ArrayType>
DynArray<ArrayType>::Iterator::~Iterator() = default;
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator DynArray<ArrayType>::Iterator::operator+(
    std::size_t size) const {
  return Iterator(array_, index_ + size);
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator&
DynArray<ArrayType>::Iterator::operator++() {
  ++index_;
  if (index_ >= array_->size_) {
  }
  return *this;
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator
DynArray<ArrayType>::Iterator::operator++(int) {
  Iterator temp(*this);
  ++(*this);
  return temp;
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator DynArray<ArrayType>::Iterator::operator-(
    std::size_t size) const {
  return Iterator(array_, index_ - size);
}
template <typename ArrayType>
std::size_t DynArray<ArrayType>::Iterator::operator-(
    const Iterator& other) const {
  return static_cast<std::size_t>(index_) -
         static_cast<std::size_t>(other.index_);
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator&
DynArray<ArrayType>::Iterator::operator--() {
  --index_;
  if (index_ < 0) {
  }
  return *this;
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator
DynArray<ArrayType>::Iterator::operator--(int) {
  Iterator temp(*this);
  --(*this);
  return temp;
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator&
DynArray<ArrayType>::Iterator::operator+=(std::size_t size) {
  index_ += size;
  if (index_ >= array_->size_) {
  }
  return *this;
}
template <typename ArrayType>
typename DynArray<ArrayType>::Iterator&
DynArray<ArrayType>::Iterator::operator-=(std::size_t size) {
  index_ -= size;
  if (index_ < 0) {
  }
  return *this;
}
template <typename ArrayType>
ArrayType& DynArray<ArrayType>::Iterator::operator*() const {
  if (index_ >= array_->size_) {
  }
  return (*array_)[index_];
}
template <typename ArrayType>
ArrayType* DynArray<ArrayType>::Iterator::operator->() const {
  return &(operator*());
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator==(const Iterator& other) const {
  return array_ == other.array_ && index_ == other.index_;
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator!=(const Iterator& other) const {
  return !(*this == other);
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator<(const Iterator& other) const {
  return index_ < other.index_;
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator<=(const Iterator& other) const {
  return index_ <= other.index_;
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator>(const Iterator& other) const {
  return index_ > other.index_;
}
template <typename ArrayType>
bool DynArray<ArrayType>::Iterator::operator>=(const Iterator& other) const {
  return index_ >= other.index_;
}

template <typename ArrayType>
typename DynArray<ArrayType>::Iterator DynArray<ArrayType>::Begin() {
  return Iterator(this, 0);
}

template <typename ArrayType>
typename DynArray<ArrayType>::Iterator DynArray<ArrayType>::End() {
  return Iterator(this, size_);
}

template <typename ValueType>
class HashTable {
 public:
  HashTable(std::size_t init_capacity = 1024);
  ~HashTable();

  HashTable(const HashTable&) = default;
  HashTable(HashTable&&) noexcept = default;
  HashTable& operator=(const HashTable&) = default;
  HashTable& operator=(HashTable&&) noexcept = default;

  void Insert(std::string key, ValueType value);

  bool Find(std::string key, ValueType& value);

 private:
  std::size_t capacity_ = 0;

  std::size_t size_ = 0;

  DynArray<LinkedList<Pair<std::string, std::string>>>* pair_list_ = nullptr;

  unsigned int Hash(std::string key) const;

  int Resize();
};

template <typename ValueType>
HashTable<ValueType>::HashTable(std::size_t init_capacity) {
  pair_list_ = new DynArray<ValueType>(init_capacity);
  if (!pair_list_) {
    return;
  }
  capacity_ = init_capacity;
}
template <typename ValueType>
HashTable<ValueType>::~HashTable() {
  delete pair_list_;
}

template <typename ValueType>
void HashTable<ValueType>::Insert(std::string key, ValueType value) {
  auto hash = static_cast<std::size_t>(Hash(key));
  ++size_;
  if (size_ / capacity_ > 0.8) {
    Resize();
  }
  LinkedList<Pair<std::string, std::string>> insert_list = pair_list_[hash];
  insert_list.Insert(insert_list.End(), {key, value});
}

template <typename ValueType>
bool HashTable<ValueType>::Find(std::string key, ValueType& value) {
  auto hash = static_cast<std::size_t>(Hash(key));
  LinkedList<Pair<std::string, std::string>> find_list = pair_list_[hash];
  typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
      find_list.Begin();
  while (temp_node != find_list.End()) {
    if (key == *temp_node.first) {
      value = *temp_node.second;
      return true;
    }
    ++temp_node;
  }

  return false;
}

template <typename ValueType>
unsigned int HashTable<ValueType>::Hash(std::string key) const {
  unsigned int hash = 5381;
  for (char character : key) {
    hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
  }
  hash = hash % capacity_;
  return hash;
};

template <typename ValueType>
int HashTable<ValueType>::Resize() {
  DynArray<LinkedList<Pair<std::string, std::string>>>* temp = pair_list_;
  std::size_t new_capacity = capacity_ * 1.5;
  pair_list_ =
      new DynArray<LinkedList<Pair<std::string, std::string>>>[new_capacity];
  if (!pair_list_) {
    return -1;
  }
  for (std::size_t i = 0; i < capacity_; ++i) {
    LinkedList<Pair<std::string, ValueType>>& origin_list = temp[i];
    typename LinkedList<Pair<std::string, ValueType>>::Iterator temp_node =
        origin_list.Begin();
    while (temp_node != origin_list.End()) {
      auto hash = static_cast<std::size_t>(Hash(*temp_node.first));
      LinkedList<Pair<std::string, ValueType>>& insert_list = pair_list_[hash];
      insert_list.Insert(insert_list.End(), *temp_node);
      ++temp_node;
    }
  }
  delete[] temp;
  capacity_ = new_capacity;
  return 0;
}

class Token {
 public:
  Token();
  ~Token();

  Token(const Token&) = default;
  Token(Token&&) noexcept = default;
  Token& operator=(const Token&) = default;
  Token& operator=(Token&&) noexcept = default;

  enum class Kind;

  void SetKind(Kind kind);
  Kind GetKind() const;
  void SetDataPtr(void* data_ptr);
  void* GetDataPtr() const;

 private:
  Kind kind_;
  void* data_ptr_;
};

enum class Token::Kind {
  UNKNOWN,
  KEYWORD,
  kAuto,
  kAnd,
  kBitand,
  kBitor,
  kBool,
  kBreak,
  kCase,
  kCatch,
  kChar,
  kClass,
  kConst,
  kContinue,
  kDefault,
  kDo,
  kDouble,
  kElse,
  kEnum,
  kExport,
  kExtern,
  kFalse,
  kFloat,
  kFor,
  kFriend,
  kGoto,
  kImport,
  kInline,
  kInt,
  kLong,
  kNamespace,
  kNew,
  kNot,
  kNumber,
  kOperator,
  kOr,
  kPrivate,
  kProtected,
  kPublic,
  kReturn,
  kShort,
  kSigned,
  kSizeof,
  kStatic,
  kString,
  kStruct,
  kSwitch,
  kTemplate,
  kThis,
  kThread,
  kTrue,
  kTry,
  kTypedef,
  kTypeid,
  kTypename,
  kUnion,
  kUnsigned,
  kUsing,
  kVirtual,
  kVoid,
  kWcharT,
  kWhile,
  kXor,
  IDENTIFIER,
  OPERATOR,
  kLSquare,
  kRSquare,
  kLParen,
  kRParen,
  kLBrace,
  kRBrace,
  kPeriod,
  kEllipsis,
  kAmp,
  kAmpAmp,
  kAmpEqual,
  kStar,
  kStarEqual,
  kPlus,
  kPlusPlus,
  kPlusEqual,
  kMinus,
  kArrow,
  kMinusMinus,
  kMinusEqual,
  kTilde,
  kExclaim,
  kExclaimEqual,
  kSlash,
  kSlashEqual,
  kPercent,
  kPercentEqual,
  kLess,
  kLessLess,
  kLessEqual,
  kLessLessEqual,
  kSpaceship,
  kGreater,
  kGreaterGreater,
  kGreaterEqual,
  kGreaterGreaterEqual,
  kCaret,
  kCaretEqual,
  kPipe,
  kPipePipe,
  kPipeEqual,
  kQuestion,
  kColon,
  kSemi,
  kEqual,
  kEqualEqual,
  kComma,
  kHash,
  kHashHash,
  kHashAt,
  kPeriodStar,
  kArrowStar,
  kColonColon,
  kAt,
  kLessLessLess,
  kGreaterGreaterGreater,
  kCaretCaret,
  NUMBER,
  CHARACTER,
  STRING,
  COMMENT
};

Token::Token() = default;
Token::~Token() = default;

void Token::SetKind(Kind kind) { this->kind_ = kind; }

Token::Kind Token::GetKind() const { return kind_; }

void Token::SetDataPtr(void* data_ptr) { this->data_ptr_ = data_ptr; }

void* Token::GetDataPtr() const { return data_ptr_; }

Token::Token() = default;
Token::~Token() = default;

void Token::SetKind(Kind kind) { this->kind_ = kind; }

Token::Kind Token::GetKind() const { return kind_; }

void Token::SetDataPtr(void* data_ptr) { this->data_ptr_ = data_ptr; }

void* Token::GetDataPtr() const { return data_ptr_; }

class TokenMap {
 public:
  TokenMap();
  ~TokenMap();

  Token::Kind GetKind(std::string key);

  TokenMap(const TokenMap&) = default;
  TokenMap(TokenMap&&) noexcept = default;
  TokenMap& operator=(const TokenMap&) = default;
  TokenMap& operator=(TokenMap&&) noexcept = default;

 private:
  HashTable<Token::Kind> token_map_;
};
TokenMap::TokenMap() {
  token_map_.Insert("auto", Token::Kind::kAuto);
  token_map_.Insert("and", Token::Kind::kAnd);
  token_map_.Insert("bitand", Token::Kind::kBitand);
  token_map_.Insert("bitor", Token::Kind::kBitor);
  token_map_.Insert("bool", Token::Kind::kBool);
  token_map_.Insert("break", Token::Kind::kBreak);
  token_map_.Insert("case", Token::Kind::kCase);
  token_map_.Insert("catch", Token::Kind::kCatch);
  token_map_.Insert("char", Token::Kind::kChar);
  token_map_.Insert("class", Token::Kind::kClass);
  token_map_.Insert("const", Token::Kind::kConst);
  token_map_.Insert("continue", Token::Kind::kContinue);
  token_map_.Insert("default", Token::Kind::kDefault);
  token_map_.Insert("do", Token::Kind::kDo);
  token_map_.Insert("double", Token::Kind::kDouble);
  token_map_.Insert("else", Token::Kind::kElse);
  token_map_.Insert("enum", Token::Kind::kEnum);
  token_map_.Insert("export", Token::Kind::kExport);
  token_map_.Insert("extern", Token::Kind::kExtern);
  token_map_.Insert("false", Token::Kind::kFalse);
  token_map_.Insert("float", Token::Kind::kFloat);
  token_map_.Insert("for", Token::Kind::kFor);
  token_map_.Insert("friend", Token::Kind::kFriend);
  token_map_.Insert("goto", Token::Kind::kGoto);
  token_map_.Insert("import", Token::Kind::kImport);
  token_map_.Insert("inline", Token::Kind::kInline);
  token_map_.Insert("int", Token::Kind::kInt);
  token_map_.Insert("long", Token::Kind::kLong);
  token_map_.Insert("namespace", Token::Kind::kNamespace);
  token_map_.Insert("new", Token::Kind::kNew);
  token_map_.Insert("not", Token::Kind::kNot);
  token_map_.Insert("number", Token::Kind::kNumber);
  token_map_.Insert("operator", Token::Kind::kOperator);
  token_map_.Insert("or", Token::Kind::kOr);
  token_map_.Insert("private", Token::Kind::kPrivate);
  token_map_.Insert("protected", Token::Kind::kProtected);
  token_map_.Insert("public", Token::Kind::kPublic);
  token_map_.Insert("return", Token::Kind::kReturn);
  token_map_.Insert("short", Token::Kind::kShort);
  token_map_.Insert("signed", Token::Kind::kSigned);
  token_map_.Insert("sizeof", Token::Kind::kSizeof);
  token_map_.Insert("static", Token::Kind::kStatic);
  token_map_.Insert("string", Token::Kind::kString);
  token_map_.Insert("struct", Token::Kind::kStruct);
  token_map_.Insert("switch", Token::Kind::kSwitch);
  token_map_.Insert("template", Token::Kind::kTemplate);
  token_map_.Insert("this", Token::Kind::kThis);
  token_map_.Insert("thread", Token::Kind::kThread);
  token_map_.Insert("true", Token::Kind::kTrue);
  token_map_.Insert("try", Token::Kind::kTry);
  token_map_.Insert("typedef", Token::Kind::kTypedef);
  token_map_.Insert("typeid", Token::Kind::kTypeid);
  token_map_.Insert("typename", Token::Kind::kTypename);
  token_map_.Insert("union", Token::Kind::kUnion);
  token_map_.Insert("unsigned", Token::Kind::kUnsigned);
  token_map_.Insert("using", Token::Kind::kUsing);
  token_map_.Insert("virtual", Token::Kind::kVirtual);
  token_map_.Insert("void", Token::Kind::kVoid);
  token_map_.Insert("wchar_t", Token::Kind::kWcharT);
  token_map_.Insert("while", Token::Kind::kWhile);
  token_map_.Insert("xor", Token::Kind::kXor);

  token_map_.Insert("[", Token::Kind::kLSquare);
  token_map_.Insert("]", Token::Kind::kRSquare);
  token_map_.Insert("(", Token::Kind::kLParen);
  token_map_.Insert(")", Token::Kind::kRParen);
  token_map_.Insert("{", Token::Kind::kLBrace);
  token_map_.Insert("}", Token::Kind::kRBrace);
  token_map_.Insert(".", Token::Kind::kPeriod);
  token_map_.Insert("...", Token::Kind::kEllipsis);
  token_map_.Insert("&", Token::Kind::kAmp);
  token_map_.Insert("&&", Token::Kind::kAmpAmp);
  token_map_.Insert("&=", Token::Kind::kAmpEqual);
  token_map_.Insert("*", Token::Kind::kStar);
  token_map_.Insert("*=", Token::Kind::kStarEqual);
  token_map_.Insert("+", Token::Kind::kPlus);
  token_map_.Insert("++", Token::Kind::kPlusPlus);
  token_map_.Insert("+=", Token::Kind::kPlusEqual);
  token_map_.Insert("-", Token::Kind::kMinus);
  token_map_.Insert("->", Token::Kind::kArrow);
  token_map_.Insert("--", Token::Kind::kMinusMinus);
  token_map_.Insert("-=", Token::Kind::kMinusEqual);
  token_map_.Insert("~", Token::Kind::kTilde);
  token_map_.Insert("!", Token::Kind::kExclaim);
  token_map_.Insert("!=", Token::Kind::kExclaimEqual);
  token_map_.Insert("/", Token::Kind::kSlash);
  token_map_.Insert("/=", Token::Kind::kSlashEqual);
  token_map_.Insert("%", Token::Kind::kPercent);
  token_map_.Insert("%=", Token::Kind::kPercentEqual);
  token_map_.Insert("<", Token::Kind::kLess);
  token_map_.Insert("<<", Token::Kind::kLessLess);
  token_map_.Insert("<=", Token::Kind::kLessEqual);
  token_map_.Insert("<<=", Token::Kind::kLessLessEqual);
  token_map_.Insert("<=>", Token::Kind::kSpaceship);
  token_map_.Insert(">", Token::Kind::kGreater);
  token_map_.Insert(">>", Token::Kind::kGreaterGreater);
  token_map_.Insert(">=", Token::Kind::kGreaterEqual);
  token_map_.Insert(">>=", Token::Kind::kGreaterGreaterEqual);
  token_map_.Insert("^", Token::Kind::kCaret);
  token_map_.Insert("^=", Token::Kind::kCaretEqual);
  token_map_.Insert("|", Token::Kind::kPipe);
  token_map_.Insert("||", Token::Kind::kPipePipe);
  token_map_.Insert("|=", Token::Kind::kPipeEqual);
  token_map_.Insert("?", Token::Kind::kQuestion);
  token_map_.Insert(":", Token::Kind::kColon);
  token_map_.Insert(";", Token::Kind::kSemi);
  token_map_.Insert("=", Token::Kind::kEqual);
  token_map_.Insert("==", Token::Kind::kEqualEqual);
  token_map_.Insert(",", Token::Kind::kComma);
  token_map_.Insert("#", Token::Kind::kHash);
  token_map_.Insert("##", Token::Kind::kHashHash);
  token_map_.Insert("#@", Token::Kind::kHashAt);
  token_map_.Insert(".*", Token::Kind::kPeriodStar);
  token_map_.Insert("->*", Token::Kind::kArrowStar);
  token_map_.Insert("::", Token::Kind::kColonColon);
  token_map_.Insert("@", Token::Kind::kAt);
  token_map_.Insert("<<<", Token::Kind::kLessLessLess);
  token_map_.Insert(">>>", Token::Kind::kGreaterGreaterGreater);
  token_map_.Insert("^^", Token::Kind::kCaretCaret);
}
TokenMap::~TokenMap() = default;

Token::Kind TokenMap::GetKind(std::string key) {
  /// TODO: Should be improved
  /// return token_map_.Find(_operator);
}

class Lexer {
 public:
  Lexer(char* source_code, size_t length);
  ~Lexer();

  Lexer(const Lexer&) = delete;
  Lexer(Lexer&&) noexcept = delete;
  Lexer& operator=(const Lexer&) = delete;
  Lexer& operator=(Lexer&&) noexcept = delete;

  int Lex(Token& return_token);

  bool IsReadEnd() const;

 private:
  char* buffer_ptr_;

  char* buffer_end_;

  TokenMap token_map_;

  bool ProcessToken(Token& token, Token::Kind start_kind, int next_kind_size,
                    ...) const;
};

Lexer::Lexer(char* source_code, size_t length)
    : buffer_ptr_(source_code), buffer_end_(source_code + length - 1) {};
Lexer::~Lexer() = default;

int Lexer::Lex(Token& return_token) {
  using Tok = Token::Kind;
  return_token.SetKind(Tok::UNKNOWN);
  char* read_ptr = buffer_ptr_;

LexStart:
  if (read_ptr > buffer_end_) {
    buffer_ptr_ = read_ptr;
    return -1;
  }

  switch (*read_ptr) {
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '(':
    case ')':
    case '*':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '[':
    case ']':
    case '^':
    case '`':
    case '{':
    case '|':
    case '}':
    case '~':
      if (ProcessToken(return_token, Tok::OPERATOR, 4, Tok::OPERATOR,
                       Tok::CHARACTER, Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    // The string flag.
    case '"':
      if (ProcessToken(return_token, Tok::STRING, 2, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      // End of string.
      if (return_token.GetKind() == Tok::STRING) {
        ++read_ptr;
      }

      goto LexEnd;

    // The character flag.
    case '\'':
      if (ProcessToken(return_token, Tok::CHARACTER, 2, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      // End of character.
      if (return_token.GetKind() == Tok::CHARACTER) {
        ++read_ptr;
      }

      goto LexEnd;

    // Escape character.
    case '\\':
      if (ProcessToken(return_token, Tok::OPERATOR, 2, Tok::OPERATOR,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      // Skip escape characters.
      if (return_token.GetKind() == Tok::CHARACTER ||
          return_token.GetKind() == Tok::STRING) {
        if (read_ptr + 2 <= buffer_end_) {
          ++read_ptr;
        }
        goto LexNext;
      }

      goto LexEnd;

    // Positive and negative numbers.
    case '+':
    case '-':
      // Signed numbers.
      if (return_token.GetKind() == Tok::UNKNOWN && *(read_ptr + 1) == '0' ||
          *(read_ptr + 1) == '1' || *(read_ptr + 1) == '2' ||
          *(read_ptr + 1) == '3' || *(read_ptr + 1) == '4' ||
          *(read_ptr + 1) == '5' || *(read_ptr + 1) == '6' ||
          *(read_ptr + 1) == '7' || *(read_ptr + 1) == '8' ||
          *(read_ptr + 1) == '9') {
        return_token.SetKind(Tok::NUMBER);
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 4, Tok::OPERATOR,
                       Tok::CHARACTER, Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      // Dealing with scientific notation.
      if (return_token.GetKind() == Tok::NUMBER &&
          (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e')) {
        goto LexNext;
      }

      goto LexEnd;

    // Decimal point.
    case '.':
      if (ProcessToken(return_token, Tok::OPERATOR, 5, Tok::OPERATOR,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    // The comment flag.
    case '/':
      // Comment start.
      if (return_token.GetKind() == Tok::UNKNOWN && *(buffer_ptr_ + 1) == '/' ||
          *(buffer_ptr_ + 1) == '*') {
        return_token.SetKind(Tok::COMMENT);
        if (read_ptr + 2 <= buffer_end_) {
          ++read_ptr;
        }
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 2, Tok::CHARACTER,
                       Tok::STRING)) {
        goto LexNext;
      }

      if (return_token.GetKind() == Tok::OPERATOR) {
        // Comment.
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          goto LexNext;
        }
      }

      if (return_token.GetKind() == Tok::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*' && *(read_ptr - 1) == '*') {
          // /**/ style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          return_token.SetKind(Tok::UNKNOWN);
          goto LexStart;
        } else {
          // // style comments or Non-end comment mark, continue reading until
          // the end mark of the comment.
          goto LexNext;
        }
      }

      goto LexEnd;

    // Numbers.
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if (ProcessToken(return_token, Tok::NUMBER, 5, Tok::IDENTIFIER,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    // Whitespace characters.
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      if (return_token.GetKind() == Tok::UNKNOWN) {
        // Skip whitespace characters.
        ++buffer_ptr_;
        goto LexNext;
      }

      if (ProcessToken(return_token, Tok::UNKNOWN, 3, Tok::CHARACTER,
                       Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    // Newlines.
    case '\n':
      if (return_token.GetKind() == Tok::UNKNOWN) {
        // Skip newlines.
        ++buffer_ptr_;
        goto LexNext;
      }

      if (return_token.GetKind() == Tok::COMMENT && *(buffer_ptr_ + 1) == '/') {
        // // style comments, skip all comments.
        buffer_ptr_ = ++read_ptr;
        return_token.SetKind(Tok::UNKNOWN);
        goto LexStart;
      }

      if (ProcessToken(return_token, Tok::UNKNOWN, 3, Tok::CHARACTER,
                       Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    // EOF.
    case '\0':
      goto LexEnd;

    // Separator flag.
    case ',':
    case ';':
      if (return_token.GetKind() == Tok::UNKNOWN) {
        return_token.SetKind(Tok::OPERATOR);
        ++read_ptr;
        goto LexEnd;
      }

      if (ProcessToken(return_token, Tok::OPERATOR, 3, Tok::CHARACTER,
                       Tok::STRING, Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;

    default:
      if (ProcessToken(return_token, Tok::IDENTIFIER, 5, Tok::IDENTIFIER,
                       Tok::NUMBER, Tok::CHARACTER, Tok::STRING,
                       Tok::COMMENT)) {
        goto LexNext;
      }

      goto LexEnd;
  }

LexNext:
  ++read_ptr;
  goto LexStart;

LexEnd:
    // Meaningless token.
    if (return_token.GetKind() == Tok::UNKNOWN ||
        return_token.GetKind() == Tok::COMMENT) {
      return_token.SetKind(Tok::UNKNOWN);
      buffer_ptr_ = read_ptr;
      return 0;
    } else {
      // Meaningful token. Determine the specific token information.
      char* location = buffer_ptr_;
      size_t length = read_ptr - buffer_ptr_;
      buffer_ptr_ = read_ptr;

      // Handle the detailed information of tokens.
      Token::ValueStr value;
      value.location = location;
      value.length = length;
      switch (return_token.GetKind()) {
        case Tok::IDENTIFIER:
          return_token.value.keyword =
              token_map_.GetKeywordValue(std::string(location, length));
          if (return_token.value.keyword == Token::Kind::UNKNOWN) {
            return_token.value.identifier = value;
            break;
          }
          return_token.SetKind(Tok::KEYWORD);
          break;

        case Tok::CHARACTER:
          return_token.value.character = value;
          break;

        case Tok::STRING:
          return_token.value.string = value;
          break;

        case Tok::OPERATOR:
          return_token.value._operator =
              token_map_.GetOperatorValue(std::string(location, length));
          while (return_token.GetKind() == Token::Kind::UNKNOWN && length > 1) {
            length--;
            buffer_ptr_--;
            return_token.value._operator =
                token_map_.GetOperatorValue(std::string(location, length));
          }
          break;

        case Tok::NUMBER:
          return_token.value.number = value;
          break;

        default:
          return -2;
      }
    }
  return 0;
}

bool Lexer::IsReadEnd() const {
  if (buffer_ptr_ >= buffer_end_) {
    return true;
  }
  return false;
}

bool Lexer::ProcessToken(Token& token, Token::Kind start_kind,
                         int next_kind_size, ...) const {
  if (token.GetKind() == Token::Kind::UNKNOWN) {
    token.SetKind(start_kind);
    return true;
  }
  std::va_list next_kind_list;
  va_start(next_kind_list, next_kind_size);
  for (int i = 0; i < next_kind_size; ++i) {
    if (token.GetKind() == va_arg(next_kind_list, Token::Kind)) {
      return true;
    }
  }
  return false;
}
}  // namespace Aq

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  char* filename = argv[1];
  std::ifstream file;
  file.open(filename);
  if (!file.is_open()) {
    printf("Error: Could not open file %s\n", argv[1]);
    return -2;
  }

  std::vector<char> code;
  char ch;
  while (file.get(ch)) {
    code.push_back(ch);
  }
  code.push_back('\0');
  file.close();
  char* buffer_ptr_ = code.data();
  Aq::Lexer lexer(buffer_ptr_, code.size());
  Aq::Token token;
  while (true) {
    lexer.Lex(token);
    if (lexer.IsReadEnd()) {
      break;
    }
  }
}