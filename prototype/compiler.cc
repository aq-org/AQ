// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define _AQVM_OPERATOR_NOP 0x00
#define _AQVM_OPERATOR_LOAD 0x01
#define _AQVM_OPERATOR_STORE 0x02
#define _AQVM_OPERATOR_NEW 0x03
#define _AQVM_OPERATOR_FREE 0x04
#define _AQVM_OPERATOR_PTR 0x05
#define _AQVM_OPERATOR_ADD 0x06
#define _AQVM_OPERATOR_SUB 0x07
#define _AQVM_OPERATOR_MUL 0x08
#define _AQVM_OPERATOR_DIV 0x09
#define _AQVM_OPERATOR_REM 0x0A
#define _AQVM_OPERATOR_NEG 0x0B
#define _AQVM_OPERATOR_SHL 0x0C
#define _AQVM_OPERATOR_SHR 0x0D
#define _AQVM_OPERATOR_SAR 0x0E
#define _AQVM_OPERATOR_IF 0x0F
#define _AQVM_OPERATOR_AND 0x10
#define _AQVM_OPERATOR_OR 0x11
#define _AQVM_OPERATOR_XOR 0x12
#define _AQVM_OPERATOR_CMP 0x13
#define _AQVM_OPERATOR_INVOKE 0x14
#define _AQVM_OPERATOR_EQUAL 0x15
#define _AQVM_OPERATOR_GOTO 0x16
#define _AQVM_OPERATOR_THROW 0x17
#define _AQVM_OPERATOR_WIDE 0xFF

inline void EXIT_COMPILER(const char* func_name, const char* message) {
  std::cerr << "[ERROR] " << func_name << ": " << message << std::endl;
  exit(EXIT_FAILURE);
}

#define TRACE_FUNCTION Trace trace(__FUNCTION__)

std::stack<std::string> call_stack;

class Trace {
 public:
  explicit Trace(const std::string& function_name) {
    call_stack.push(function_name);
    printStack();
  }

  ~Trace() {
    call_stack.pop();
    printStack();
  }

 private:
  void printStack() const {
    std::stack<std::string> temp_stack = call_stack;
    std::vector<std::string> reverse_stack;

    while (!temp_stack.empty()) {
      reverse_stack.push_back(temp_stack.top());
      temp_stack.pop();
    }

    std::cerr << "[INFO] Run: ";
    for (auto it = reverse_stack.rbegin(); it != reverse_stack.rend(); ++it) {
      std::cerr << *it << " -> ";
    }
    std::cerr << "Success" << std::endl;
  }
};

namespace Aq {
namespace Compiler {
// A hash table for the lexer. Used to find special definitions such as compiler
// keywords.
template <typename T>
class LexMap {
 public:
  // Construct a LexMap class, and the default hash table memory size is 1024.
  // Do not modify it unless necessary.
  LexMap() {
    TRACE_FUNCTION;
    std::size_t init_capacity = 1024;
    pair_list_ = new PairList[init_capacity];
    if (pair_list_ == nullptr) {
      EXIT_COMPILER("LexMap::LexMap()", "Memory allocation failed.");
    }
    capacity_ = init_capacity;
  };
  ~LexMap() { delete[] pair_list_; };

  LexMap(const LexMap&) = default;
  LexMap(LexMap&&) noexcept = default;
  LexMap& operator=(const LexMap&) = default;
  LexMap& operator=(LexMap&&) noexcept = default;

  // Insert a new pair into the hash table.
  void Insert(std::string key, T value) {
    unsigned int hash = Hash(key);

    // Increase the size of the hash table.
    size_++;
    if (static_cast<double>(size_) / static_cast<double>(capacity_) > 0.8) {
      Resize();
    }

    // Create key-value pairs and insert them into the linked list.
    Pair pair;
    pair.key = key;
    pair.value = value;
    pair_list_[hash].Prepend(pair);
  };

  void Delete(std::string key) {
    unsigned int hash = Hash(key);
    pair_list_[hash].Delete(key);
  };

  // Find the value of a key.
  T Find(std::string key) {
    unsigned int hash = Hash(key);
    return pair_list_[hash].Find(key);
  };

  bool IsElementExist(std::string key) {
    unsigned int hash = Hash(key);
    return pair_list_[hash].IsElementExist(key);
  }

 private:
  struct Pair {
    std::string key = std::string();
    T value = T();
  };

  // A linked list of Pair type, used to resolve hash conflicts.
  class PairList {
   public:
    // Construct a PairList class.
    PairList() = default;
    ~PairList() {
      while (head_ptr_ != nullptr) {
        Node* temp = head_ptr_;
        head_ptr_ = head_ptr_->next;
        delete temp;
      }
    };

    PairList(const PairList&) = default;
    PairList(PairList&&) noexcept = default;
    PairList& operator=(const PairList&) = default;
    PairList& operator=(PairList&&) noexcept = default;

    // Prepend a new pair to the list.
    void Prepend(Pair value) {
      Node* new_node = new Node(value);
      new_node->next = head_ptr_;
      head_ptr_ = new_node;
    };

    // Copy all the data in the linked list to |new_list|.
    void CopyDataToNewList(PairList* new_list, std::size_t new_capacity) {
      PairList::Node* temp_node = head_ptr_;
      while (temp_node != nullptr) {
        unsigned int hash = 5381;
        for (char character : temp_node->data.key) {
          // hash = hash * 33 + static_cast<unsigned int>(character)
          hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
        }
        hash = hash % new_capacity;
        new_list[hash].Append(temp_node->data);
        temp_node = temp_node->next;
      }
    };

    // Append a new pair to the list. It is not recommended to use it when
    // dealing with large amounts of data.
    void Append(Pair value) {
      if (head_ptr_ == nullptr) {
        head_ptr_ = new Node(value);
      } else {
        // Find the last node and append the new node.
        Node* temp = head_ptr_;
        while (temp->next != nullptr) {
          temp = temp->next;
        }
        temp->next = new Node(value);
      }
    };

    void Delete(std::string key) {
      Node* temp = head_ptr_;
      Node* prev = nullptr;

      // Find the key to delete.
      while (temp != nullptr) {
        if (key == temp->data.key) {
          break;
        }
        prev = temp;
        temp = temp->next;
      }

      // Key not found.
      if (temp == nullptr) {
        return;
      }

      // Delete the key.
      if (prev == nullptr) {
        head_ptr_ = temp->next;
      } else {
        prev->next = temp->next;
      }
      delete temp;
    };

    // Find the value of a key.
    T Find(std::string key) {
      Node* temp = head_ptr_;

      // Compare keys one by one to find the corresponding value.
      while (temp != nullptr) {
        if (key == temp->data.key) {
          return temp->data.value;
        };
        temp = temp->next;
      }

      // Key not found, return nullptr.
      return T();
    };

    bool IsElementExist(std::string key) {
      Node* temp = head_ptr_;

      // Compare keys one by one to find the corresponding value.
      while (temp != nullptr) {
        if (key == temp->data.key) {
          return true;
        };
        temp = temp->next;
      }
      return false;
    }

   private:
    // A node type of the linked list.
    struct Node {
      Pair data;
      Node* next = nullptr;
      Node(Pair pair) : data(pair){};
    };

    // The head pointer of the linked list.
    Node* head_ptr_ = nullptr;
  };

  // The memory size of the hash table.
  std::size_t capacity_ = 1;

  // The number of elements in the hash table.
  std::size_t size_ = 0;

  // The data collection of the hash table is stored in a linked list of type
  // PairList.
  PairList* pair_list_ = nullptr;

  // The hash function. Based on DJB2 hashing algorithm.
  unsigned int Hash(std::string key) const {
    unsigned int hash = 5381;
    for (char character : key) {
      // hash = hash * 33 + static_cast<unsigned int>(character)
      hash = ((hash << 5) + hash) + static_cast<unsigned int>(character);
    }
    hash = hash % capacity_;
    return hash;
  };

  // Re-allocate the memory of the hash table.
  int Resize() {
    PairList* temp = pair_list_;
    std::size_t new_capacity = capacity_ * 1.5;
    pair_list_ = new PairList[new_capacity];

    // Memory allocation failed.
    if (!pair_list_) {
      return -1;
    }

    // Copy data.
    for (int i = 0; i < capacity_; i++) {
      temp[i].CopyDataToNewList(pair_list_, new_capacity);
    }

    // Release the memory of the original linked list.
    delete[] temp;

    capacity_ = new_capacity;
    return 0;
  };
};

struct Token {
  enum class Type {
    NONE,
    START,
    KEYWORD,
    IDENTIFIER,
    OPERATOR,
    NUMBER,
    CHARACTER,
    STRING,
    COMMENT
  };
  enum class KeywordType {
    NONE = 0,
    Auto,
    And,
    Bitand,
    Bitor,
    Bool,
    Break,
    Case,
    Catch,
    Char,
    Class,
    Const,
    Continue,
    Default,
    Do,
    Double,
    Else,
    Enum,
    Export,
    Extern,
    False,
    Float,
    For,
    Friend,
    Goto,
    If,
    Import,
    Inline,
    Int,
    Long,
    Namespace,
    New,
    Not,
    Number,
    Operator,
    Or,
    Private,
    Protected,
    Public,
    Return,
    Short,
    Signed,
    Sizeof,
    Static,
    String,
    Struct,
    Switch,
    Template,
    This,
    Thread,
    True,
    Try,
    Typedef,
    Typeid,
    Typename,
    Union,
    Unsigned,
    Using,
    Virtual,
    Void,
    Wchar_t,
    While,
    Xor,
  };
  enum class OperatorType {
    // TODO: Add more operators.
    NONE = 0,
    l_square,               //[
    r_square,               //]
    l_paren,                //(
    r_paren,                //)
    l_brace,                //{
    r_brace,                //}
    period,                 //.
    ellipsis,               //...
    amp,                    //&
    ampamp,                 //&&
    ampequal,               //&=
    star,                   //*
    starequal,              //*=
    plus,                   //+
    plusplus,               //++
    plusequal,              //+=
    minus,                  //-
    arrow,                  //->
    minusminus,             //--
    minusequal,             //-=
    tilde,                  //~
    exclaim,                //!
    exclaimequal,           //!=
    slash,                  ///
    slashequal,             ///=
    percent,                //%
    percentequal,           //%=
    less,                   //<
    lessless,               //<<
    lessequal,              //<=
    lesslessequal,          //<<=
    spaceship,              //<=>
    greater,                //>
    greatergreater,         //>>
    greaterequal,           //>=
    greatergreaterequal,    //>>=
    caret,                  //^
    caretequal,             //^=
    pipe,                   //|
    pipepipe,               //||
    pipeequal,              //|=
    question,               //?
    colon,                  //:
    semi,                   //;
    equal,                  //=
    equalequal,             //==
    comma,                  //,
    hash,                   // #
    hashhash,               // ##
    hashat,                 // #@
    periodstar,             //.*
    arrowstar,              //->*
    coloncolon,             //::
    at,                     //@
    lesslessless,           //<<<
    greatergreatergreater,  //>>>
    caretcaret,             //^^
  };
  struct ValueStr {
    char* location;
    std::size_t length;
  };
  union Value {
    ValueStr number;
    KeywordType keyword;
    ValueStr identifier;
    OperatorType _operator;
    char character;
    std::string* string;
  };

  Type type = Type::START;
  Value value = Value();

  Token() = default;
  ~Token() = default;

  Token(const Token& other) = default;
  Token(Token&& other) noexcept = default;

  Token& operator=(const Token& other) = default;
  Token& operator=(Token&& other) noexcept = default;

  /*Token(const Token& other) {
    type = other.type;
    switch (type) {
      case Type::NUMBER:
        value.number = other.value.number;
        break;
      case Type::KEYWORD:
        value.keyword = other.value.keyword;
        break;
      case Type::IDENTIFIER:
        value.identifier = other.value.identifier;
        break;
      case Type::OPERATOR:
        value._operator = other.value._operator;
        break;
      case Type::CHARACTER:
        value.character = other.value.character;
        break;
      case Type::STRING:
        value.string = other.value.string;
        break;
      default:
        break;
    }
  }

  Token(Token&& other) noexcept {
    type = other.type;
    switch (type) {
      case Type::NUMBER:
        value.number = other.value.number;
        break;
      case Type::KEYWORD:
        value.keyword = other.value.keyword;
        break;
      case Type::IDENTIFIER:
        value.identifier = other.value.identifier;
        break;
      case Type::OPERATOR:
        value._operator = other.value._operator;
        break;
      case Type::CHARACTER:
        value.character = other.value.character;
        break;
      case Type::STRING:
        value.string = other.value.string;
        break;
      default:
        break;
    }
    other.type = Type::NONE;
  }

  Token& operator=(const Token& other) {
    type = other.type;
    switch (type) {
      case Type::NUMBER:
        value.number = other.value.number;
        break;
      case Type::KEYWORD:
        value.keyword = other.value.keyword;
        break;
      case Type::IDENTIFIER:
        value.identifier = other.value.identifier;
        break;
      case Type::OPERATOR:
        value._operator = other.value._operator;
        break;
      case Type::CHARACTER:
        value.character = other.value.character;
        break;
      case Type::STRING:
        value.string = other.value.string;
        break;
      default:
        break;
    }
    return *this;
  }

  Token& operator=(Token&& other) noexcept {
    type = other.type;
    switch (type) {
      case Type::NUMBER:
        value.number = other.value.number;
        break;
      case Type::KEYWORD:
        value.keyword = other.value.keyword;
        break;
      case Type::IDENTIFIER:
        value.identifier = other.value.identifier;
        break;
      case Type::OPERATOR:
        value._operator = other.value._operator;
        break;
      case Type::CHARACTER:
        value.character = other.value.character;
        break;
      case Type::STRING:
        value.string = other.value.string;
        break;
      default:
        break;
    }
    other.type = Type::NONE;
    return *this;
  }*/

  std::string TokenTypeToString(Token::Type type) {
    switch (type) {
      case Token::Type::NONE:
        return "NONE";
      case Token::Type::START:
        return "START";
      case Token::Type::KEYWORD:
        return "KEYWORD";
      case Token::Type::IDENTIFIER:
        return "IDENTIFIER";
      case Token::Type::OPERATOR:
        return "OPERATOR";
      case Token::Type::NUMBER:
        return "NUMBER";
      case Token::Type::CHARACTER:
        return "CHARACTER";
      case Token::Type::STRING:
        return "STRING";
      case Token::Type::COMMENT:
        return "COMMENT";
      default:
        return "UNKNOWN";
    }
  }

  std::string KeywordTypeToString(Token::KeywordType keyword) {
    switch (keyword) {
      case Token::KeywordType::Auto:
        return "Auto";
      case Token::KeywordType::And:
        return "And";
      case Token::KeywordType::Bitand:
        return "Bitand";
      case Token::KeywordType::Bitor:
        return "Bitor";
      case Token::KeywordType::Bool:
        return "Bool";
      case Token::KeywordType::Break:
        return "Break";
      case Token::KeywordType::Case:
        return "Case";
      case Token::KeywordType::Catch:
        return "Catch";
      case Token::KeywordType::Char:
        return "Char";
      case Token::KeywordType::Class:
        return "Class";
      case Token::KeywordType::Const:
        return "Const";
      case Token::KeywordType::Continue:
        return "Continue";
      case Token::KeywordType::Default:
        return "Default";
      case Token::KeywordType::Do:
        return "Do";
      case Token::KeywordType::Double:
        return "Double";
      case Token::KeywordType::Else:
        return "Else";
      case Token::KeywordType::Enum:
        return "Enum";
      case Token::KeywordType::Export:
        return "Export";
      case Token::KeywordType::Extern:
        return "Extern";
      case Token::KeywordType::False:
        return "False";
      case Token::KeywordType::Float:
        return "Float";
      case Token::KeywordType::For:
        return "For";
      case Token::KeywordType::Friend:
        return "Friend";
      case Token::KeywordType::Goto:
        return "Goto";
      case Token::KeywordType::If:
        return "If";
      case Token::KeywordType::Import:
        return "Import";
      case Token::KeywordType::Inline:
        return "Inline";
      case Token::KeywordType::Int:
        return "Int";
      case Token::KeywordType::Long:
        return "Long";
      case Token::KeywordType::Namespace:
        return "Namespace";
      case Token::KeywordType::New:
        return "New";
      case Token::KeywordType::Not:
        return "Not";
      case Token::KeywordType::Number:
        return "Number";
      case Token::KeywordType::Operator:
        return "Operator";
      case Token::KeywordType::Or:
        return "Or";
      case Token::KeywordType::Private:
        return "Private";
      case Token::KeywordType::Protected:
        return "Protected";
      case Token::KeywordType::Public:
        return "Public";
      case Token::KeywordType::Return:
        return "Return";
      case Token::KeywordType::Short:
        return "Short";
      case Token::KeywordType::Signed:
        return "Signed";
      case Token::KeywordType::Sizeof:
        return "Sizeof";
      case Token::KeywordType::Static:
        return "Static";
      case Token::KeywordType::String:
        return "String";
      case Token::KeywordType::Struct:
        return "Struct";
      case Token::KeywordType::Switch:
        return "Switch";
      case Token::KeywordType::Template:
        return "Template";
      case Token::KeywordType::This:
        return "This";
      case Token::KeywordType::Thread:
        return "Thread";
      case Token::KeywordType::True:
        return "True";
      case Token::KeywordType::Try:
        return "Try";
      case Token::KeywordType::Typedef:
        return "Typedef";
      case Token::KeywordType::Typeid:
        return "Typeid";
      case Token::KeywordType::Typename:
        return "Typename";
      case Token::KeywordType::Union:
        return "Union";
      case Token::KeywordType::Unsigned:
        return "Unsigned";
      case Token::KeywordType::Using:
        return "Using";
      case Token::KeywordType::Virtual:
        return "Virtual";
      case Token::KeywordType::Void:
        return "Void";
      case Token::KeywordType::Wchar_t:
        return "Wchar_t";
      case Token::KeywordType::While:
        return "While";
      case Token::KeywordType::Xor:
        return "Xor";
      default:
        return "UNKNOWN_KEYWORD";
    }
  }

  std::string OperatorTypeToString(Token::OperatorType op) {
    switch (op) {
      case Token::OperatorType::NONE:
        return "NONE";
      case Token::OperatorType::l_square:
        return "[";
      case Token::OperatorType::r_square:
        return "]";
      case Token::OperatorType::l_paren:
        return "(";
      case Token::OperatorType::r_paren:
        return ")";
      case Token::OperatorType::l_brace:
        return "{";
      case Token::OperatorType::r_brace:
        return "}";
      case Token::OperatorType::period:
        return ".";
      case Token::OperatorType::ellipsis:
        return "...";
      case Token::OperatorType::amp:
        return "&";
      case Token::OperatorType::ampamp:
        return "&&";
      case Token::OperatorType::ampequal:
        return "&=";
      case Token::OperatorType::star:
        return "*";
      case Token::OperatorType::starequal:
        return "*=";
      case Token::OperatorType::plus:
        return "+";
      case Token::OperatorType::plusplus:
        return "++";
      case Token::OperatorType::plusequal:
        return "+=";
      case Token::OperatorType::minus:
        return "-";
      case Token::OperatorType::arrow:
        return "->";
      case Token::OperatorType::minusminus:
        return "--";
      case Token::OperatorType::minusequal:
        return "-=";
      case Token::OperatorType::tilde:
        return "~";
      case Token::OperatorType::exclaim:
        return "!";
      case Token::OperatorType::exclaimequal:
        return "!=";
      case Token::OperatorType::slash:
        return "/";
      case Token::OperatorType::slashequal:
        return "/=";
      case Token::OperatorType::percent:
        return "%";
      case Token::OperatorType::percentequal:
        return "%=";
      case Token::OperatorType::less:
        return "<";
      case Token::OperatorType::lessless:
        return "<<";
      case Token::OperatorType::lessequal:
        return "<=";
      case Token::OperatorType::lesslessequal:
        return "<<=";
      case Token::OperatorType::spaceship:
        return "<=>";
      case Token::OperatorType::greater:
        return ">";
      case Token::OperatorType::greatergreater:
        return ">>";
      case Token::OperatorType::greaterequal:
        return ">=";
      case Token::OperatorType::greatergreaterequal:
        return ">>=";
      case Token::OperatorType::caret:
        return "^";
      case Token::OperatorType::caretequal:
        return "^=";
      case Token::OperatorType::pipe:
        return "|";
      case Token::OperatorType::pipepipe:
        return "||";
      case Token::OperatorType::pipeequal:
        return "|=";
      case Token::OperatorType::question:
        return "?";
      case Token::OperatorType::colon:
        return ":";
      case Token::OperatorType::semi:
        return ";";
      case Token::OperatorType::equal:
        return "=";
      case Token::OperatorType::equalequal:
        return "==";
      case Token::OperatorType::comma:
        return ",";
      case Token::OperatorType::hash:
        return "#";
      case Token::OperatorType::hashhash:
        return "##";
      case Token::OperatorType::hashat:
        return "#@";
      case Token::OperatorType::periodstar:
        return ".*";
      case Token::OperatorType::arrowstar:
        return "->*";
      case Token::OperatorType::coloncolon:
        return "::";
      case Token::OperatorType::at:
        return "@";
      case Token::OperatorType::lesslessless:
        return "<<<";
      case Token::OperatorType::greatergreatergreater:
        return ">>>";
      case Token::OperatorType::caretcaret:
        return "^^";
      default:
        return "UNKNOWN_OPERATOR";
    }
  }

  friend std::ostream& operator<<(std::ostream& os, Token& token) {
    os << "Type: " << token.TokenTypeToString(token.type) << ", Value: ";
    switch (token.type) {
      case Token::Type::KEYWORD:
        os << token.KeywordTypeToString(token.value.keyword);
        break;
      case Token::Type::IDENTIFIER:
        os << std::string(token.value.identifier.location,
                          token.value.identifier.length);
        break;
      case Token::Type::OPERATOR:
        os << token.OperatorTypeToString(token.value._operator);
        break;
      case Token::Type::NUMBER:
        os << std::string(token.value.number.location,
                          token.value.number.length);
        break;
      case Token::Type::CHARACTER:
        os << token.value.character;
        break;
      case Token::Type::STRING:
        os << token.value.string;
        break;
      default:
        os << "N/A";
        break;
    }
    return os;
  }
};

class TokenMap {
 public:
  TokenMap();
  ~TokenMap();

  Token::KeywordType GetKeywordValue(std::string keyword);
  Token::OperatorType GetOperatorValue(std::string _operator);

  TokenMap(const TokenMap&) = default;
  TokenMap(TokenMap&&) noexcept = default;
  TokenMap& operator=(const TokenMap&) = default;
  TokenMap& operator=(TokenMap&&) noexcept = default;

 private:
  LexMap<Token::KeywordType> keyword_map;
  LexMap<Token::OperatorType> operator_map;
};

TokenMap::TokenMap() {
  keyword_map.Insert("auto", Token::KeywordType::Auto);
  keyword_map.Insert("and", Token::KeywordType::And);
  keyword_map.Insert("bitand", Token::KeywordType::Bitand);
  keyword_map.Insert("bitor", Token::KeywordType::Bitor);
  keyword_map.Insert("bool", Token::KeywordType::Bool);
  keyword_map.Insert("break", Token::KeywordType::Break);
  keyword_map.Insert("case", Token::KeywordType::Case);
  keyword_map.Insert("catch", Token::KeywordType::Catch);
  keyword_map.Insert("char", Token::KeywordType::Char);
  keyword_map.Insert("class", Token::KeywordType::Class);
  keyword_map.Insert("const", Token::KeywordType::Const);
  keyword_map.Insert("continue", Token::KeywordType::Continue);
  keyword_map.Insert("default", Token::KeywordType::Default);
  keyword_map.Insert("do", Token::KeywordType::Do);
  keyword_map.Insert("double", Token::KeywordType::Double);
  keyword_map.Insert("else", Token::KeywordType::Else);
  keyword_map.Insert("enum", Token::KeywordType::Enum);
  keyword_map.Insert("export", Token::KeywordType::Export);
  keyword_map.Insert("extern", Token::KeywordType::Extern);
  keyword_map.Insert("false", Token::KeywordType::False);
  keyword_map.Insert("float", Token::KeywordType::Float);
  keyword_map.Insert("for", Token::KeywordType::For);
  keyword_map.Insert("friend", Token::KeywordType::Friend);
  keyword_map.Insert("goto", Token::KeywordType::Goto);
  keyword_map.Insert("if", Token::KeywordType::If);
  keyword_map.Insert("import", Token::KeywordType::Import);
  keyword_map.Insert("inline", Token::KeywordType::Inline);
  keyword_map.Insert("int", Token::KeywordType::Int);
  keyword_map.Insert("long", Token::KeywordType::Long);
  keyword_map.Insert("namespace", Token::KeywordType::Namespace);
  keyword_map.Insert("new", Token::KeywordType::New);
  keyword_map.Insert("not", Token::KeywordType::Not);
  keyword_map.Insert("number", Token::KeywordType::Number);
  keyword_map.Insert("operator", Token::KeywordType::Operator);
  keyword_map.Insert("or", Token::KeywordType::Or);
  keyword_map.Insert("private", Token::KeywordType::Private);
  keyword_map.Insert("protected", Token::KeywordType::Protected);
  keyword_map.Insert("public", Token::KeywordType::Public);
  keyword_map.Insert("return", Token::KeywordType::Return);
  keyword_map.Insert("short", Token::KeywordType::Short);
  keyword_map.Insert("signed", Token::KeywordType::Signed);
  keyword_map.Insert("sizeof", Token::KeywordType::Sizeof);
  keyword_map.Insert("static", Token::KeywordType::Static);
  keyword_map.Insert("string", Token::KeywordType::String);
  keyword_map.Insert("struct", Token::KeywordType::Struct);
  keyword_map.Insert("switch", Token::KeywordType::Switch);
  keyword_map.Insert("template", Token::KeywordType::Template);
  keyword_map.Insert("this", Token::KeywordType::This);
  keyword_map.Insert("thread", Token::KeywordType::Thread);
  keyword_map.Insert("true", Token::KeywordType::True);
  keyword_map.Insert("try", Token::KeywordType::Try);
  keyword_map.Insert("typedef", Token::KeywordType::Typedef);
  keyword_map.Insert("typeid", Token::KeywordType::Typeid);
  keyword_map.Insert("typename", Token::KeywordType::Typename);
  keyword_map.Insert("union", Token::KeywordType::Union);
  keyword_map.Insert("unsigned", Token::KeywordType::Unsigned);
  keyword_map.Insert("using", Token::KeywordType::Using);
  keyword_map.Insert("virtual", Token::KeywordType::Virtual);
  keyword_map.Insert("void", Token::KeywordType::Void);
  keyword_map.Insert("wchar_t", Token::KeywordType::Wchar_t);
  keyword_map.Insert("while", Token::KeywordType::While);
  keyword_map.Insert("xor", Token::KeywordType::Xor);

  operator_map.Insert("[", Token::OperatorType::l_square);
  operator_map.Insert("]", Token::OperatorType::r_square);
  operator_map.Insert("(", Token::OperatorType::l_paren);
  operator_map.Insert(")", Token::OperatorType::r_paren);
  operator_map.Insert("{", Token::OperatorType::l_brace);
  operator_map.Insert("}", Token::OperatorType::r_brace);
  operator_map.Insert(".", Token::OperatorType::period);
  operator_map.Insert("...", Token::OperatorType::ellipsis);
  operator_map.Insert("&", Token::OperatorType::amp);
  operator_map.Insert("&&", Token::OperatorType::ampamp);
  operator_map.Insert("&=", Token::OperatorType::ampequal);
  operator_map.Insert("*", Token::OperatorType::star);
  operator_map.Insert("*=", Token::OperatorType::starequal);
  operator_map.Insert("+", Token::OperatorType::plus);
  operator_map.Insert("++", Token::OperatorType::plusplus);
  operator_map.Insert("+=", Token::OperatorType::plusequal);
  operator_map.Insert("-", Token::OperatorType::minus);
  operator_map.Insert("->", Token::OperatorType::arrow);
  operator_map.Insert("--", Token::OperatorType::minusminus);
  operator_map.Insert("-=", Token::OperatorType::minusequal);
  operator_map.Insert("~", Token::OperatorType::tilde);
  operator_map.Insert("!", Token::OperatorType::exclaim);
  operator_map.Insert("!=", Token::OperatorType::exclaimequal);
  operator_map.Insert("/", Token::OperatorType::slash);
  operator_map.Insert("/=", Token::OperatorType::slashequal);
  operator_map.Insert("%", Token::OperatorType::percent);
  operator_map.Insert("%=", Token::OperatorType::percentequal);
  operator_map.Insert("<", Token::OperatorType::less);
  operator_map.Insert("<<", Token::OperatorType::lessless);
  operator_map.Insert("<=", Token::OperatorType::lessequal);
  operator_map.Insert("<<=", Token::OperatorType::lesslessequal);
  operator_map.Insert("<=>", Token::OperatorType::spaceship);
  operator_map.Insert(">", Token::OperatorType::greater);
  operator_map.Insert(">>", Token::OperatorType::greatergreater);
  operator_map.Insert(">=", Token::OperatorType::greaterequal);
  operator_map.Insert(">>=", Token::OperatorType::greatergreaterequal);
  operator_map.Insert("^", Token::OperatorType::caret);
  operator_map.Insert("^=", Token::OperatorType::caretequal);
  operator_map.Insert("|", Token::OperatorType::pipe);
  operator_map.Insert("||", Token::OperatorType::pipepipe);
  operator_map.Insert("|=", Token::OperatorType::pipeequal);
  operator_map.Insert("?", Token::OperatorType::question);
  operator_map.Insert(":", Token::OperatorType::colon);
  operator_map.Insert(";", Token::OperatorType::semi);
  operator_map.Insert("=", Token::OperatorType::equal);
  operator_map.Insert("==", Token::OperatorType::equalequal);
  operator_map.Insert(",", Token::OperatorType::comma);
  operator_map.Insert("#", Token::OperatorType::hash);
  operator_map.Insert("##", Token::OperatorType::hashhash);
  operator_map.Insert("#@", Token::OperatorType::hashat);
  operator_map.Insert(".*", Token::OperatorType::periodstar);
  operator_map.Insert("->*", Token::OperatorType::arrowstar);
  operator_map.Insert("::", Token::OperatorType::coloncolon);
  operator_map.Insert("@", Token::OperatorType::at);
  operator_map.Insert("<<<", Token::OperatorType::lesslessless);
  operator_map.Insert(">>>", Token::OperatorType::greatergreatergreater);
  operator_map.Insert("^^", Token::OperatorType::caretcaret);
}
TokenMap::~TokenMap() = default;

Token::KeywordType TokenMap::GetKeywordValue(std::string keyword) {
  return keyword_map.Find(keyword);
}
Token::OperatorType TokenMap::GetOperatorValue(std::string _operator) {
  return operator_map.Find(_operator);
}

class Lexer {
 public:
  // Initialize the Lexer class and store |source_code| to |buffer_ptr_|.
  Lexer(char* source_code, std::size_t length)
      : buffer_ptr_(source_code), buffer_end_(source_code + length - 1){};
  ~Lexer() = default;

  Lexer(const Lexer&) = default;
  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(const Lexer&) = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  // Lexical analysis |buffer_ptr_|, and store the analyzed token into
  // |return_token|.
  int LexToken(Token last_token, Token& return_token);

  // Return true if the lexer is at the end of |buffer_ptr_|.
  bool IsReadEnd() const;

 private:
  char* buffer_ptr_;
  char* buffer_end_;
  TokenMap token_map_;
};

bool Lexer::IsReadEnd() const {
  if (buffer_ptr_ >= buffer_end_) {
    return true;
  } else {
    return false;
  }
}

int Lexer::LexToken(Token last_token, Token& return_token) {
  // Set the return token type to start.
  return_token.type = Token::Type::START;

  // Set the reading position pointer equal to the buffer pointer.
  char* read_ptr = buffer_ptr_;

LexStart:
  // Memory out of bounds occurred. Return an error.
  if (read_ptr > buffer_end_) {
    buffer_ptr_ = read_ptr;
    return -1;
  }

  // Start lexical analysis.
  switch (*read_ptr) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // General operators.
    case '!':
    case '#':
    case '$':
    case '%':
    case '&':
    case '*':
    case ':':
    case '<':
    case '=':
    case '>':
    case '?':
    case '@':
    case '^':
    case '`':
    case '|':
    case '~':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // The string flag.
    case '"':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::STRING;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // The character flag.
    case '\'':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::CHARACTER;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER) {
        read_ptr++;
        goto LexEnd;
      } else {
        goto LexEnd;
      }

    // Escape character.
    case '\\':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        // Skip escape characters.
        if (read_ptr + 2 <= buffer_end_) {
          read_ptr += 2;
          goto LexStart;
        } else {
          read_ptr++;
          goto LexStart;
        }
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Positive and negative numbers.
    case '+':
    case '-':
      if (return_token.type == Token::Type::START) {
        if (last_token.type != Token::Type::IDENTIFIER &&
            last_token.type != Token::Type::NUMBER &&
            (*(read_ptr + 1) == '0' || *(read_ptr + 1) == '1' ||
             *(read_ptr + 1) == '2' || *(read_ptr + 1) == '3' ||
             *(read_ptr + 1) == '4' || *(read_ptr + 1) == '5' ||
             *(read_ptr + 1) == '6' || *(read_ptr + 1) == '7' ||
             *(read_ptr + 1) == '8' || *(read_ptr + 1) == '9')) {
          return_token.type = Token::Type::NUMBER;
        } else {
          return_token.type = Token::Type::OPERATOR;
        }
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::NUMBER) {
        // Dealing with scientific notation.
        if (*(read_ptr - 1) == 'E' || *(read_ptr - 1) == 'e') {
          read_ptr++;
          goto LexStart;
        } else {
          goto LexEnd;
        }
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Decimal point.
    case '.':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::OPERATOR ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // The comment flag.
    case '/':
      if (return_token.type == Token::Type::START) {
        if (*(buffer_ptr_ + 1) == '/' || *(buffer_ptr_ + 1) == '*') {
          return_token.type = Token::Type::COMMENT;
          if (read_ptr + 2 <= buffer_end_) {
            read_ptr += 2;
            goto LexStart;
          } else {
            read_ptr++;
            goto LexStart;
          }
        } else {
          return_token.type = Token::Type::OPERATOR;
          read_ptr++;
        }
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::OPERATOR) {
        if (*(read_ptr + 1) == '/' || *(read_ptr + 1) == '*') {
          goto LexEnd;
        } else {
          read_ptr++;
          goto LexStart;
        }
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          if (*(read_ptr - 1) == '*') {
            // /**/ style comments, skip all comments.
            buffer_ptr_ = ++read_ptr;
            return_token.type = Token::Type::START;
            goto LexStart;
          } else {
            // Non-end comment mark, continue reading until the end mark of the
            // comment.
            read_ptr++;
            goto LexStart;
          }
        } else {
          // // style comments, continue reading until newlines are skipped.
          read_ptr++;
          goto LexStart;
        }
      } else {
        goto LexEnd;
      }

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
      if (return_token.type == Token::Type::START) {
        read_ptr++;
        return_token.type = Token::Type::NUMBER;
        goto LexStart;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Whitespace characters.
    case '\f':
    case '\r':
    case '\t':
    case '\v':
    case ' ':
      if (return_token.type == Token::Type::START) {
        // Skip whitespace characters.
        read_ptr++;
        buffer_ptr_++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    // Newlines.
    case '\n':
      if (return_token.type == Token::Type::START) {
        // Skip newlines.
        read_ptr++;
        buffer_ptr_++;
        goto LexStart;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING) {
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::COMMENT) {
        if (*(buffer_ptr_ + 1) == '*') {
          // /**/ style comments, continue reading until the end mark of the
          // comment.
          read_ptr++;
          goto LexStart;
        } else {
          // // style comments, skip all comments.
          buffer_ptr_ = ++read_ptr;
          return_token.type = Token::Type::START;
          goto LexStart;
        }
      } else {
        goto LexEnd;
      }

    // EOF.
    case '\0':
      goto LexEnd;

    // Separator flag.
    case ',':
    case ';':
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::OPERATOR;
        read_ptr++;
        goto LexEnd;
      } else if (return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }

    default:
      if (return_token.type == Token::Type::START) {
        return_token.type = Token::Type::IDENTIFIER;
        read_ptr++;
        goto LexStart;
      } else if (return_token.type == Token::Type::IDENTIFIER ||
                 return_token.type == Token::Type::NUMBER ||
                 return_token.type == Token::Type::CHARACTER ||
                 return_token.type == Token::Type::STRING ||
                 return_token.type == Token::Type::COMMENT) {
        read_ptr++;
        goto LexStart;
      } else {
        goto LexEnd;
      }
  }

LexEnd:
  // Meaningless token.
  if (return_token.type == Token::Type::START ||
      return_token.type == Token::Type::COMMENT) {
    return_token.type = Token::Type::NONE;
    buffer_ptr_ = read_ptr;
    return 0;
  } else {
    // Meaningful token. Determine the specific token information.
    char* location = buffer_ptr_;
    std::size_t length = read_ptr - buffer_ptr_;
    buffer_ptr_ = read_ptr;

    // Handle the detailed information of tokens.
    Token::ValueStr value;
    value.location = location;
    value.length = length;

    switch (return_token.type) {
      case Token::Type::IDENTIFIER:
        return_token.value.keyword =
            token_map_.GetKeywordValue(std::string(location, length));
        if (return_token.value.keyword == Token::KeywordType::NONE) {
          return_token.value.identifier = value;
          break;
        }
        return_token.type = Token::Type::KEYWORD;
        break;

      case Token::Type::CHARACTER:
        return_token.value.character = value.location[1];
        break;

      case Token::Type::STRING:;
        return_token.value.string =
            new std::string(value.location + 1, length - 2);
        break;

      case Token::Type::OPERATOR:
        return_token.value._operator =
            token_map_.GetOperatorValue(std::string(location, length));
        while (return_token.value._operator == Token::OperatorType::NONE &&
               length > 1) {
          length--;
          buffer_ptr_--;
          return_token.value._operator =
              token_map_.GetOperatorValue(std::string(location, length));
        }
        break;

      case Token::Type::NUMBER:
        return_token.value.number = value;
        break;

      default:
        return -1;
    }
  }
  return 0;
}

class Type {
 public:
  enum class TypeType { NONE, kBase, kConst, kPointer, kArray, kReference };

  enum class BaseType {
    NONE,
    kVoid,
    kBool,
    kChar,
    kShort,
    kInt,
    kLong,
    kFloat,
    kDouble,
    kStruct,
    kUnion,
    kEnum,
    kPointer,
    kArray,
    kFunction,
    kTypedef,
    kAuto
  };

  Type() {
    type_ = TypeType::kBase;
    base_data_ = BaseType::NONE;
    type_data_ = nullptr;
  }
  virtual void SetType(BaseType type) { base_data_ = type; }
  virtual ~Type() = default;

  Type(const Type&) = default;
  Type& operator=(const Type&) = default;

  TypeType GetType() { return type_; }

  BaseType GetBaseType() { return base_data_; }

  static Type* CreateType(Token* token, std::size_t length, std::size_t& index);

  virtual std::size_t GetSize() {
    switch (base_data_) {
      case BaseType::kVoid:
      case BaseType::kBool:
      case BaseType::kChar:
        return 1;
      case BaseType::kShort:
      case BaseType::kInt:
      case BaseType::kFloat:
        return 4;
      case BaseType::kLong:
      case BaseType::kDouble:
      case BaseType::kStruct:
      case BaseType::kUnion:
      case BaseType::kEnum:
      case BaseType::kPointer:
      case BaseType::kArray:
      case BaseType::kFunction:
      case BaseType::kTypedef:
      case BaseType::kAuto:
        return 8;
      default:
        return 1;
    }
  }

 protected:
  TypeType type_ = TypeType::NONE;
  Type* type_data_;

 private:
  BaseType base_data_ = BaseType::NONE;

  Type* CreateBaseType(Token* token, std::size_t length, std::size_t& index);
};

class StmtNode {
 public:
  StmtNode() { type_ = StmtType::kStmt; }
  virtual ~StmtNode() = default;

  enum class StmtType {
    kStmt,
    kCompound,
    kDecl,
    kExpr,
    kFuncDecl,
    kVarDecl,
    kIf,
    kWhile,
    kGoto,
    kValue,
    kIdentifier,
    kUnary,
    kBinary,
    kConditional,
    kFunc,
    kCast,
    kArrayDecl,
    kArray,
    kConvert
  };

  StmtType GetType() { return type_; }

  StmtNode(const StmtNode&) = default;
  StmtNode& operator=(const StmtNode&) = default;

 protected:
  StmtType type_;
};

class CompoundNode : public StmtNode {
 public:
  CompoundNode() { type_ = StmtType::kCompound; }
  virtual ~CompoundNode() = default;

  void SetCompoundNode(std::vector<StmtNode*> stmts) { stmts_ = stmts; }

  std::vector<StmtNode*> GetStmts() { return stmts_; }

  CompoundNode(const CompoundNode&) = default;
  CompoundNode& operator=(const CompoundNode&) = default;

 private:
  std::vector<StmtNode*> stmts_;
};

class ExprNode : public StmtNode {
 public:
  ExprNode() { type_ = StmtType::kExpr; }
  virtual ~ExprNode() = default;

  virtual operator std::string();

  ExprNode(const ExprNode&) = default;
  ExprNode& operator=(const ExprNode&) = default;
};

class ValueNode : public ExprNode {
 public:
  ValueNode() { type_ = StmtType::kValue; }
  void SetValueNode(Token value) { value_ = value; }
  virtual ~ValueNode() = default;

  char GetCharValue() { return value_.value.character; }
  std::string GetStringValue() { return *value_.value.string; }
  int GetIntValue() {
    return std::stoi(
        std::string(value_.value.number.location, value_.value.number.length));
  }
  long GetLongValue() {
    return std::stol(
        std::string(value_.value.number.location, value_.value.number.length));
  }
  float GetFloatValue() {
    return std::stof(
        std::string(value_.value.number.location, value_.value.number.length));
  }
  double GetDoubleValue() {
    return std::stod(
        std::string(value_.value.number.location, value_.value.number.length));
  }
  uint64_t GetUInt64Value() {
    return std::stoull(
        std::string(value_.value.number.location, value_.value.number.length));
  }

  /*std::variant<char,const char*, int, long, float, double, uint64_t>
  GetValue() { if (value_.type == Token::Type::CHARACTER) { return
  value_.value.character;
    }
    if (value_.type == Token::Type::STRING) {
      return value_.value.string.c_str();
    }

    std::string str(value_.value.number.location, value_.value.number.length);
    try {
      std::size_t pos;
      int int_value = std::stoi(str, &pos);
      if (pos == str.size()) {
        return int_value;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      long long_value = std::stol(str, &pos);
      if (pos == str.size()) {
        return long_value;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      uint64_t uint64_value = std::stoull(str, &pos);
      if (pos == str.size()) {
        return uint64_value;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      float float_value = std::stof(str, &pos);
      if (pos == str.size()) {
        return float_value;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      double double_value = std::stod(str, &pos);
      if (pos == str.size()) {
        return double_value;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    return 0;
  }*/

  std::size_t GetVmType() {
    if (value_.type == Token::Type::CHARACTER) {
      return 0x01;
    }
    if (value_.type == Token::Type::STRING) {
      return 0x06;
    }

    std::string str(value_.value.number.location, value_.value.number.length);
    try {
      std::size_t pos;
      (void)std::stoi(str, &pos);
      if (pos == str.size()) {
        return 0x02;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stol(str, &pos);
      if (pos == str.size()) {
        return 0x03;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stoull(str, &pos);
      if (pos == str.size()) {
        return 0x06;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stof(str, &pos);
      if (pos == str.size()) {
        return 0x04;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stod(str, &pos);
      if (pos == str.size()) {
        return 0x05;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    return 0x00;
  }

  std::size_t GetSize() {
    if (value_.type == Token::Type::CHARACTER) {
      return 1;
    }
    if (value_.type == Token::Type::STRING) {
      return value_.value.string->size();
    }
    switch (GetVmType()) {
      case 0x02:
        return 4;
      case 0x03:
        return 8;
      case 0x04:
        return 4;
      case 0x05:
        return 8;
      case 0x06:
        return 8;
      default:
        return 0;
    }
  }

  Token GetToken() { return value_; }

  ValueNode(const ValueNode&) = default;
  ValueNode& operator=(const ValueNode&) = default;

 private:
  Token value_;
};

class UnaryNode : public ExprNode {
 public:
  enum class Operator {
    NONE,
    kPostInc,
    kPostDec,
    kPreInc,
    kPreDec,
    kAddrOf,
    kDeref,
    kPlus,
    kMinus,
    kNot,
    kBitwiseNot,
    ARRAY,
    CONVERT
  };

  UnaryNode() {
    type_ = StmtType::kUnary;
    expr_ = nullptr;
    op_ = Operator::NONE;
  }
  void SetUnaryNode(Operator op, ExprNode* expr) {
    op_ = op;
    expr_ = expr;
  }

  Operator GetOperator() { return op_; }

  ExprNode* GetExpr() { return expr_; }

  virtual ~UnaryNode() = default;

  UnaryNode(const UnaryNode&) = default;
  UnaryNode& operator=(const UnaryNode&) = default;

 protected:
  Operator op_;
  ExprNode* expr_;
};

class ArrayNode : public UnaryNode {
 public:
  ArrayNode() {
    type_ = StmtType::kArray;
    op_ = Operator::ARRAY;
    index_ = nullptr;
  }
  virtual ~ArrayNode() = default;

  void SetArrayNode(ExprNode* expr, ExprNode* index) {
    expr_ = expr;
    index_ = index;
  }

  ExprNode* GetIndex() { return index_; }

  ArrayNode(const ArrayNode&) = default;
  ArrayNode& operator=(const ArrayNode&) = default;

 private:
  ExprNode* index_;
};

class ConvertNode : public UnaryNode {
 public:
  ConvertNode() {
    type_ = StmtType::kConvert;
    op_ = Operator::CONVERT;
    converted_type_ = nullptr;
  }
  void SetConvertNode(Type* type, ExprNode* expr) {
    converted_type_ = type;
    expr_ = expr;
  }
  virtual ~ConvertNode() = default;

  Type* GetConvertedType() { return converted_type_; }

  ConvertNode(const ConvertNode&) = default;
  ConvertNode& operator=(const ConvertNode&) = default;

 private:
  Type* converted_type_;
};

class BinaryNode : public ExprNode {
 public:
  enum class Operator {
    NONE,
    kAdd,        // +
    kSub,        // -
    kMul,        // *
    kDiv,        // /
    kRem,        // %
    kAnd,        // &
    kOr,         // |
    kXor,        // ^
    kShl,        // <<
    kShr,        // >>
    kLT,         // <
    kGT,         // >
    kLE,         // <=
    kGE,         // >=
    kEQ,         // ==
    kNE,         // !=
    kLAnd,       // &&
    kLOr,        // ||
    kAssign,     // =
    kAddAssign,  // +=
    kSubAssign,  // -=
    kMulAssign,  // *=
    kDivAssign,  // /=
    kRemAssign,  // %=
    kAndAssign,  // &=
    kOrAssign,   // |=
    kXorAssign,  // ^=
    kShlAssign,  // <<=
    kShrAssign,  // >>=
    kComma,      // ,
    kPtrMemD,    // .*
    kPtrMemI,    // ->*
  };

  BinaryNode() {
    type_ = StmtType::kBinary;
    left_ = nullptr;
    op_ = Operator::NONE;
    right_ = nullptr;
  }
  void SetBinaryNode(Operator op, ExprNode* left, ExprNode* right) {
    op_ = op;
    left_ = left;
    right_ = right;
  }
  virtual ~BinaryNode() = default;

  Operator GetOperator() { return op_; }

  ExprNode* GetLeftExpr() { return left_; }
  ExprNode* GetRightExpr() { return right_; }

  BinaryNode(const BinaryNode&) = default;
  BinaryNode& operator=(const BinaryNode&) = default;

 private:
  Operator op_;
  ExprNode* left_;
  ExprNode* right_;
};

class ConditionalNode : public ExprNode {
 public:
  ConditionalNode() {
    type_ = StmtType::kConditional;
    condition_ = nullptr;
    false_expr_ = nullptr;
    true_expr_ = nullptr;
  }
  void SetConditionalNode(ExprNode* condition, ExprNode* true_expr,
                          ExprNode* false_expr) {
    condition_ = condition;
    true_expr_ = true_expr;
    false_expr_ = false_expr;
  }
  virtual ~ConditionalNode() = default;

  ExprNode* GetTrueExpr() { return true_expr_; }
  ExprNode* GetFalseExpr() { return false_expr_; }

  ConditionalNode(const ConditionalNode&) = default;
  ConditionalNode& operator=(const ConditionalNode&) = default;

 private:
  ExprNode* condition_;
  ExprNode* true_expr_;
  ExprNode* false_expr_;
};

class FuncNode : public ExprNode {
 public:
  FuncNode() {
    type_ = StmtType::kFunc;
    name_ = nullptr;
  }
  void SetFuncNode(ExprNode* name, std::vector<ExprNode*> args) {
    name_ = name;
    args_ = args;
  }
  virtual ~FuncNode() = default;

  ExprNode* GetName() { return name_; }
  std::vector<ExprNode*> GetArgs() { return args_; }

  FuncNode(const FuncNode&) = default;
  FuncNode& operator=(const FuncNode&) = default;

 private:
  ExprNode* name_;
  std::vector<ExprNode*> args_;
};

class IdentifierNode : public ExprNode {
 public:
  IdentifierNode() { type_ = StmtType::kIdentifier; }
  void SetIdentifierNode(Token name) { name_ = name; }
  virtual ~IdentifierNode() = default;

  operator std::string() override {
    return std::string(name_.value.identifier.location,
                       name_.value.identifier.length);
  }

  IdentifierNode(const IdentifierNode&) = default;
  IdentifierNode& operator=(const IdentifierNode&) = default;

 private:
  Token name_;
};

class DeclNode : public StmtNode {
 public:
  DeclNode() { type_ = StmtType::kDecl; }
  virtual ~DeclNode() = default;

  DeclNode(const DeclNode&) = default;
  DeclNode& operator=(const DeclNode&) = default;
};

class VarDeclNode : public DeclNode, public ExprNode {
 public:
  VarDeclNode() {
    DeclNode::type_ = StmtType::kVarDecl;
    name_ = nullptr;
    var_type_ = nullptr;
  }
  void SetVarDeclNode(Type* type, ExprNode* name) {
    var_type_ = type;
    name_ = name;
    value_.push_back(nullptr);
  }
  void SetVarDeclNode(Type* type, ExprNode* name, ExprNode* value) {
    var_type_ = type;
    name_ = name;
    value_[0] = value;
  }

  virtual ~VarDeclNode() = default;

  VarDeclNode(const VarDeclNode&) = default;
  VarDeclNode& operator=(const VarDeclNode&) = default;

  Type* GetVarType() { return var_type_; }
  ExprNode* GetName() { return name_; }
  std::vector<ExprNode*> GetValue() { return value_; }

 protected:
  Type* var_type_;
  ExprNode* name_;
  std::vector<ExprNode*> value_;
};

class ArrayDeclNode : public VarDeclNode {
 public:
  ArrayDeclNode() {
    DeclNode::type_ = StmtType::kArrayDecl;
    size_ = nullptr;
  }
  void SetArrayDeclNode(Type* type, ExprNode* name, ExprNode* size) {
    var_type_ = type;
    name_ = name;
    size_ = size;
    value_.push_back(new ExprNode());
  }
  void SetArrayDeclNode(Type* type, ExprNode* name, ExprNode* size,
                        std::vector<ExprNode*> value) {
    var_type_ = type;
    name_ = name;
    size_ = size;
    value_ = value;
  }

  virtual ~ArrayDeclNode() = default;

  ArrayDeclNode(const ArrayDeclNode&) = default;
  ArrayDeclNode& operator=(const ArrayDeclNode&) = default;

 private:
  ExprNode* size_;
};

class FuncDeclNode : public DeclNode {
 public:
  FuncDeclNode() {
    type_ = StmtType::kFuncDecl;
    return_type_ = nullptr;
    stat_ = nullptr;
    stmts_ = nullptr;
  }
  void SetFuncDeclNode(Type* type, FuncNode* stat, CompoundNode* stmts) {
    return_type_ = type;
    stat_ = stat;
    stmts_ = stmts;
  }
  virtual ~FuncDeclNode() = default;

  Type* GetReturnType() { return return_type_; }
  FuncNode* GetStat() { return stat_; }
  CompoundNode* GetStmts() { return stmts_; }

  FuncDeclNode(const FuncDeclNode&) = default;
  FuncDeclNode& operator=(const FuncDeclNode&) = default;

 private:
  Type* return_type_;
  FuncNode* stat_;
  CompoundNode* stmts_;
};

class IfNode : public StmtNode {
 public:
  IfNode() {
    type_ = StmtType::kIf;
    body_ = nullptr;
    condition_ = nullptr;
    else_body_ = nullptr;
  }
  void SetIfNode(ExprNode* condition, StmtNode* body) {
    condition_ = condition;
    body_ = body;
  }

  void SetIfNode(ExprNode* condition, StmtNode* body, StmtNode* else_body) {
    condition_ = condition;
    body_ = body;
    else_body_ = else_body;
  }

  ExprNode* GetCondition() { return condition_; }
  StmtNode* GetBody() { return body_; }
  StmtNode* GetElseBody() { return else_body_; }

  IfNode(const IfNode&) = default;
  IfNode& operator=(const IfNode&) = default;

 private:
  ExprNode* condition_;
  StmtNode* body_;
  StmtNode* else_body_;
};

class WhileNode : public StmtNode {
 public:
  WhileNode() {
    type_ = StmtType::kWhile;
    body_ = nullptr;
    condition_ = nullptr;
  }
  virtual ~WhileNode() = default;

  void SetWhileNode(ExprNode* condition, StmtNode* body) {
    condition_ = condition;
    body_ = body;
  }

  ExprNode* GetCondition() { return condition_; }
  StmtNode* GetBody() { return body_; }

  WhileNode(const WhileNode&) = default;
  WhileNode& operator=(const WhileNode&) = default;

 private:
  ExprNode* condition_;
  StmtNode* body_;
};

class GotoNode : public StmtNode {
 public:
  GotoNode() { type_ = StmtType::kGoto; }
  void SetGotoNode(IdentifierNode label) { label_ = label; }
  virtual ~GotoNode() = default;

  IdentifierNode GetLabel() { return label_; }

  GotoNode(const GotoNode&) = default;
  GotoNode& operator=(const GotoNode&) = default;

 private:
  IdentifierNode label_;
};

class CastNode : public ExprNode {
 public:
  CastNode() {
    type_ = StmtType::kCast;
    cast_type_ = nullptr;
    expr_ = nullptr;
  }
  void SetCastNode(Type* type, ExprNode* expr) {
    cast_type_ = type;
    expr_ = expr;
  }
  virtual ~CastNode() = default;

  Type* GetCastType() { return cast_type_; }

  CastNode(const CastNode&) = default;
  CastNode& operator=(const CastNode&) = default;

 private:
  Type* cast_type_;
  ExprNode* expr_;
};

ExprNode::operator std::string() {
  // TODO
  if (type_ == StmtType::kIdentifier)
    return *dynamic_cast<IdentifierNode*>(this);
  return std::string();
}

class Parser {
 public:
  Parser();
  ~Parser();
  static CompoundNode* Parse(std::vector<Token> token);

  static ExprNode* ParseExpr(Token* token, std::size_t length,
                             std::size_t& index);

  static ExprNode* ParsePrimaryExpr(Token* token, std::size_t length,
                                    std::size_t& index);

 private:
  static bool IsDecl(Token* token, std::size_t length, std::size_t index);
  static bool IsFuncDecl(Token* token, std::size_t length, std::size_t index);
  static StmtNode* ParseStmt(Token* token, std::size_t length,
                             std::size_t& index);
  static VarDeclNode* ParseVarDecl(Token* token, std::size_t length,
                                   std::size_t& index);
  static FuncDeclNode* ParseFuncDecl(Token* token, std::size_t length,
                                     std::size_t& index);
  static ExprNode* ParseBinaryExpr(Token* token, std::size_t length,
                                   std::size_t& index, ExprNode* left,
                                   unsigned int priority);
  static unsigned int GetPriority(Token token);
};

class ConstType : public Type {
 public:
  ConstType() { type_ = TypeType::kConst; }
  void SetSubType(Type* type) {
    type_ = TypeType::kConst;
    type_data_ = type;
  }
  virtual ~ConstType() = default;

  Type* GetSubType() { return type_data_; }

  std::size_t GetSize() override { return type_data_->GetSize(); }

  ConstType(const ConstType&) = default;
  ConstType& operator=(const ConstType&) = default;
};

class PointerType : public Type {
 public:
  PointerType() { type_ = TypeType::kPointer; }
  void SetSubType(Type* type) {
    type_ = TypeType::kPointer;
    type_data_ = type;
  }
  virtual ~PointerType() = default;

  Type* GetSubType() { return type_data_; }

  std::size_t GetSize() override { return 8; }

  PointerType(const PointerType&) = default;
  PointerType& operator=(const PointerType&) = default;
};

class ArrayType : public Type {
 public:
  ArrayType() {
    type_ = TypeType::kArray;
    size_ = nullptr;
  }
  void SetSubType(Type* type, ExprNode* size) {
    type_ = TypeType::kArray;
    type_data_ = type;
    size_ = size;
  }
  virtual ~ArrayType() = default;

  Type* GetSubType() { return type_data_; }

  ExprNode* GetArraySize() { return size_; }

  std::size_t GetSize() override { return 8; }

  ArrayType(const ArrayType&) = default;
  ArrayType& operator=(const ArrayType&) = default;

 private:
  ExprNode* size_;
};

class ReferenceType : public Type {
 public:
  ReferenceType() { type_ = TypeType::kReference; }
  void SetSubType(Type* type) {
    type_ = TypeType::kReference;
    type_data_ = type;
  }
  virtual ~ReferenceType() = default;

  Type* GetSubType() { return type_data_; }

  ReferenceType(const ReferenceType&) = default;
  ReferenceType& operator=(const ReferenceType&) = default;
};

Type* Type::CreateType(Token* token, std::size_t length, std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  Type* type = nullptr;
  while (index < length) {
    if (token[index].type == Token::Type::KEYWORD) {
      switch (token[index].value.keyword) {
        case Token::KeywordType::Const: {
          ConstType* const_type = new ConstType();
          if (const_type == nullptr)
            EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                          "const_type is nullptr.");

          if (index + 1 < length &&
              token[index + 1].type == Token::Type::KEYWORD) {
            index++;
            switch (token[index].value.keyword) {
              case Token::KeywordType::Void:
                type->SetType(Type::BaseType::kVoid);
                break;

              case Token::KeywordType::Bool:
                type->SetType(Type::BaseType::kBool);
                break;

              case Token::KeywordType::Char:
                type->SetType(Type::BaseType::kChar);
                break;
              case Token::KeywordType::Short:
                type->SetType(Type::BaseType::kShort);
                break;

              case Token::KeywordType::Int:
                type->SetType(Type::BaseType::kInt);
                break;

              case Token::KeywordType::Long:
                type->SetType(Type::BaseType::kLong);
                break;

              case Token::KeywordType::Float:
                type->SetType(Type::BaseType::kFloat);
                break;

              case Token::KeywordType::Double:
                type->SetType(Type::BaseType::kDouble);
                break;

              case Token::KeywordType::Auto:
                type->SetType(Type::BaseType::kAuto);
                break;

              default:
                break;
            }
          }

          if (type == nullptr)
            EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                          "type is nullptr.");

          const_type->SetSubType(type);
          type = const_type;
          break;
        }
        case Token::KeywordType::Void:
          type = new Type();
          type->SetType(Type::BaseType::kVoid);
          break;

        case Token::KeywordType::Bool:
          type = new Type();
          type->SetType(Type::BaseType::kBool);
          break;

        case Token::KeywordType::Char:
          type = new Type();
          type->SetType(Type::BaseType::kChar);
          break;

        case Token::KeywordType::Short:
          type = new Type();
          type->SetType(Type::BaseType::kShort);
          break;

        case Token::KeywordType::Int:
          type = new Type();
          type->SetType(Type::BaseType::kInt);
          break;

        case Token::KeywordType::Long:
          type = new Type();
          type->SetType(Type::BaseType::kLong);
          break;

        case Token::KeywordType::Float:
          type = new Type();
          type->SetType(Type::BaseType::kFloat);
          break;

        case Token::KeywordType::Double:
          type = new Type();
          type->SetType(Type::BaseType::kDouble);
          break;

        case Token::KeywordType::Struct:
          type = new Type();
          type->SetType(Type::BaseType::kStruct);
          return type;

        case Token::KeywordType::Union:
          type = new Type();
          type->SetType(Type::BaseType::kUnion);
          return type;

        case Token::KeywordType::Enum:
          type = new Type();
          type->SetType(Type::BaseType::kEnum);
          return type;

        case Token::KeywordType::Auto:
          type = new Type();
          type->SetType(Type::BaseType::kAuto);
          break;

        default:
          return type;
      }
    } else if (token[index].type == Token::Type::OPERATOR) {
      if (type == nullptr)
        EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                      "type is nullptr.");

      switch (token[index].value._operator) {
        case Token::OperatorType::star: {
          PointerType* pointer_type = new PointerType();
          pointer_type->SetSubType(type);
          type = pointer_type;
          break;
        }

        case Token::OperatorType::amp: {
          ReferenceType* reference_type = new ReferenceType();
          reference_type->SetSubType(type);
          type = reference_type;
          break;
        }

        default:
          return type;
      }
    } else if (token[index].type == Token::Type::IDENTIFIER) {
      std::size_t index_temp = index;
      ExprNode* temp_expr = Parser::ParsePrimaryExpr(token, length, index_temp);
      if (temp_expr == nullptr)
        EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                      "ParsePrimaryExpr return nullptr.");

      if (temp_expr->GetType() == StmtNode::StmtType::kArray) {
        ArrayType* array_type = new ArrayType();
        array_type->SetSubType(type,
                               dynamic_cast<ArrayNode*>(temp_expr)->GetIndex());
        type = array_type;
      }

      // TODO: Add support of custom types.

      return type;
    }
    index++;
  }

  EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                "index is out of range.");
  return nullptr;
}

Parser::Parser() = default;
Parser::~Parser() = default;

CompoundNode* Parser::Parse(std::vector<Token> token) {
  TRACE_FUNCTION;
  Token* token_ptr = token.data();
  std::size_t index = 0;
  std::size_t length = token.size();
  CompoundNode* ast = nullptr;
  std::vector<StmtNode*> stmts;

  std::cout << token.size() << std::endl;

  // Delete the last NONE token.
  if (token_ptr[token.size() - 1].type == Token::Type::NONE) token.pop_back();

  while (index < token.size()) {
    if (IsDecl(token_ptr, length, index)) {
      if (IsFuncDecl(token_ptr, length, index)) {
        stmts.push_back(ParseFuncDecl(token_ptr, length, index));
      } else {
        stmts.push_back(
            dynamic_cast<DeclNode*>(ParseVarDecl(token_ptr, length, index)));
        if (token_ptr[index].type != Token::Type::OPERATOR ||
            token_ptr[index].value._operator != Token::OperatorType::semi) {
          EXIT_COMPILER("Parser::Parse(std::vector<Token>)", "not found semi.");
          return nullptr;
        }
        index++;
      }
    } else {
      EXIT_COMPILER("Parser::Parse(std::vector<Token>)", "Unexpected code.");
    }
  }

  ast = new CompoundNode();
  ast->SetCompoundNode(stmts);
  return ast;
}

bool Parser::IsDecl(Token* token, std::size_t length, std::size_t index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::IsDecl(Token*,std::size_t,std::size_t)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::IsDecl(Token*,std::size_t,std::size_t)",
                  "index is out of range.");

  if (token[index].type == Token::Type::KEYWORD) {
    if (token[index].value.keyword == Token::KeywordType::Auto ||
        token[index].value.keyword == Token::KeywordType::Bool ||
        token[index].value.keyword == Token::KeywordType::Char ||
        token[index].value.keyword == Token::KeywordType::Double ||
        token[index].value.keyword == Token::KeywordType::Float ||
        token[index].value.keyword == Token::KeywordType::Int ||
        token[index].value.keyword == Token::KeywordType::Long ||
        token[index].value.keyword == Token::KeywordType::Void ||
        token[index].value.keyword == Token::KeywordType::String ||
        token[index].value.keyword == Token::KeywordType::Struct ||
        token[index].value.keyword == Token::KeywordType::Union ||
        token[index].value.keyword == Token::KeywordType::Enum ||
        token[index].value.keyword == Token::KeywordType::Namespace ||
        token[index].value.keyword == Token::KeywordType::Template ||
        token[index].value.keyword == Token::KeywordType::Typedef ||
        token[index].value.keyword == Token::KeywordType::Extern ||
        token[index].value.keyword == Token::KeywordType::Class ||
        token[index].value.keyword == Token::KeywordType::Const ||
        token[index].value.keyword == Token::KeywordType::Friend ||
        token[index].value.keyword == Token::KeywordType::Inline ||
        token[index].value.keyword == Token::KeywordType::Number ||
        token[index].value.keyword == Token::KeywordType::Short ||
        token[index].value.keyword == Token::KeywordType::Signed ||
        token[index].value.keyword == Token::KeywordType::Unsigned ||
        token[index].value.keyword == Token::KeywordType::Virtual ||
        token[index].value.keyword == Token::KeywordType::Wchar_t) {
      return true;
    } else {
      return false;
    }
  } else if ((token[index].type == Token::Type::IDENTIFIER &&
              token[index + 1].type == Token::Type::IDENTIFIER) ||
             (token[index].type == Token::Type::IDENTIFIER &&
              token[index + 1].type == Token::Type::OPERATOR &&
              (token[index + 1].value._operator == Token::OperatorType::star ||
               token[index + 1].value._operator == Token::OperatorType::amp ||
               token[index + 1].value._operator ==
                   Token::OperatorType::ampamp) &&
              token[index + 2].type == Token::Type::IDENTIFIER)) {
    // TODO: Change the processing logic of custom types and add support of
    // custom types.
    return true;
  }

  return false;
}

bool Parser::IsFuncDecl(Token* token, std::size_t length, std::size_t index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::IsFuncDecl(Token*,std::size_t,std::size_t)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::IsFuncDecl(Token*,std::size_t,std::size_t)",
                  "index is out of range.");

  for (std::size_t i = index; i < length; i++) {
    if (token[i].type == Token::Type::OPERATOR &&
        token[i].value._operator == Token::OperatorType::semi) {
      return false;
    }
    if (token[i].type == Token::Type::OPERATOR &&
        token[i].value._operator == Token::OperatorType::l_paren) {
      return true;
    }
  }
  return false;
}

StmtNode* Parser::ParseStmt(Token* token, std::size_t length,
                            std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  if (IsDecl(token, length, index)) {
    if (IsFuncDecl(token, length, index)) {
      EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                    "FuncDecl not supported.");

    } else {
      VarDeclNode* result = ParseVarDecl(token, length, index);
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::semi)
        EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                      "semi not found.");

      index++;
      return dynamic_cast<DeclNode*>(result);
    }
  }

  switch (token[index].type) {
    case Token::Type::OPERATOR:
      switch (token[index].value._operator) {
        case Token::OperatorType::semi:
          index++;
          return new StmtNode();

        case Token::OperatorType::l_brace: {
          index++;
          CompoundNode* result = new CompoundNode();
          std::vector<StmtNode*> stmts;

          while (token[index].value._operator != Token::OperatorType::r_brace &&
                 index < length) {
            StmtNode* stmt = ParseStmt(token, length, index);
            if (stmt == nullptr)
              EXIT_COMPILER(
                  "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "StmtNode is nullptr.");
            stmts.push_back(stmt);
          }

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_brace)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "r_brace not found.");

          result->SetCompoundNode(stmts);
          index++;
          return result;
        }

        case Token::OperatorType::r_square:
        case Token::OperatorType::r_paren:
        case Token::OperatorType::r_brace:
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_brace)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unexpected r_brace.");

        default:
          StmtNode* stmt_node = ParseExpr(token, length, index);
          if (token[index].type == Token::Type::OPERATOR ||
              token[index].value._operator == Token::OperatorType::semi)
            index++;
          return stmt_node;
      }

    case Token::Type::KEYWORD:
      switch (token[index].value.keyword) {
        case Token::KeywordType::If: {
          index++;
          IfNode* result = new IfNode();

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "l_paren not found.");
          ExprNode* condition = ParseExpr(token, length, ++index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "r_paren not found.");

          StmtNode* body = ParseStmt(token, length, ++index);
          result->SetIfNode(condition, body);

          if (token[index].type == Token::Type::KEYWORD &&
              token[index].value.keyword == Token::KeywordType::Else) {
            result->SetIfNode(condition, body,
                              ParseStmt(token, length, ++index));
          }
          return result;
        }

        case Token::KeywordType::While: {
          index++;
          WhileNode* result = new WhileNode();

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "While condition l_paren not found.");
          ExprNode* condition = ParseExpr(token, length, ++index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "While condition r_paren not found.");

          result->SetWhileNode(condition, ParseStmt(token, length, ++index));
          return result;
        }

        case Token::KeywordType::Goto: {
          index++;
          GotoNode* result = new GotoNode();

          if (token[index].type != Token::Type::IDENTIFIER)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "IDENTIFIER not found.");
          IdentifierNode label;
          label.SetIdentifierNode(token[index]);
          index++;
          result->SetGotoNode(label);
          return result;
        }

        default:
          return nullptr;
      }

    default:
      StmtNode* stmt_node = ParseExpr(token, length, index);
      if (token[index].type == Token::Type::OPERATOR ||
          token[index].value._operator == Token::OperatorType::semi)
        index++;
      return stmt_node;
  }
}

FuncDeclNode* Parser::ParseFuncDecl(Token* token, std::size_t length,
                                    std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseFuncDecl(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseFuncDecl(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  FuncDeclNode* func_decl = nullptr;
  Type* type = Type::CreateType(token, length, index);
  ExprNode* stat = Parser::ParsePrimaryExpr(token, length, index);

  if (stat == nullptr || stat->GetType() != StmtNode::StmtType::kFunc)
    EXIT_COMPILER("Parser::ParseFuncDecl(Token*,std::size_t,std::size_t&)",
                  "stat is nullptr or not a function.");

  if (token[index].type != Token::Type::OPERATOR ||
      token[index].value._operator != Token::OperatorType::l_brace)
    EXIT_COMPILER("Parser::ParseFuncDecl(Token*,std::size_t,std::size_t&)",
                  "l_brace not found.");

  CompoundNode* stmts =
      dynamic_cast<CompoundNode*>(ParseStmt(token, length, index));

  func_decl = new FuncDeclNode();
  func_decl->SetFuncDeclNode(type, dynamic_cast<FuncNode*>(stat), stmts);

  return func_decl;
}

VarDeclNode* Parser::ParseVarDecl(Token* token, std::size_t length,
                                  std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  VarDeclNode* var_decl = new VarDeclNode();
  if (var_decl == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "var_decl is nullptr.");

  Type* type = Type::CreateType(token, length, index);
  if (type == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "type is nullptr.");
  ExprNode* name = ParsePrimaryExpr(token, length, index);
  if (name == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "name is nullptr.");
  var_decl->SetVarDeclNode(type, name);
  if (token[index].type != Token::Type::OPERATOR) {
    return var_decl;
  }

  switch (token[index].value._operator) {
    case Token::OperatorType::l_square: {
      ExprNode* size = ParseExpr(token, length, ++index);
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::r_square)
        EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                      "r_square not found.");
      if (token[index].type == Token::Type::OPERATOR &&
          token[index].value._operator == Token::OperatorType::equal) {
        if (token[index].type != Token::Type::OPERATOR ||
            token[index].value._operator != Token::OperatorType::l_brace)
          EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                        "l_brace not found.");

        std::vector<ExprNode*> values;
        while (true) {
          values.push_back(ParseExpr(token, length, ++index));
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::r_brace)
            break;
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::comma)
            EXIT_COMPILER(
                "Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                "comma not found.");
        }

        var_decl = new ArrayDeclNode();
        dynamic_cast<ArrayDeclNode*>(var_decl)->SetArrayDeclNode(type, name,
                                                                 size, values);
      }

      dynamic_cast<ArrayDeclNode*>(var_decl)->SetArrayDeclNode(type, name,
                                                               size);
      break;
    }

    case Token::OperatorType::equal: {
      std::cout << "VarDecl has EQUAL." << std::endl;
      ExprNode* value = ParseExpr(token, length, ++index);
      var_decl->SetVarDeclNode(type, name, value);
      break;
    }

    default:
      return var_decl;
  }
  return var_decl;
}

ExprNode* Parser::ParsePrimaryExpr(Token* token, std::size_t length,
                                   std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  enum class State { kPreOper, kPostOper, kEnd };
  State state = State::kPreOper;
  ExprNode* full_expr = nullptr;
  ExprNode* main_expr = nullptr;
  ExprNode* preoper_expr = nullptr;

  while (state != State::kEnd && index < length) {
    if (token[index].type == Token::Type::OPERATOR) {
      switch (token[index].value._operator) {
        case Token::OperatorType::amp:  // &
          if (state == State::kPreOper) {
            UnaryNode* amp_node = new UnaryNode();
            amp_node->SetUnaryNode(UnaryNode::Operator::kAddrOf, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = amp_node;
              preoper_expr = amp_node;
            } else {
              if (preoper_expr != nullptr)
                if (preoper_expr != nullptr)
                  dynamic_cast<UnaryNode*>(preoper_expr)
                      ->SetUnaryNode(
                          dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                          amp_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::star:  // *
          if (state == State::kPreOper) {
            UnaryNode* star_node = new UnaryNode();
            star_node->SetUnaryNode(UnaryNode::Operator::kDeref, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = star_node;
              preoper_expr = star_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        star_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::plus:  // +
          if (state == State::kPreOper) {
            UnaryNode* plus_node = new UnaryNode();
            plus_node->SetUnaryNode(UnaryNode::Operator::kPlus, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = plus_node;
              preoper_expr = plus_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        plus_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::minus:  // -
          if (state == State::kPreOper) {
            UnaryNode* minus_node = new UnaryNode();
            minus_node->SetUnaryNode(UnaryNode::Operator::kMinus, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = minus_node;
              preoper_expr = minus_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        minus_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::exclaim:  // !
          if (state == State::kPreOper) {
            UnaryNode* not_node = new UnaryNode();
            not_node->SetUnaryNode(UnaryNode::Operator::kNot, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = not_node;
              preoper_expr = not_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        not_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::tilde:  // ~
          if (state == State::kPreOper) {
            UnaryNode* bitwisenot_node = new UnaryNode();
            bitwisenot_node->SetUnaryNode(UnaryNode::Operator::kBitwiseNot,
                                          nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = bitwisenot_node;
              preoper_expr = bitwisenot_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        bitwisenot_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::l_square:  // [
          if (state == State::kPostOper) {
            index++;
            ArrayNode* array_node = new ArrayNode();
            array_node->SetArrayNode(main_expr,
                                     ParseExpr(token, length, index));
            if (preoper_expr != nullptr)
              dynamic_cast<UnaryNode*>(preoper_expr)
                  ->SetUnaryNode(
                      dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                      array_node);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = main_expr = array_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        array_node);
              main_expr = array_node;
            }
            index++;
            break;
          }
          break;
        case Token::OperatorType::r_square:  // ]
          state = State::kEnd;
          break;
        case Token::OperatorType::l_paren:  // (
          if (state == State::kPreOper) {
            index++;
            if (full_expr == nullptr || preoper_expr == nullptr) {
              if (token[index].type == Token::Type::KEYWORD) {
                full_expr = new ConvertNode();
                dynamic_cast<ConvertNode*>(full_expr)->SetConvertNode(
                    Type::CreateType(token, length, index), nullptr);
              } else {
                full_expr = ParseExpr(token, length, index);
                state = State::kPostOper;
              }
              preoper_expr = full_expr;
            } else {
              if (token[index].type == Token::Type::KEYWORD) {
                ConvertNode* convert_node = new ConvertNode();
                convert_node->SetConvertNode(
                    Type::CreateType(token, length, index), nullptr);
                if (preoper_expr != nullptr)
                  dynamic_cast<UnaryNode*>(preoper_expr)
                      ->SetUnaryNode(
                          dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                          convert_node);
                preoper_expr = convert_node;
              } else {
                ExprNode* full_expr_node = ParseExpr(token, length, index);
                if (preoper_expr != nullptr)
                  dynamic_cast<UnaryNode*>(preoper_expr)
                      ->SetUnaryNode(
                          dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                          full_expr_node);
                state = State::kPostOper;
              }
            }
            index++;
          } else if (state == State::kPostOper &&
                     main_expr->GetType() == StmtNode::StmtType::kIdentifier &&
                     token[index - 1].type == Token::Type::IDENTIFIER) {
            std::vector<ExprNode*> args;
            index++;
            while (index < length && token[index].value._operator !=
                                         Token::OperatorType::r_paren) {
              args.push_back(ParseExpr(token, length, index));
              if (token[index].type == Token::Type::OPERATOR &&
                  token[index].value._operator == Token::OperatorType::comma) {
                index++;
              } else if (token[index].type == Token::Type::OPERATOR &&
                         token[index].value._operator ==
                             Token::OperatorType::r_paren) {
                break;
              } else {
                state = State::kEnd;
                break;
              }
            }
            index++;
            FuncNode* func_node = new FuncNode();
            func_node->SetFuncNode(main_expr, args);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = main_expr = func_node;
            } else {
              if (preoper_expr != nullptr) {
                UnaryNode* unary_node = dynamic_cast<UnaryNode*>(preoper_expr);
                if (unary_node == nullptr) {
                  return nullptr;
                }
                unary_node->SetUnaryNode(unary_node->GetOperator(), func_node);
              }
              main_expr = func_node;
            }
          } else {
            state = State::kEnd;
          }
          break;
        case Token::OperatorType::r_paren:  // )
          state = State::kEnd;
          break;

        case Token::OperatorType::plusplus: {  // ++
          UnaryNode* preinc_node = new UnaryNode();
          if (state == State::kPreOper) {
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              preoper_expr = full_expr = preinc_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        preinc_node);
            }
          } else {
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc, full_expr);
            full_expr = preinc_node;
          }
          index++;
          break;
        }
        case Token::OperatorType::minusminus: {  // --
          UnaryNode* postinc_node = new UnaryNode();
          if (state == State::kPostOper) {
            postinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              preoper_expr = full_expr = postinc_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        postinc_node);
            }
          } else {
            postinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc,
                                       full_expr);
            full_expr = postinc_node;
          }
          index++;
          break;
        }
        // TODO(Parser): Advanced syntax awaits subsequent development.
        /*case Token::OperatorType::l_brace:  // {
          break;
        case Token::OperatorType::r_brace:  // }
          break;
        case Token::OperatorType::period:  // .
          break;
        case Token::OperatorType::arrow:  // ->
          break;
        case Token::OperatorType::question:  // ?
          break;
        case Token::OperatorType::colon:  // :
          break;
        case Token::OperatorType::periodstar:  // .*
          break;
        case Token::OperatorType::arrowstar:  // ->*
          break;
        case Token::OperatorType::coloncolon:  // ::
          break;*/
        default:
          state = State::kEnd;
          break;
      }
    } else if (token[index].type == Token::Type::IDENTIFIER) {
      IdentifierNode* identifier_node = new IdentifierNode();
      identifier_node->SetIdentifierNode(token[index]);
      if (full_expr == nullptr || preoper_expr == nullptr) {
        full_expr = main_expr = identifier_node;
      } else {
        if (preoper_expr != nullptr)
          dynamic_cast<UnaryNode*>(preoper_expr)
              ->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                  identifier_node);
        main_expr = identifier_node;
      }
      if (token[index + 1].type != Token::Type::OPERATOR ||
          (token[index].value._operator != Token::OperatorType::coloncolon &&
           token[index].value._operator != Token::OperatorType::arrow &&
           token[index].value._operator != Token::OperatorType::periodstar &&
           token[index].value._operator != Token::OperatorType::arrowstar))
        state = State::kPostOper;
      index++;
    } else if (token[index].type == Token::Type::NUMBER ||
               token[index].type == Token::Type::CHARACTER ||
               token[index].type == Token::Type::STRING) {
      ValueNode* number_node = new ValueNode();
      number_node->SetValueNode(token[index]);
      if (full_expr == nullptr || preoper_expr == nullptr) {
        full_expr = main_expr = number_node;
      } else {
        dynamic_cast<UnaryNode*>(preoper_expr)
            ->SetUnaryNode(
                dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                number_node);
        main_expr = number_node;
      }
      index++;
      state = State::kEnd;
    } else {
      state = State::kEnd;
    }
  }

  return full_expr;
}

ExprNode* Parser::ParseExpr(Token* token, std::size_t length,
                            std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseExpr(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseExpr(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  if (IsDecl(token, length, index)) return ParseVarDecl(token, length, index);
  ExprNode* expr = ParsePrimaryExpr(token, length, index);
  if (expr == nullptr)
    EXIT_COMPILER("Parser::ParseExpr(Token*,std::size_t,std::size_t&)",
                  "expr is nullptr.");
  expr = ParseBinaryExpr(token, length, index, expr, 0);
  return expr;
}

ExprNode* Parser::ParseBinaryExpr(Token* token, std::size_t length,
                                  std::size_t& index, ExprNode* left,
                                  unsigned int priority) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER(
        "Parser::ParseBinaryExpr(Token*,std::size_t,std::size_t&,ExprNode*,"
        "unsigned int)",
        "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER(
        "Parser::ParseBinaryExpr(Token*,std::size_t,std::size_t&,ExprNode*,"
        "unsigned int)",
        "index is out of range.");
  if (left == nullptr)
    EXIT_COMPILER(
        "Parser::ParseBinaryExpr(Token*,std::size_t,std::size_t&,ExprNode*,"
        "unsigned int)",
        "left is nullptr.");

  ExprNode* expr = left;
  while (index < length && GetPriority(token[index]) > priority) {
    if (token[index].type != Token::Type::OPERATOR)
      EXIT_COMPILER(
          "Parser::ParseBinaryExpr(Token*,std::size_t,std::size_t&,ExprNode*,"
          "unsigned int)",
          "Unexpected code.");
    switch (token[index].value._operator) {
      case Token::OperatorType::periodstar: {
        BinaryNode* periodstar_node = new BinaryNode();
        index++;
        periodstar_node->SetBinaryNode(
            BinaryNode::Operator::kPtrMemD, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 14));
        expr = periodstar_node;
        break;
      }
      case Token::OperatorType::arrowstar: {
        BinaryNode* arrowstar_node = new BinaryNode();
        index++;
        arrowstar_node->SetBinaryNode(
            BinaryNode::Operator::kPtrMemI, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 14));
        expr = arrowstar_node;
        break;
      }

      case Token::OperatorType::star: {
        BinaryNode* star_node = new BinaryNode();
        index++;
        star_node->SetBinaryNode(
            BinaryNode::Operator::kMul, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 13));
        expr = star_node;
        break;
      }
      case Token::OperatorType::slash: {
        BinaryNode* slash_node = new BinaryNode();
        index++;
        slash_node->SetBinaryNode(
            BinaryNode::Operator::kDiv, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 13));
        expr = slash_node;
        break;
      }
      case Token::OperatorType::percent: {
        BinaryNode* percent_node = new BinaryNode();
        index++;
        percent_node->SetBinaryNode(
            BinaryNode::Operator::kRem, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 13));
        expr = percent_node;
        break;
      }

      case Token::OperatorType::plus: {
        BinaryNode* plus_node = new BinaryNode();
        index++;
        plus_node->SetBinaryNode(
            BinaryNode::Operator::kAdd, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 12));
        expr = plus_node;
        break;
      }
      case Token::OperatorType::minus: {
        BinaryNode* minus_node = new BinaryNode();
        index++;
        minus_node->SetBinaryNode(
            BinaryNode::Operator::kSub, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 12));
        expr = minus_node;
        break;
      }

      case Token::OperatorType::lessless: {
        BinaryNode* lessless_node = new BinaryNode();
        index++;
        lessless_node->SetBinaryNode(
            BinaryNode::Operator::kShl, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 11));
        expr = lessless_node;
        break;
      }
      case Token::OperatorType::greatergreater: {
        BinaryNode* greatergreater_node = new BinaryNode();
        index++;
        greatergreater_node->SetBinaryNode(
            BinaryNode::Operator::kShr, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 11));
        expr = greatergreater_node;
        break;
      }

      case Token::OperatorType::less: {
        BinaryNode* less_node = new BinaryNode();
        index++;
        less_node->SetBinaryNode(
            BinaryNode::Operator::kLT, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 10));
        expr = less_node;
        break;
      }
      case Token::OperatorType::lessequal: {
        BinaryNode* lessequal_node = new BinaryNode();
        index++;
        lessequal_node->SetBinaryNode(
            BinaryNode::Operator::kLE, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 10));
        expr = lessequal_node;
        break;
      }
      case Token::OperatorType::greater: {
        BinaryNode* greater_node = new BinaryNode();
        index++;
        greater_node->SetBinaryNode(
            BinaryNode::Operator::kGT, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 10));
        expr = greater_node;
        break;
      }
      case Token::OperatorType::greaterequal: {
        BinaryNode* greaterequal_node = new BinaryNode();
        index++;
        greaterequal_node->SetBinaryNode(
            BinaryNode::Operator::kGE, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 10));
        expr = greaterequal_node;
        break;
      }

      case Token::OperatorType::equalequal: {
        BinaryNode* equalequal_node = new BinaryNode();
        index++;
        equalequal_node->SetBinaryNode(
            BinaryNode::Operator::kEQ, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 9));
        expr = equalequal_node;
        break;
      }
      case Token::OperatorType::exclaimequal: {
        BinaryNode* exclaimequal_node = new BinaryNode();
        index++;
        exclaimequal_node->SetBinaryNode(
            BinaryNode::Operator::kNE, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 9));
        expr = exclaimequal_node;
        break;
      }

      case Token::OperatorType::amp: {
        BinaryNode* amp_node = new BinaryNode();
        index++;
        amp_node->SetBinaryNode(
            BinaryNode::Operator::kAnd, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 8));
        expr = amp_node;
        break;
      }

      case Token::OperatorType::caret: {
        BinaryNode* caret_node = new BinaryNode();
        index++;
        caret_node->SetBinaryNode(
            BinaryNode::Operator::kXor, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 7));
        expr = caret_node;
        break;
      }

      case Token::OperatorType::pipe: {
        BinaryNode* pipe_node = new BinaryNode();
        index++;
        pipe_node->SetBinaryNode(
            BinaryNode::Operator::kOr, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 6));
        expr = pipe_node;
        break;
      }

      case Token::OperatorType::ampamp: {
        BinaryNode* ampamp_node = new BinaryNode();
        index++;
        ampamp_node->SetBinaryNode(
            BinaryNode::Operator::kLAnd, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 5));
        expr = ampamp_node;
        break;
      }

      case Token::OperatorType::pipepipe: {
        BinaryNode* pipepipe_node = new BinaryNode();
        index++;
        pipepipe_node->SetBinaryNode(
            BinaryNode::Operator::kLOr, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 4));
        expr = pipepipe_node;
        break;
      }

      case Token::OperatorType::question:
        // TODO(TriExpr): Complete the case.
        break;

      case Token::OperatorType::equal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::plusequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kAddAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::minusequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kSubAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::starequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kMulAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::slashequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kDivAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::percentequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kRemAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::ampequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kAndAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::caretequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kXorAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::pipeequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kOrAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::lesslessequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kShlAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }
      case Token::OperatorType::greatergreaterequal: {
        BinaryNode* equal_node = new BinaryNode();
        index++;
        equal_node->SetBinaryNode(
            BinaryNode::Operator::kShrAssign, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 2));
        expr = equal_node;
        break;
      }

      case Token::OperatorType::comma: {
        BinaryNode* comma_node = new BinaryNode();
        index++;
        comma_node->SetBinaryNode(
            BinaryNode::Operator::kComma, expr,
            ParseBinaryExpr(token, length, index,
                            ParsePrimaryExpr(token, length, index), 1));
        expr = comma_node;
        break;
      }

      default:
        return expr;
    }
  }

  return expr;
}

unsigned int Parser::GetPriority(Token token) {
  TRACE_FUNCTION;
  if (token.type == Token::Type::OPERATOR) {
    switch (token.value._operator) {
      case Token::OperatorType::periodstar:
      case Token::OperatorType::arrowstar:
        return 14;
      case Token::OperatorType::star:
      case Token::OperatorType::slash:
      case Token::OperatorType::percent:
        return 13;
      case Token::OperatorType::plus:
      case Token::OperatorType::minus:
        return 12;
      case Token::OperatorType::lessless:
      case Token::OperatorType::greatergreater:
        return 11;
      case Token::OperatorType::less:
      case Token::OperatorType::lessequal:
      case Token::OperatorType::greater:
      case Token::OperatorType::greaterequal:
        return 10;
      case Token::OperatorType::equalequal:
      case Token::OperatorType::exclaimequal:
        return 9;
      case Token::OperatorType::amp:
        return 8;
      case Token::OperatorType::caret:
        return 7;
      case Token::OperatorType::pipe:
        return 6;
      case Token::OperatorType::ampamp:
        return 5;
      case Token::OperatorType::pipepipe:
        return 4;
      case Token::OperatorType::question:
        return 3;
      case Token::OperatorType::equal:
      case Token::OperatorType::plusequal:
      case Token::OperatorType::minusequal:
      case Token::OperatorType::starequal:
      case Token::OperatorType::slashequal:
      case Token::OperatorType::percentequal:
      case Token::OperatorType::ampequal:
      case Token::OperatorType::caretequal:
      case Token::OperatorType::pipeequal:
      case Token::OperatorType::lesslessequal:
      case Token::OperatorType::greatergreaterequal:
        return 2;
      case Token::OperatorType::comma:
        return 1;
      default:
        return 0;
    }
  }
  EXIT_COMPILER("Parser::GetPriority(Token)", "Unexpected code.");
  return 0;
}

class BytecodeGenerator {
 public:
  BytecodeGenerator();
  ~BytecodeGenerator() = default;

  void GenerateBytecode(CompoundNode* stmt);

 private:
  class Memory {
   public:
    Memory() {
      TRACE_FUNCTION;
      IsBigEndian();
    }
    ~Memory() = default;

    std::size_t Add(uint8_t type, std::size_t size) {
      TRACE_FUNCTION;
      std::size_t index = memory_.size();

      if (type > 0x0F)
        EXIT_COMPILER("BytecodeGenerator::Memory::Add(uint8_t,std::size_t)",
                      "Unexpected type.");

      for (std::size_t i = 0; i < size; i++) {
        if (memory_.size() % 2 != 0) {
          memory_.push_back(0);
          type_[type_.size() - 1] = (type_[type_.size() - 1]) | type;
        } else {
          memory_.push_back(0);
          type_.push_back(type << 4);
        }
      }

      return index;
    }

    std::size_t Add(uint8_t type, std::size_t size, const void* data) {
      TRACE_FUNCTION;
      std::size_t index = memory_.size();

      if (type > 0x0F)
        EXIT_COMPILER(
            "BytecodeGenerator::Memory::Add(uint8_t,std::size_t,const void*)",
            "Unexpected type.");

      void* memory_data = malloc(size);
      if (memory_data == nullptr)
        EXIT_COMPILER(
            "BytecodeGenerator::Memory::Add(uint8_t,std::size_t,const void*)",
            "malloc failed.");
      std::memcpy(memory_data, data, size);

      for (std::size_t i = 0; i < size; i++) {
        if (memory_.size() % 2 != 0) {
          memory_.push_back(*(uint8_t*)memory_data);
          memory_data = (void*)((uintptr_t)memory_data + 1);
          type_[type_.size() - 1] = (type_[type_.size() - 1]) | type;
        } else {
          memory_.push_back(*(uint8_t*)memory_data);
          memory_data = (void*)((uintptr_t)memory_data + 1);
          type_.push_back(type << 4);
        }
      }
      return index;
    }

    void Set(std::size_t index, const void* data, std::size_t size) {
      TRACE_FUNCTION;

      void* memory_data = malloc(size);
      if (memory_data == nullptr)
        EXIT_COMPILER(
            "BytecodeGenerator::Memory::Set(std::size_t,const "
            "void*,std::size_t)",
            "malloc failed.");
      std::memcpy(memory_data, data, size);

      for (std::size_t i = 0; i < size; i++) {
        memory_.push_back(*(uint8_t*)memory_data);
        memory_data = (void*)((uintptr_t)memory_data + 1);
      }
    }

    size_t GetSize() {
      TRACE_FUNCTION;
      return memory_.size();
    }
    std::vector<uint8_t> GetMemory() {
      TRACE_FUNCTION;
      return memory_;
    }
    std::vector<uint8_t> GetType() {
      TRACE_FUNCTION;
      return type_;
    }

   private:
    std::vector<uint8_t> memory_;
    std::vector<uint8_t> type_;
    bool is_big_endian_ = false;

    void IsBigEndian() {
      TRACE_FUNCTION;
      uint16_t test_data = 0x0011;
      is_big_endian_ = *(uint8_t*)&test_data == 0x00;
    }

    int SwapInt(int x) {
      TRACE_FUNCTION;
      uint32_t ux = (uint32_t)x;
      ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
           ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
      return (int)ux;
    }

    long SwapLong(long x) {
      TRACE_FUNCTION;
      uint64_t ux = (uint64_t)x;
      ux = ((ux << 56) & 0xFF00000000000000ULL) |
           ((ux << 40) & 0x00FF000000000000ULL) |
           ((ux << 24) & 0x0000FF0000000000ULL) |
           ((ux << 8) & 0x000000FF00000000ULL) |
           ((ux >> 8) & 0x00000000FF000000ULL) |
           ((ux >> 24) & 0x0000000000FF0000ULL) |
           ((ux >> 40) & 0x000000000000FF00ULL) |
           ((ux >> 56) & 0x00000000000000FFULL);
      return (long)ux;
    }

    float SwapFloat(float x) {
      TRACE_FUNCTION;
      uint32_t ux;
      memcpy(&ux, &x, sizeof(uint32_t));
      ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
           ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
      float result;
      memcpy(&result, &ux, sizeof(float));
      return result;
    }

    double SwapDouble(double x) {
      TRACE_FUNCTION;
      uint64_t ux;
      memcpy(&ux, &x, sizeof(uint64_t));
      ux = ((ux << 56) & 0xFF00000000000000ULL) |
           ((ux << 40) & 0x00FF000000000000ULL) |
           ((ux << 24) & 0x0000FF0000000000ULL) |
           ((ux << 8) & 0x000000FF00000000ULL) |
           ((ux >> 8) & 0x00000000FF000000ULL) |
           ((ux >> 24) & 0x0000000000FF0000ULL) |
           ((ux >> 40) & 0x000000000000FF00ULL) |
           ((ux >> 56) & 0x00000000000000FFULL);
      double result;
      memcpy(&result, &ux, sizeof(double));
      return result;
    }

    uint64_t SwapUint64t(uint64_t x) {
      TRACE_FUNCTION;
      x = ((x << 56) & 0xFF00000000000000ULL) |
          ((x << 40) & 0x00FF000000000000ULL) |
          ((x << 24) & 0x0000FF0000000000ULL) |
          ((x << 8) & 0x000000FF00000000ULL) |
          ((x >> 8) & 0x00000000FF000000ULL) |
          ((x >> 24) & 0x0000000000FF0000ULL) |
          ((x >> 40) & 0x000000000000FF00ULL) |
          ((x >> 56) & 0x00000000000000FFULL);
      return x;
    }
  };

  class Bytecode {
   public:
    // Bytecode(std::size_t oper) { oper_ = oper; }
    Bytecode(std::size_t oper, size_t args_count, ...) {
      TRACE_FUNCTION;
      oper_ = oper;
      va_list args;
      va_start(args, args_count);
      for (std::size_t i = 0; i < args_count; i++) {
        arg_.push_back(va_arg(args, std::size_t));
        if (oper == _AQVM_OPERATOR_CMP && i == 1) {
          std::cout << "CMP OPCODE: " << arg_[1] << std::endl;
        }
      }
      va_end(args);
    }
    Bytecode(std::size_t oper, std::vector<std::size_t> args) {
      TRACE_FUNCTION;
      oper_ = oper;
      arg_ = args;
    }
    ~Bytecode() = default;

    uint8_t GetOper() {
      TRACE_FUNCTION;
      return oper_;
    }
    std::vector<std::size_t> GetArgs() {
      TRACE_FUNCTION;
      return arg_;
    }

    void SetOper(uint8_t oper) {
      TRACE_FUNCTION;
      oper_ = oper;
    }
    void SetArgs(std::vector<std::size_t> args) {
      TRACE_FUNCTION;
      arg_ = args;
    }

   private:
    uint8_t oper_;
    std::vector<std::size_t> arg_;
  };

  class Function {
   public:
    Function(std::string name, std::vector<Bytecode> code) {
      TRACE_FUNCTION;
      name_ = name;
      code_ = code;
    }
    ~Function() = default;

    std::string GetName() {
      TRACE_FUNCTION;
      return name_;
    }

    std::vector<Bytecode> GetCode() {
      TRACE_FUNCTION;
      return code_;
    }

   private:
    std::string name_;
    std::vector<Bytecode> code_;
  };

  void HandleFuncDecl(FuncDeclNode* func_decl);
  void HandleVarDecl(VarDeclNode* var_decl, std::vector<Bytecode>& code);
  void HandleArrayDecl(ArrayDeclNode* array_decl, std::vector<Bytecode>& code);
  void HandleStmt(StmtNode* stmt, std::vector<Bytecode>& code);
  void HandleCompoundStmt(CompoundNode* stmt, std::vector<Bytecode>& code);
  void HandleIfStmt(IfNode* stmt, std::vector<Bytecode>& code);
  void HandleWhileStmt(WhileNode* stmt, std::vector<Bytecode>& code);
  std::size_t HandleExpr(ExprNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleUnaryExpr(UnaryNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleBinaryExpr(BinaryNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleFuncInvoke(FuncNode* func, std::vector<Bytecode>& code);
  std::size_t GetIndex(ExprNode* expr, std::vector<Bytecode>& code);
  std::size_t AddConstInt8t(int8_t value);
  uint8_t GetExprVmType(ExprNode* expr);
  uint8_t GetExprPtrValueVmType(ExprNode* expr);
  std::size_t GetExprVmSize(uint8_t type);
  int SwapInt(int x);
  long SwapLong(long x);
  float SwapFloat(float x);
  double SwapDouble(double x);
  uint64_t SwapUint64t(uint64_t x);
  void InsertUint64ToCode(uint64_t value);
  size_t EncodeUleb128(size_t value, std::vector<uint8_t>& output);
  void GenerateBytecodeFile();
  void GenerateMnemonicFile();

  bool is_big_endian_;
  std::unordered_map<std::string, FuncDeclNode> func_decl_map;
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
      var_decl_map;
  std::vector<Function> func_list_;
  Memory global_memory_;
  std::vector<Bytecode> global_code_;
  std::vector<uint8_t> code_;
};

BytecodeGenerator::BytecodeGenerator() {
  TRACE_FUNCTION;
  uint16_t test_data = 0x0011;
  is_big_endian_ = *(uint8_t*)&test_data == 0x00;
}

void BytecodeGenerator::GenerateBytecode(CompoundNode* stmt) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "stmt is nullptr.");

  for (std::size_t i = 0; i < stmt->GetStmts().size(); i++) {
    switch (stmt->GetStmts()[i]->GetType()) {
      case StmtNode::StmtType::kFuncDecl:
        HandleFuncDecl(dynamic_cast<FuncDeclNode*>(stmt->GetStmts()[i]));
        break;

      case StmtNode::StmtType::kVarDecl:
        HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt->GetStmts()[i]),
                      global_code_);
        break;

      case StmtNode::StmtType::kArrayDecl:
        HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt->GetStmts()[i]),
                        global_code_);
        break;

      default:
        EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                      "Unexpected code.");
    }
  }

  GenerateBytecodeFile();
}

void BytecodeGenerator::GenerateBytecodeFile() {
  TRACE_FUNCTION;

  code_.push_back(0x41);
  code_.push_back(0x51);
  code_.push_back(0x42);
  code_.push_back(0x43);

  // Version
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x01);

  size_t memory_size = global_memory_.GetSize();
  InsertUint64ToCode(is_big_endian_ ? memory_size : SwapUint64t(memory_size));
  for (size_t i = 0; i < global_memory_.GetMemory().size(); i++) {
    code_.push_back(global_memory_.GetMemory()[i]);
  }
  for (size_t i = 0; i < global_memory_.GetType().size(); i++) {
    code_.push_back(global_memory_.GetType()[i]);
  }

  for (size_t i = 0; i < func_list_.size(); i++) {
    // Function name (with '\0')
    std::cout << "function name size: " << func_list_[i].GetName().size()
              << std::endl;
    std::cout << "code size: " << func_list_[i].GetCode().size() << std::endl;
    std::string func_name_str = func_list_[i].GetName();
    const char* func_name = func_name_str.c_str();
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                 reinterpret_cast<const uint8_t*>(
                     func_name + func_list_[i].GetName().size() + 1));

    uint64_t value = is_big_endian_
                         ? func_list_[i].GetCode().size()
                         : SwapUint64t(func_list_[i].GetCode().size());
    code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                 reinterpret_cast<uint8_t*>(&value) + 8);

    for (size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
      std::vector<uint8_t> buffer;
      switch (func_list_[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          code_.push_back(_AQVM_OPERATOR_NOP);
          break;

        case _AQVM_OPERATOR_LOAD:
          code_.push_back(_AQVM_OPERATOR_LOAD);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_STORE:
          code_.push_back(_AQVM_OPERATOR_STORE);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected STORE args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEW:
          code_.push_back(_AQVM_OPERATOR_NEW);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEW args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_FREE:
          code_.push_back(_AQVM_OPERATOR_FREE);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected FREE args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_PTR:

          code_.push_back(_AQVM_OPERATOR_PTR);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected PTR args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ADD:

          code_.push_back(_AQVM_OPERATOR_ADD);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected ADD args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SUB:

          code_.push_back(_AQVM_OPERATOR_SUB);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SUB args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_MUL:

          code_.push_back(_AQVM_OPERATOR_MUL);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected MUL args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_DIV:

          code_.push_back(_AQVM_OPERATOR_DIV);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected DIV args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REM:

          code_.push_back(_AQVM_OPERATOR_REM);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected REM args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEG:

          code_.push_back(_AQVM_OPERATOR_NEG);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEG args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHL:

          code_.push_back(_AQVM_OPERATOR_SHL);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHL args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHR:

          code_.push_back(_AQVM_OPERATOR_SHR);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHR args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SAR:

          code_.push_back(_AQVM_OPERATOR_SAR);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SAR args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_IF:

          code_.push_back(_AQVM_OPERATOR_IF);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected IF args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_AND:

          code_.push_back(_AQVM_OPERATOR_AND);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected AND args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_OR:

          code_.push_back(_AQVM_OPERATOR_OR);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected OR args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_XOR:

          code_.push_back(_AQVM_OPERATOR_XOR);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected XOR args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CMP:

          code_.push_back(_AQVM_OPERATOR_CMP);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CMP args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[3], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE:

          code_.push_back(_AQVM_OPERATOR_INVOKE);

          if (func_list_[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected INVOKE args size.");

          for (size_t k = 0; k != func_list_[i].GetCode()[j].GetArgs()[1] + 2;
               k++) {
            EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_EQUAL:
          code_.push_back(_AQVM_OPERATOR_EQUAL);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected EQUAL args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_GOTO:

          code_.push_back(_AQVM_OPERATOR_GOTO);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected GOTO args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_THROW:

          code_.push_back(_AQVM_OPERATOR_THROW);
          break;

        case _AQVM_OPERATOR_WIDE:

          code_.push_back(_AQVM_OPERATOR_WIDE);
          break;

        default:
          break;
      }
    }
  }

  std::string filename = "a.out";
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile) {
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Can't open file.");
    return;
  }

  outFile.write(reinterpret_cast<const char*>(code_.data()), code_.size());
  outFile.close();

  if (!outFile) {
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Failed to write file.");
  } else {
    std::cout << "Write file success: " << filename << std::endl;
  }

  bool is_output_mnemonic = true;
  if (is_output_mnemonic) {
    GenerateMnemonicFile();
  }
}

void BytecodeGenerator::GenerateMnemonicFile() {
  TRACE_FUNCTION;

  std::ofstream output_file("mnemonic.txt");
  if (!output_file) {
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Can't open file.");
  }

  std::streambuf* cout_buffer = std::cout.rdbuf();
  std::cout.rdbuf(output_file.rdbuf());

  size_t memory_size = global_memory_.GetSize();
  std::cout << "Memory Size: " << memory_size << std::endl;
  std::cout << std::endl << std::endl << std::endl;

  for (size_t i = 0; i < func_list_.size(); i++) {
    std::cout << "Function Name: " << func_list_[i].GetName()
              << ", Size: " << func_list_[i].GetCode().size() << std::endl;

    for (size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
      switch (func_list_[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:

          std::cout << "NOP" << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD args size.");
          std::cout << "LOAD: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_STORE:

          std::cout << "STORE: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected STORE args size.");
          break;

        case _AQVM_OPERATOR_NEW:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEW args size.");
          std::cout << "NEW: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_FREE:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected FREE args size.");
          break;

        case _AQVM_OPERATOR_PTR:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected PTR args size.");
          std::cout << "PTR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_ADD:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected ADD args size.");
          std::cout << "ADD: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SUB:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SUB args size.");
          std::cout << "SUB: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_MUL:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected MUL args size.");
          std::cout << "MUL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_DIV:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected DIV args size.");
          std::cout << "DIV: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_REM:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected REM args size.");
          std::cout << "REM: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_NEG:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEG args size.");
          std::cout << "NEG: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_SHL:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHL args size.");
          std::cout << "SHL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SHR:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHR args size.");
          std::cout << "SHR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SAR:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SAR args size.");
          std::cout << "SAR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_IF:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected IF args size.");
          std::cout << "IF: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_AND:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected AND args size.");
          std::cout << "AND: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_OR:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected OR args size.");
          std::cout << "OR: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_XOR:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected XOR args size.");
          std::cout << "XOR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_CMP:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CMP args size.");
          std::cout << "CMP: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[3] << std::endl;
          break;

        case _AQVM_OPERATOR_INVOKE:

          if (func_list_[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected INVOKE args size.");
          std::cout << "INVOKE: ";
          for (size_t k = 0; k != func_list_[i].GetCode()[j].GetArgs()[1] + 2;
               k++) {
            std::cout << func_list_[i].GetCode()[j].GetArgs()[k] << " ,";
          }
          std::cout << std::endl;
          break;

        case _AQVM_OPERATOR_EQUAL:
          std::cout << "EQUAL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_GOTO:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected GOTO args size.");
          std::cout << "GOTO: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_THROW:

          std::cout << "THROW" << std::endl;
          break;

        case _AQVM_OPERATOR_WIDE:

          std::cout << "WIDE" << std::endl;
          break;

        default:
          break;
      }
    }
    std::cout << std::endl << std::endl << std::endl;
  }

  std::cout.rdbuf(cout_buffer);
  output_file.close();
}

void BytecodeGenerator::HandleFuncDecl(FuncDeclNode* func_decl) {
  TRACE_FUNCTION;
  if (func_decl == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                  "func_decl is nullptr.");

  std::vector<Bytecode> code;
  func_decl_map.emplace(*func_decl->GetStat()->GetName(), *func_decl);
  HandleStmt(func_decl->GetStmts(), code);
  Function func_decl_bytecode(*func_decl->GetStat()->GetName(), code);
  func_list_.push_back(func_decl_bytecode);
}

void BytecodeGenerator::HandleVarDecl(VarDeclNode* var_decl,
                                      std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
        "var_decl is nullptr.");

  Type* var_type = var_decl->GetVarType();
  if (var_type == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
        "var_type is nullptr.");

  while (var_type->GetType() != Type::TypeType::kBase &&
         var_type->GetType() != Type::TypeType::kPointer &&
         var_type->GetType() != Type::TypeType::kArray &&
         var_type->GetType() != Type::TypeType::kReference) {
    if (var_type->GetType() == Type::TypeType::NONE)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&"
          ")",
          "Unexpected code.");
    if (var_type->GetType() == Type::TypeType::kConst)
      var_type = dynamic_cast<ConstType*>(var_type)->GetSubType();
  }

  uint8_t vm_type = 0x00;
  if (var_type->GetType() == Type::TypeType::kBase) {
    switch (var_type->GetBaseType()) {
      case Type::BaseType::kVoid:
        vm_type = 0x00;
        break;
      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        vm_type = 0x01;
        break;
      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
        vm_type = 0x02;
        break;
      case Type::BaseType::kLong:
        vm_type = 0x03;
        break;
      case Type::BaseType::kFloat:
        vm_type = 0x04;
        break;
      case Type::BaseType::kDouble:
        vm_type = 0x05;
        break;
      case Type::BaseType::kStruct:
      case Type::BaseType::kUnion:
      case Type::BaseType::kEnum:
      case Type::BaseType::kPointer:
      case Type::BaseType::kArray:
      case Type::BaseType::kFunction:
      case Type::BaseType::kTypedef:
      case Type::BaseType::kAuto:
        vm_type = 0x06;
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }
  } else if (var_type->GetType() == Type::TypeType::kPointer ||
             var_type->GetType() == Type::TypeType::kReference) {
    vm_type = 0x06;
  }
  if (var_decl->GetValue()[0] == nullptr) {
    std::cout << "None Value" << std::endl;
    var_decl_map.emplace(
        *var_decl->GetName(),
        std::pair<VarDeclNode*, std::size_t>(
            var_decl, global_memory_.Add(vm_type, var_type->GetSize())));
  } else {
    std::cout << "Has Value" << std::endl;
    size_t var_index = global_memory_.Add(vm_type, var_type->GetSize());
    size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    var_decl_map.emplace(
        *var_decl->GetName(),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
  }
}

void BytecodeGenerator::HandleArrayDecl(ArrayDeclNode* array_decl,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_type->GetType() == Type::TypeType::kConst)
    array_type = dynamic_cast<ConstType*>(array_type)->GetSubType();
  uint8_t vm_type = 0x00;
  if (array_type->GetType() == Type::TypeType::kBase) {
    switch (array_type->GetBaseType()) {
      case Type::BaseType::kVoid:
        vm_type = 0x00;
        break;
      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        vm_type = 0x01;
        break;
      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
        vm_type = 0x02;
        break;
      case Type::BaseType::kLong:
        vm_type = 0x03;
        break;
      case Type::BaseType::kFloat:
        vm_type = 0x04;
        break;
      case Type::BaseType::kDouble:
        vm_type = 0x05;
        break;
      case Type::BaseType::kStruct:
      case Type::BaseType::kUnion:
      case Type::BaseType::kEnum:
      case Type::BaseType::kPointer:
      case Type::BaseType::kArray:
      case Type::BaseType::kFunction:
      case Type::BaseType::kTypedef:
      case Type::BaseType::kAuto:
        vm_type = 0x06;
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }
  } else if (array_type->GetType() == Type::TypeType::kPointer ||
             array_type->GetType() == Type::TypeType::kArray ||
             array_type->GetType() == Type::TypeType::kReference) {
    vm_type = 0x06;
  }
  if (array_decl->GetValue().empty()) {
    size_t array_index = global_memory_.Add(vm_type, array_type->GetSize());
    size_t array_ptr_index = global_memory_.Add(0x06, 8);
    code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));

    var_decl_map.emplace(
        *array_decl->GetName(),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_ptr_index));
  } else {
    size_t array_index = global_memory_.Add(vm_type, array_type->GetSize());
    size_t array_ptr_index = global_memory_.Add(0x06, 8);
    code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));

    for (size_t i = 0; i < array_decl->GetValue().size(); i++) {
      size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, array_index, value_index));
    }

    var_decl_map.emplace(
        *array_decl->GetName(),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_ptr_index));
  }
}

std::size_t BytecodeGenerator::HandleExpr(ExprNode* expr,
                                          std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleExpr(ExprNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    return HandleUnaryExpr(dynamic_cast<UnaryNode*>(expr), code);
  } else if (expr->GetType() == StmtNode::StmtType::kBinary) {
    return HandleBinaryExpr(dynamic_cast<BinaryNode*>(expr), code);
  }

  return GetIndex(expr, code);
}

std::size_t BytecodeGenerator::HandleUnaryExpr(UnaryNode* expr,
                                               std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleUnaryExpr(UnaryNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  std::size_t sub_expr = HandleExpr(expr->GetExpr(), code);
  switch (expr->GetOperator()) {
    case UnaryNode::Operator::kPostInc: {  // ++ (postfix)
      uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index =
          global_memory_.Add(vm_type, GetExprVmSize(vm_type));

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      return new_index;
    }
    case UnaryNode::Operator::kPostDec: {  // -- (postfix)
      uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index =
          global_memory_.Add(vm_type, GetExprVmSize(vm_type));

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      return new_index;
    }
    case UnaryNode::Operator::kPreInc:  // ++ (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      return sub_expr;
    case UnaryNode::Operator::kPreDec:  // -- (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      return sub_expr;
    case UnaryNode::Operator::kAddrOf: {  // & (address of)
      std::size_t ptr_index = global_memory_.Add(0x06, 8);

      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, sub_expr, ptr_index));
      return ptr_index;
    }
    case UnaryNode::Operator::kDeref: {  // * (dereference)
      uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index =
          global_memory_.Add(vm_type, GetExprVmSize(vm_type));

      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD, 2, sub_expr, new_index));
      return new_index;
    }
    case UnaryNode::Operator::kPlus:  // + (unary plus)
      return sub_expr;
    case UnaryNode::Operator::kMinus: {  // - (unary minus)
      uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index =
          global_memory_.Add(vm_type, GetExprVmSize(vm_type));

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }
    case UnaryNode::Operator::kNot: {  // ! (logical NOT)
      uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index =
          global_memory_.Add(vm_type, GetExprVmSize(vm_type));

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }
    case UnaryNode::Operator::kBitwiseNot:  // ~ (bitwise NOT)
                                            // TODO
    default:
      return sub_expr;
  }
}
std::size_t BytecodeGenerator::HandleBinaryExpr(BinaryNode* expr,
                                                std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<Bytecode>&"
        ")",
        "expr is nullptr.");

  ExprNode* left_expr = expr->GetLeftExpr();
  ExprNode* right_expr = expr->GetRightExpr();
  std::size_t right = HandleExpr(right_expr, code);
  std::size_t left = HandleExpr(left_expr, code);
  uint8_t left_type = GetExprVmType(left_expr);
  uint8_t right_type = GetExprVmType(right_expr);
  uint8_t result_type = left_type > right_type ? left_type : right_type;

  switch (expr->GetOperator()) {
    case BinaryNode::Operator::kAdd: {  // +
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kSub: {  // -
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kMul: {  // *
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kDiv: {  // /
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kRem: {  // %
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAnd: {  // &
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kOr: {  // |
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kXor: {  // ^
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShl: {  // <<
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShr: {  // >>
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLT: {  // <
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x04, left, right));
      return result;
    }
    case BinaryNode::Operator::kGT: {  // >
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x02, left, right));
      return result;
    }
    case BinaryNode::Operator::kLE: {  // <=
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x05, left, right));
      return result;
    }
    case BinaryNode::Operator::kGE: {  // >=
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x03, left, right));
      return result;
    }
    case BinaryNode::Operator::kEQ: {  // ==
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x00, left, right));
      return result;
    }
    case BinaryNode::Operator::kNE: {  // !=
      std::size_t result = global_memory_.Add(0x01, 1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CMP, 4, result, (size_t)0x01, left, right));
      return result;
    }
    case BinaryNode::Operator::kLAnd: {  // &&
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLOr: {  // ||
      std::size_t result =
          global_memory_.Add(result_type, GetExprVmSize(result_type));
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAssign:  // =
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, left, right));
      return left;
    case BinaryNode::Operator::kAddAssign:  // +=
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kSubAssign:  // -=
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kMulAssign:  // *=
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kDivAssign:  // /=
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kRemAssign:  // %=
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kAndAssign:  // &=
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kOrAssign:  // |=
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kXorAssign:  // ^=
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kShlAssign:  // <<=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kShrAssign:  // >>=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, left, left, right));
      return left;
    case BinaryNode::Operator::kComma:    // :
    case BinaryNode::Operator::kPtrMemD:  // .*
    case BinaryNode::Operator::kPtrMemI:  // ->*
    default:
      // TODO
      EXIT_COMPILER(
          "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unexpected code.");
      break;
  }
  EXIT_COMPILER(
      "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<Bytecode>&)",
      "Unexpected code.");
  return 0;
}

void BytecodeGenerator::HandleStmt(StmtNode* stmt,
                                   std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStmt(StmtNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  switch (stmt->GetType()) {
    case StmtNode::StmtType::kCompound:
      HandleCompoundStmt(dynamic_cast<CompoundNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kExpr:
      HandleExpr(dynamic_cast<ExprNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kUnary:
      HandleUnaryExpr(dynamic_cast<UnaryNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kBinary:
      HandleBinaryExpr(dynamic_cast<BinaryNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kIf:
      HandleIfStmt(dynamic_cast<IfNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kWhile:
      HandleWhileStmt(dynamic_cast<WhileNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kFuncDecl:
      HandleFuncDecl(dynamic_cast<FuncDeclNode*>(stmt));
      break;

    case StmtNode::StmtType::kVarDecl:
      HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kArrayDecl:
      HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kFunc:
      HandleFuncInvoke(dynamic_cast<FuncNode*>(stmt), code);
      break;

    default:
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStmt(StmtNode*,std::vector<Bytecode>&)",
          "Unexpected code.");
      break;
  }
}

void BytecodeGenerator::HandleCompoundStmt(CompoundNode* stmt,
                                           std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleCompoundStmt(CompoundNode*,std::vector<"
        "Bytecode>&)",
        "stmt is nullptr.");

  for (std::size_t i = 0; i < stmt->GetStmts().size(); i++) {
    HandleStmt(stmt->GetStmts()[i], code);
  }
}

void BytecodeGenerator::HandleIfStmt(IfNode* stmt,
                                     std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleIfStmt(IfNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  size_t if_location = code.size();

  // Need true branch and false branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  size_t true_location = code.size();
  HandleStmt(stmt->GetBody(), code);

  size_t goto_location = code.size();
  // Need exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  size_t false_location = code.size();
  if (stmt->GetElseBody() != nullptr) {
    HandleStmt(stmt->GetElseBody(), code);
  }
  size_t exit_branch = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<size_t> if_args;
  std::vector<size_t> goto_args;
  if_args.push_back(condition_index);
  if_args.push_back(true_location);
  if_args.push_back(false_location);
  code[if_location].SetArgs(if_args);

  goto_args.push_back(exit_branch);
  code[goto_location].SetArgs(goto_args);

  /*  // true branch name
    std::string true_name("$" + std::to_string(++undefined_name_count_));
    std::vector<Bytecode> true_code;
    HandleStmt(stmt->GetBody(), true_code);
    func_list_.push_back(Function(true_name, true_code));

    // true branch ptr
    size_t true_name_index =
        global_memory_.Add(0x01, true_name.size() + 1, true_name.c_str());
    size_t true_name_ptr_index = global_memory_.Add(0x06, 8);
    code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, true_name_index, true_name_ptr_index));

    if (stmt->GetElseBody() != nullptr) {
      // have else branch
      std::string false_name("$" + std::to_string(++undefined_name_count_));
      std::vector<Bytecode> false_code;
      HandleStmt(stmt->GetElseBody(), false_code);
      func_list_.push_back(Function(false_name, false_code));

      size_t false_name_index =
          global_memory_.Add(0x01, false_name.size() + 1, false_name.c_str());
      size_t false_name_ptr_index = global_memory_.Add(0x06, 8);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, false_name_index, false_name_ptr_index));

      code.push_back(Bytecode(_AQVM_OPERATOR_IF, condition_index,
                              true_name_ptr_index, false_name_ptr_index));
    } else {
      // no else branch (void branch)
      size_t false_name_index = global_memory_.Add(0x01, 3, "$0");
      size_t false_name_ptr_index = global_memory_.Add(0x06, 8);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, false_name_index, false_name_ptr_index));

      code.push_back(Bytecode(_AQVM_OPERATOR_IF, condition_index,
                              true_name_ptr_index, false_name_ptr_index));
    }*/
}

void BytecodeGenerator::HandleWhileStmt(WhileNode* stmt,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleWhileStmt(WhileNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  size_t start_location = code.size();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  size_t if_location = code.size();

  // Need body branch and exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  size_t body_location = code.size();

  HandleStmt(stmt->GetBody(), code);
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1, start_location));

  size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  /*// condition branch
  std::string condition_name("$condition_" +
                             std::to_string(++undefined_name_count_));
  size_t condition_name_index = global_memory_.Add(
      0x01, condition_name.size() + 1, condition_name.c_str());
  size_t condition_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(Bytecode(_AQVM_OPERATOR_PTR, condition_name_index,
                          condition_name_ptr_index));

  // body branch
  std::string body_name("$body_" + std::to_string(++undefined_name_count_));
  size_t body_name_index =
      global_memory_.Add(0x01, body_name.size() + 1, body_name.c_str());
  size_t body_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_PTR, body_name_index, body_name_ptr_index));

  // exit branch
  std::string exit_name("$exit");
  size_t exit_name_index =
      global_memory_.Add(0x01, exit_name.size() + 1, exit_name.c_str());
  size_t exit_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_PTR, exit_name_index, exit_name_ptr_index));

  // Generate body code
  std::vector<Bytecode> body_code;
  HandleStmt(stmt->GetBody(), body_code);
  body_code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, condition_name_ptr_index));

  std::vector<Bytecode> condition_code;
  condition_code.push_back(Bytecode(_AQVM_OPERATOR_IF, condition_index,
                                    body_name_ptr_index, exit_name_ptr_index));

  func_list_.push_back(Function(condition_name, condition_code));

  func_list_.push_back(Function(body_name, body_code));

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, condition_name_ptr_index, 0));
  */
}

std::size_t BytecodeGenerator::HandleFuncInvoke(FuncNode* func,
                                                std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (func == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func is nullptr.");

  std::vector<ExprNode*> args = func->GetArgs();
  auto iterator = func_decl_map.find(*func->GetName());
  if (iterator == func_decl_map.end())
    EXIT_COMPILER(
        "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "Not found function.");
  FuncDeclNode func_decl = iterator->second;

  Type* func_type = func_decl.GetReturnType();
  if (func_type == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func_type is nullptr.");

  while (func_type->GetType() != Type::TypeType::kBase &&
         func_type->GetType() != Type::TypeType::kPointer &&
         func_type->GetType() != Type::TypeType::kArray &&
         func_type->GetType() != Type::TypeType::kReference) {
    if (func_type->GetType() == Type::TypeType::NONE)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&"
          ")",
          "Unexpected code.");
    if (func_type->GetType() == Type::TypeType::kConst)
      func_type = dynamic_cast<ConstType*>(func_type)->GetSubType();
  }

  uint8_t vm_type = 0x00;
  if (func_type->GetType() == Type::TypeType::kBase) {
    switch (func_type->GetBaseType()) {
      case Type::BaseType::kVoid:
        vm_type = 0x00;
        break;
      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        vm_type = 0x01;
        break;
      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
        vm_type = 0x02;
        break;
      case Type::BaseType::kLong:
        vm_type = 0x03;
        break;
      case Type::BaseType::kFloat:
        vm_type = 0x04;
        break;
      case Type::BaseType::kDouble:
        vm_type = 0x05;
        break;
      case Type::BaseType::kStruct:
      case Type::BaseType::kUnion:
      case Type::BaseType::kEnum:
      case Type::BaseType::kPointer:
      case Type::BaseType::kArray:
      case Type::BaseType::kFunction:
      case Type::BaseType::kTypedef:
      case Type::BaseType::kAuto:
        vm_type = 0x06;
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }
  } else if (func_type->GetType() == Type::TypeType::kPointer ||
             func_type->GetType() == Type::TypeType::kArray ||
             func_type->GetType() == Type::TypeType::kReference) {
    vm_type = 0x06;
  }

  std::vector<std::size_t> vm_args;

  std::string func_name = std::string(*func->GetName());
  std::size_t func_name_index =
      global_memory_.Add(0x01, func_name.size() + 1, func_name.c_str());
  std::size_t func_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_PTR, 2, func_name_index, func_name_ptr_index));

  vm_args.push_back(func_name_ptr_index);
  vm_args.push_back(args.size() + 1);

  std::size_t return_value_index =
      global_memory_.Add(vm_type, func_type->GetSize());
  vm_args.push_back(return_value_index);

  for (std::size_t i = 0; i < args.size(); i++) {
    vm_args.push_back(HandleExpr(args[i], code));
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, vm_args));

  return vm_args[2];
}

std::size_t BytecodeGenerator::GetIndex(ExprNode* expr,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  switch (expr->GetType()) {
    case StmtNode::StmtType::kIdentifier: {
      auto iterator = var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
      if (iterator == var_decl_map.end()) {
        std::cout << "Identifier: "
                  << (std::string) * dynamic_cast<IdentifierNode*>(expr)
                  << std::endl;
        EXIT_COMPILER(
            "BytecodeGenerator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
            "Not found variable.");
      }

      return iterator->second.second;
    }

    case StmtNode::StmtType::kValue: {
      std::size_t vm_type = dynamic_cast<ValueNode*>(expr)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = dynamic_cast<ValueNode*>(expr)->GetCharValue();
          return global_memory_.Add(
              vm_type, dynamic_cast<ValueNode*>(expr)->GetSize(), &value);
          break;
        }

        case 0x02: {
          int value = dynamic_cast<ValueNode*>(expr)->GetIntValue();
          std::cout << "value: " << value << std::endl;
          value = is_big_endian_ ? value : SwapInt(value);
          return global_memory_.Add(vm_type, 4, &value);
        }

        case 0x03: {
          long value = dynamic_cast<ValueNode*>(expr)->GetLongValue();
          value = is_big_endian_ ? value : SwapLong(value);
          return global_memory_.Add(vm_type, 8, &value);
        }

        case 0x04: {
          float value = dynamic_cast<ValueNode*>(expr)->GetFloatValue();
          value = is_big_endian_ ? value : SwapFloat(value);
          return global_memory_.Add(vm_type, 4, &value);
        }

        case 0x05: {
          double value = dynamic_cast<ValueNode*>(expr)->GetDoubleValue();
          value = is_big_endian_ ? value : SwapDouble(value);
          return global_memory_.Add(vm_type, 8, &value);
        }

        case 0x06: {
          if (dynamic_cast<ValueNode*>(expr)->GetToken().type ==
              Token::Type::STRING) {
            std::string value =
                dynamic_cast<ValueNode*>(expr)->GetStringValue();
            std::size_t str_index =
                global_memory_.Add(0x01, value.size() + 1,
                                   static_cast<const void*>(value.c_str()));
            std::size_t ptr_index = global_memory_.Add(vm_type, 8);
            code.push_back(
                Bytecode(_AQVM_OPERATOR_PTR, 2, str_index, ptr_index));
            return ptr_index;
          }
          uint64_t value = dynamic_cast<ValueNode*>(expr)->GetUInt64Value();
          value = is_big_endian_ ? value : SwapUint64t(value);
          return global_memory_.Add(vm_type, 8, &value);
        }

        default:
          EXIT_COMPILER(
              "BytecodeGenerator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
              "Unexpected code.");
          break;
      }
    }

    case StmtNode::StmtType::kFunc:
      return HandleFuncInvoke(dynamic_cast<FuncNode*>(expr), code);

    default:
      return 0;
  }
}

std::size_t BytecodeGenerator::AddConstInt8t(int8_t value) {
  TRACE_FUNCTION;
  return global_memory_.Add(0x01, 1, &value);
}

uint8_t BytecodeGenerator::GetExprVmType(ExprNode* expr) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                  "expr is nullptr.");

  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kAddrOf) {
      return 0x06;
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::CONVERT) {
      switch (dynamic_cast<ConvertNode*>(expr)->GetConvertedType()->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (dynamic_cast<ConvertNode*>(expr)
                      ->GetConvertedType()
                      ->GetBaseType()) {
            case Type::BaseType::kVoid:
              return 0x00;
            case Type::BaseType::kBool:
            case Type::BaseType::kChar:
              return 0x01;
            case Type::BaseType::kShort:
            case Type::BaseType::kInt:
              return 0x02;
            case Type::BaseType::kLong:
              return 0x03;
            case Type::BaseType::kFloat:
              return 0x04;
            case Type::BaseType::kDouble:
              return 0x05;
            case Type::BaseType::kStruct:
            case Type::BaseType::kUnion:
            case Type::BaseType::kEnum:
            case Type::BaseType::kPointer:
            case Type::BaseType::kArray:
            case Type::BaseType::kFunction:
            case Type::BaseType::kTypedef:
            case Type::BaseType::kAuto:
              return 0x06;
            default:
              EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                            "Unexpected code.");
              break;
          }

        case Type::TypeType::kArray:
        case Type::TypeType::kPointer:
        case Type::TypeType::kReference:
          return 0x06;

        default:
          EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                        "Unexpected code.");
          break;
      }
    }

    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::ARRAY) {
      return GetExprPtrValueVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kDeref) {
      return GetExprPtrValueVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
    }

    return GetExprVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
  }

  if (expr->GetType() == StmtNode::StmtType::kBinary) {
    uint8_t left =
        GetExprVmType(dynamic_cast<BinaryNode*>(expr)->GetLeftExpr());
    uint8_t right =
        GetExprVmType(dynamic_cast<BinaryNode*>(expr)->GetRightExpr());

    return left > right ? left : right;
  }

  if (expr->GetType() == StmtNode::StmtType::kValue) {
    return dynamic_cast<ValueNode*>(expr)->GetVmType();
  }

  if (expr->GetType() == StmtNode::StmtType::kConditional) {
    uint8_t true_value =
        GetExprVmType(dynamic_cast<ConditionalNode*>(expr)->GetTrueExpr());
    uint8_t false_value =
        GetExprVmType(dynamic_cast<ConditionalNode*>(expr)->GetFalseExpr());

    return true_value > false_value ? true_value : false_value;
  }

  if (expr->GetType() == StmtNode::StmtType::kFunc) {
    auto iterator =
        func_decl_map.find(*dynamic_cast<FuncNode*>(expr)->GetName());
    if (iterator == func_decl_map.end())
      EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                    "Not found function define.");
    Type* return_type = iterator->second.GetReturnType();

    if (return_type == nullptr)
      EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                    "return_type is nullptr.");

    switch (return_type->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (return_type->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                          "Unexpected code.");
            break;
        }

      case Type::TypeType::kArray:
      case Type::TypeType::kPointer:
      case Type::TypeType::kReference:
        return 0x06;

      default:
        EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                      "Unexpected code.");
        break;
    }
  }

  if (expr->GetType() == StmtNode::StmtType::kIdentifier) {
    auto iterator = var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
    if (iterator == var_decl_map.end())
      EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                    "Not found identifier define.");
    switch (iterator->second.first->GetVarType()->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (iterator->second.first->GetVarType()->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                          "Unexpected code.");
            break;
        }

      case Type::TypeType::kArray:
      case Type::TypeType::kPointer:
      case Type::TypeType::kReference:
        return 0x06;

      default:
        EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                      "Unexpected code.");
        break;
    }
  }
  if (expr->GetType() == StmtNode::StmtType::kVarDecl) {
    switch (dynamic_cast<VarDeclNode*>(expr)->GetVarType()->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (dynamic_cast<VarDeclNode*>(expr)->GetVarType()->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                          "Unexpected code.");
            break;
        }

      case Type::TypeType::kArray:
      case Type::TypeType::kPointer:
      case Type::TypeType::kReference:
        return 0x06;

      default:
        EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                      "Unexpected code.");
        break;
    }
  }
  if (expr->GetType() == StmtNode::StmtType::kCast) {
    switch (dynamic_cast<CastNode*>(expr)->GetCastType()->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (dynamic_cast<CastNode*>(expr)->GetCastType()->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                          "Unexpected code.");
            break;
        }

      case Type::TypeType::kArray:
      case Type::TypeType::kPointer:
      case Type::TypeType::kReference:
        return 0x06;

      default:
        EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                      "Unexpected code.");
        break;
    }
  }
  EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                "Unexpected code.");
  return 0;
}

uint8_t BytecodeGenerator::GetExprPtrValueVmType(ExprNode* expr) {
  TRACE_FUNCTION;
  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kAddrOf) {
      return GetExprVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::CONVERT) {
      switch (dynamic_cast<ConvertNode*>(expr)->GetConvertedType()->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (dynamic_cast<ConvertNode*>(expr)
                      ->GetConvertedType()
                      ->GetBaseType()) {
            case Type::BaseType::kVoid:
              return 0x00;
            case Type::BaseType::kBool:
            case Type::BaseType::kChar:
              return 0x01;
            case Type::BaseType::kShort:
            case Type::BaseType::kInt:
              return 0x02;
            case Type::BaseType::kLong:
              return 0x03;
            case Type::BaseType::kFloat:
              return 0x04;
            case Type::BaseType::kDouble:
              return 0x05;
            case Type::BaseType::kStruct:
            case Type::BaseType::kUnion:
            case Type::BaseType::kEnum:
            case Type::BaseType::kPointer:
            case Type::BaseType::kArray:
            case Type::BaseType::kFunction:
            case Type::BaseType::kTypedef:
            case Type::BaseType::kAuto:
              return 0x06;
            default:
              return 0x00;
          }

        case Type::TypeType::kArray:
        case Type::TypeType::kPointer:
        case Type::TypeType::kReference:
          return GetExprPtrValueVmType(
              dynamic_cast<UnaryNode*>(expr)->GetExpr());

        default:
          return 0x00;
      }
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::ARRAY) {
      return GetExprPtrValueVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
    }
    return GetExprPtrValueVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
  }
  if (expr->GetType() == StmtNode::StmtType::kBinary) {
    uint8_t left =
        GetExprPtrValueVmType(dynamic_cast<BinaryNode*>(expr)->GetLeftExpr());
    uint8_t right =
        GetExprPtrValueVmType(dynamic_cast<BinaryNode*>(expr)->GetRightExpr());

    return left > right ? left : right;
  }
  if (expr->GetType() == StmtNode::StmtType::kValue) {
    return dynamic_cast<ValueNode*>(expr)->GetVmType();
  }
  if (expr->GetType() == StmtNode::StmtType::kConditional) {
    uint8_t true_value = GetExprPtrValueVmType(
        dynamic_cast<ConditionalNode*>(expr)->GetTrueExpr());
    uint8_t false_value = GetExprPtrValueVmType(
        dynamic_cast<ConditionalNode*>(expr)->GetFalseExpr());

    return true_value > false_value ? true_value : false_value;
  }
  if (expr->GetType() == StmtNode::StmtType::kFunc) {
    auto iterator =
        func_decl_map.find(*dynamic_cast<FuncNode*>(expr)->GetName());
    if (iterator == func_decl_map.end())
      EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                    "Not found func.");
    Type* return_type = iterator->second.GetReturnType();
    if (return_type == nullptr)
      EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                    "return_type is nullptr.");
    switch (return_type->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (return_type->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            return 0x00;
        }

      case Type::TypeType::kArray:
        switch (
            dynamic_cast<ArrayType*>(return_type)->GetSubType()->GetType()) {
          case Type::TypeType::kBase:
          case Type::TypeType::kConst:
            switch (dynamic_cast<ArrayType*>(return_type)
                        ->GetSubType()
                        ->GetBaseType()) {
              case Type::BaseType::kVoid:
                return 0x00;
              case Type::BaseType::kBool:
              case Type::BaseType::kChar:
                return 0x01;
              case Type::BaseType::kShort:
              case Type::BaseType::kInt:
                return 0x02;
              case Type::BaseType::kLong:
                return 0x03;
              case Type::BaseType::kFloat:
                return 0x04;
              case Type::BaseType::kDouble:
                return 0x05;
              case Type::BaseType::kStruct:
              case Type::BaseType::kUnion:
              case Type::BaseType::kEnum:
              case Type::BaseType::kPointer:
              case Type::BaseType::kArray:
              case Type::BaseType::kFunction:
              case Type::BaseType::kTypedef:
              case Type::BaseType::kAuto:
                return 0x06;
              default:
                return 0x00;
            }

          case Type::TypeType::kArray:
          case Type::TypeType::kPointer:
          case Type::TypeType::kReference:
            return 0x06;

          default:
            return 0x00;
        }
      case Type::TypeType::kPointer:
        switch (
            dynamic_cast<PointerType*>(return_type)->GetSubType()->GetType()) {
          case Type::TypeType::kBase:
          case Type::TypeType::kConst:
            switch (dynamic_cast<PointerType*>(return_type)
                        ->GetSubType()
                        ->GetBaseType()) {
              case Type::BaseType::kVoid:
                return 0x00;
              case Type::BaseType::kBool:
              case Type::BaseType::kChar:
                return 0x01;
              case Type::BaseType::kShort:
              case Type::BaseType::kInt:
                return 0x02;
              case Type::BaseType::kLong:
                return 0x03;
              case Type::BaseType::kFloat:
                return 0x04;
              case Type::BaseType::kDouble:
                return 0x05;
              case Type::BaseType::kStruct:
              case Type::BaseType::kUnion:
              case Type::BaseType::kEnum:
              case Type::BaseType::kPointer:
              case Type::BaseType::kArray:
              case Type::BaseType::kFunction:
              case Type::BaseType::kTypedef:
              case Type::BaseType::kAuto:
                return 0x06;
              default:
                return 0x00;
            }

          case Type::TypeType::kArray:
          case Type::TypeType::kPointer:
          case Type::TypeType::kReference:
            return 0x06;

          default:
            return 0x00;
        }
      case Type::TypeType::kReference:
        switch (dynamic_cast<ReferenceType*>(return_type)
                    ->GetSubType()
                    ->GetType()) {
          case Type::TypeType::kBase:
          case Type::TypeType::kConst:
            switch (dynamic_cast<ReferenceType*>(return_type)
                        ->GetSubType()
                        ->GetBaseType()) {
              case Type::BaseType::kVoid:
                return 0x00;
              case Type::BaseType::kBool:
              case Type::BaseType::kChar:
                return 0x01;
              case Type::BaseType::kShort:
              case Type::BaseType::kInt:
                return 0x02;
              case Type::BaseType::kLong:
                return 0x03;
              case Type::BaseType::kFloat:
                return 0x04;
              case Type::BaseType::kDouble:
                return 0x05;
              case Type::BaseType::kStruct:
              case Type::BaseType::kUnion:
              case Type::BaseType::kEnum:
              case Type::BaseType::kPointer:
              case Type::BaseType::kArray:
              case Type::BaseType::kFunction:
              case Type::BaseType::kTypedef:
              case Type::BaseType::kAuto:
                return 0x06;
              default:
                return 0x00;
            }

          case Type::TypeType::kArray:
          case Type::TypeType::kPointer:
          case Type::TypeType::kReference:
            return 0x06;

          default:
            return 0x00;
        }
      default:
        return 0x00;
    }
    if (expr->GetType() == StmtNode::StmtType::kIdentifier) {
      auto iterator = var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
      if (iterator == var_decl_map.end())
        EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                      "Not found variable.");
      switch (iterator->second.first->GetVarType()->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (iterator->second.first->GetVarType()->GetBaseType()) {
            case Type::BaseType::kVoid:
              return 0x00;
            case Type::BaseType::kBool:
            case Type::BaseType::kChar:
              return 0x01;
            case Type::BaseType::kShort:
            case Type::BaseType::kInt:
              return 0x02;
            case Type::BaseType::kLong:
              return 0x03;
            case Type::BaseType::kFloat:
              return 0x04;
            case Type::BaseType::kDouble:
              return 0x05;
            case Type::BaseType::kStruct:
            case Type::BaseType::kUnion:
            case Type::BaseType::kEnum:
            case Type::BaseType::kPointer:
            case Type::BaseType::kArray:
            case Type::BaseType::kFunction:
            case Type::BaseType::kTypedef:
            case Type::BaseType::kAuto:
              return 0x06;
            default:
              return 0x00;
          }

        case Type::TypeType::kArray: {
          auto iterator =
              var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
          if (iterator == var_decl_map.end())
            EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                          "Not found variable.");
          switch (iterator->second.first->GetVarType()->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (
                  dynamic_cast<ArrayType*>(iterator->second.first->GetVarType())
                      ->GetSubType()
                      ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }
        }

        case Type::TypeType::kPointer: {
          auto iterator =
              var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
          if (iterator == var_decl_map.end())
            EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                          "Not found variable.");
          switch (iterator->second.first->GetVarType()->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (dynamic_cast<PointerType*>(
                          iterator->second.first->GetVarType())
                          ->GetSubType()
                          ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }
        }

        case Type::TypeType::kReference: {
          auto iterator =
              var_decl_map.find(*dynamic_cast<IdentifierNode*>(expr));
          if (iterator == var_decl_map.end())
            EXIT_COMPILER("BytecodeGenerator::GetExprPtrValueVmType(ExprNode*)",
                          "Not found variable.");
          switch (iterator->second.first->GetVarType()->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (dynamic_cast<ReferenceType*>(
                          iterator->second.first->GetVarType())
                          ->GetSubType()
                          ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }
        }

        default:
          return 0x00;
      }
    }
    if (expr->GetType() == StmtNode::StmtType::kVarDecl) {
      switch (dynamic_cast<VarDeclNode*>(expr)->GetVarType()->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (
              dynamic_cast<VarDeclNode*>(expr)->GetVarType()->GetBaseType()) {
            case Type::BaseType::kVoid:
              return 0x00;
            case Type::BaseType::kBool:
            case Type::BaseType::kChar:
              return 0x01;
            case Type::BaseType::kShort:
            case Type::BaseType::kInt:
              return 0x02;
            case Type::BaseType::kLong:
              return 0x03;
            case Type::BaseType::kFloat:
              return 0x04;
            case Type::BaseType::kDouble:
              return 0x05;
            case Type::BaseType::kStruct:
            case Type::BaseType::kUnion:
            case Type::BaseType::kEnum:
            case Type::BaseType::kPointer:
            case Type::BaseType::kArray:
            case Type::BaseType::kFunction:
            case Type::BaseType::kTypedef:
            case Type::BaseType::kAuto:
              return 0x06;
            default:
              return 0x00;
          }

        case Type::TypeType::kArray:
          switch (dynamic_cast<ArrayType*>(
                      dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                      ->GetSubType()
                      ->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (dynamic_cast<ArrayType*>(
                          dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                          ->GetSubType()
                          ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }
        case Type::TypeType::kPointer:
          switch (dynamic_cast<PointerType*>(
                      dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                      ->GetSubType()
                      ->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (dynamic_cast<PointerType*>(
                          dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                          ->GetSubType()
                          ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }
        case Type::TypeType::kReference:
          switch (dynamic_cast<ReferenceType*>(
                      dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                      ->GetSubType()
                      ->GetType()) {
            case Type::TypeType::kBase:
            case Type::TypeType::kConst:
              switch (dynamic_cast<ReferenceType*>(
                          dynamic_cast<VarDeclNode*>(expr)->GetVarType())
                          ->GetSubType()
                          ->GetBaseType()) {
                case Type::BaseType::kVoid:
                  return 0x00;
                case Type::BaseType::kBool:
                case Type::BaseType::kChar:
                  return 0x01;
                case Type::BaseType::kShort:
                case Type::BaseType::kInt:
                  return 0x02;
                case Type::BaseType::kLong:
                  return 0x03;
                case Type::BaseType::kFloat:
                  return 0x04;
                case Type::BaseType::kDouble:
                  return 0x05;
                case Type::BaseType::kStruct:
                case Type::BaseType::kUnion:
                case Type::BaseType::kEnum:
                case Type::BaseType::kPointer:
                case Type::BaseType::kArray:
                case Type::BaseType::kFunction:
                case Type::BaseType::kTypedef:
                case Type::BaseType::kAuto:
                  return 0x06;
                default:
                  return 0x00;
              }

            case Type::TypeType::kArray:
            case Type::TypeType::kPointer:
            case Type::TypeType::kReference:
              return 0x06;

            default:
              return 0x00;
          }

        default:
          return 0x00;
      }
    }
  }
  if (expr->GetType() == StmtNode::StmtType::kCast) {
    switch (dynamic_cast<CastNode*>(expr)->GetCastType()->GetType()) {
      case Type::TypeType::kBase:
      case Type::TypeType::kConst:
        switch (dynamic_cast<CastNode*>(expr)->GetCastType()->GetBaseType()) {
          case Type::BaseType::kVoid:
            return 0x00;
          case Type::BaseType::kBool:
          case Type::BaseType::kChar:
            return 0x01;
          case Type::BaseType::kShort:
          case Type::BaseType::kInt:
            return 0x02;
          case Type::BaseType::kLong:
            return 0x03;
          case Type::BaseType::kFloat:
            return 0x04;
          case Type::BaseType::kDouble:
            return 0x05;
          case Type::BaseType::kStruct:
          case Type::BaseType::kUnion:
          case Type::BaseType::kEnum:
          case Type::BaseType::kPointer:
          case Type::BaseType::kArray:
          case Type::BaseType::kFunction:
          case Type::BaseType::kTypedef:
          case Type::BaseType::kAuto:
            return 0x06;
          default:
            return 0x00;
        }

      case Type::TypeType::kArray:
      case Type::TypeType::kPointer:
      case Type::TypeType::kReference:
        return 0x06;

      default:
        return 0x00;
    }
  }
  return 0x00;
}

std::size_t BytecodeGenerator::GetExprVmSize(uint8_t type) {
  TRACE_FUNCTION;
  switch (type) {
    case 0x01:
      return 1;
    case 0x02:
      return 4;
    case 0x03:
      return 8;
    case 0x04:
      return 4;
    case 0x05:
      return 8;
    case 0x06:
      return 8;
    default:
      return 0;
  }
}
int BytecodeGenerator::SwapInt(int x) {
  TRACE_FUNCTION;
  uint32_t ux = (uint32_t)x;
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  return (int)ux;
}

long BytecodeGenerator::SwapLong(long x) {
  TRACE_FUNCTION;
  uint64_t ux = (uint64_t)x;
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  return (long)ux;
}

float BytecodeGenerator::SwapFloat(float x) {
  TRACE_FUNCTION;
  uint32_t ux;
  memcpy(&ux, &x, sizeof(uint32_t));
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  float result;
  memcpy(&result, &ux, sizeof(float));
  return result;
}

double BytecodeGenerator::SwapDouble(double x) {
  TRACE_FUNCTION;
  uint64_t ux;
  memcpy(&ux, &x, sizeof(uint64_t));
  ux = ((ux << 56) & 0xFF00000000000000ULL) |
       ((ux << 40) & 0x00FF000000000000ULL) |
       ((ux << 24) & 0x0000FF0000000000ULL) |
       ((ux << 8) & 0x000000FF00000000ULL) |
       ((ux >> 8) & 0x00000000FF000000ULL) |
       ((ux >> 24) & 0x0000000000FF0000ULL) |
       ((ux >> 40) & 0x000000000000FF00ULL) |
       ((ux >> 56) & 0x00000000000000FFULL);
  double result;
  memcpy(&result, &ux, sizeof(double));
  return result;
}
uint64_t BytecodeGenerator::SwapUint64t(uint64_t x) {
  TRACE_FUNCTION;
  x = ((x << 56) & 0xFF00000000000000ULL) |
      ((x << 40) & 0x00FF000000000000ULL) |
      ((x << 24) & 0x0000FF0000000000ULL) | ((x << 8) & 0x000000FF00000000ULL) |
      ((x >> 8) & 0x00000000FF000000ULL) | ((x >> 24) & 0x0000000000FF0000ULL) |
      ((x >> 40) & 0x000000000000FF00ULL) | ((x >> 56) & 0x00000000000000FFULL);
  return x;
}

void BytecodeGenerator::InsertUint64ToCode(uint64_t value) {
  TRACE_FUNCTION;
  for (int i = 0; i < 8; ++i) {
    code_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
  }
}

size_t BytecodeGenerator::EncodeUleb128(size_t value,
                                        std::vector<uint8_t>& output) {
  TRACE_FUNCTION;
  size_t count = 0;
  do {
    uint8_t byte = value & 0x7F;
    value >>= 7;
    if (value != 0) {
      byte |= 0x80;
    }
    output.push_back(byte);
    count++;
  } while (value != 0);
  return count;
}

}  // namespace Compiler
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
  Aq::Compiler::Lexer lexer(buffer_ptr_, code.size());
  std::vector<Aq::Compiler::Token> token;
  Aq::Compiler::Token first_token_buffer;
  lexer.LexToken(first_token_buffer,first_token_buffer);
  token.push_back(first_token_buffer);
  while (true) {
    Aq::Compiler::Token token_buffer;
    lexer.LexToken(token.back(),token_buffer);
    token.push_back(token_buffer);
    if (lexer.IsReadEnd()) {
      break;
    }
  }

  std::cout << "Lex End." << std::endl;

  Aq::Compiler::CompoundNode* ast = Aq::Compiler::Parser::Parse(token);

  if (ast == nullptr) printf("ast is nullptr\n");

  std::cout << "Parse End." << std::endl;

  Aq::Compiler::BytecodeGenerator bytecode_generator;

  bytecode_generator.GenerateBytecode(ast);

  return 0;
}