// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cassert>
#include <chrono>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
// #include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define _AQVM_OPERATOR_NOP 0x00
#define _AQVM_OPERATOR_LOAD 0x01
#define _AQVM_OPERATOR_STORE 0x02
#define _AQVM_OPERATOR_NEW 0x03
#define _AQVM_OPERATOR_ARRAY 0x04
#define _AQVM_OPERATOR_PTR 0x05
#define _AQVM_OPERATOR_ADD 0x06
#define _AQVM_OPERATOR_SUB 0x07
#define _AQVM_OPERATOR_MUL 0x08
#define _AQVM_OPERATOR_DIV 0x09
#define _AQVM_OPERATOR_REM 0x0A
#define _AQVM_OPERATOR_NEG 0x0B
#define _AQVM_OPERATOR_SHL 0x0C
#define _AQVM_OPERATOR_SHR 0x0D
#define _AQVM_OPERATOR_REFER 0x0E
#define _AQVM_OPERATOR_IF 0x0F
#define _AQVM_OPERATOR_AND 0x10
#define _AQVM_OPERATOR_OR 0x11
#define _AQVM_OPERATOR_XOR 0x12
#define _AQVM_OPERATOR_CMP 0x13
#define _AQVM_OPERATOR_INVOKE 0x14
#define _AQVM_OPERATOR_EQUAL 0x15
#define _AQVM_OPERATOR_GOTO 0x16
#define _AQVM_OPERATOR_LOAD_CONST 0x17
#define _AQVM_OPERATOR_CONVERT 0x18
#define _AQVM_OPERATOR_CONST 0x19
#define _AQVM_OPERATOR_INVOKE_METHOD 0x1A
#define _AQVM_OPERATOR_LOAD_MEMBER 0x1B
#define _AQVM_OPERATOR_WIDE 0xFF

inline void EXIT_COMPILER(const char* func_name, const char* message) {
  std::cerr << "[ERROR] " << func_name << ": " << message << std::endl;
  exit(EXIT_FAILURE);
}

/*#define TRACE_FUNCTION Trace trace(__FUNCTION__)

std::stack<std::string> call_stack;

class Trace {
 public:
  explicit Trace(const std::string& function_name) {
    call_stack.push(function_name);
    PrintStack();
  }

  ~Trace() {
    call_stack.pop();
    PrintStack();
  }

 private:
  void PrintStack() const {
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
};*/
#define TRACE_FUNCTION

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
    for (std::size_t i = 0; i < capacity_; i++) {
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
    As,
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
    Func,
    Float,
    For,
    Friend,
    From,
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
    Var,
    Virtual,
    Void,
    Wchar_t,
    While,
    Xor,
  };
  enum class OperatorType {
    // TODO: Add more operators.
    NONE = 0,
    l_square,   //[
    r_square,   //]
    l_paren,    //(
    r_paren,    //)
    l_brace,    //{
    r_brace,    //}
    period,     //.
    ellipsis,   //...
    amp,        //&
    ampamp,     //&&
    ampequal,   //&=
    star,       //*
    starequal,  //*=
    plus,       //+
    plusplus,   //++
    plusequal,  //+=
    minus,      //-
    // arrow,                  //->
    minusminus,           //--
    minusequal,           //-=
    tilde,                //~
    exclaim,              //!
    exclaimequal,         //!=
    slash,                ///
    slashequal,           ///=
    percent,              //%
    percentequal,         //%=
    less,                 //<
    lessless,             //<<
    lessequal,            //<=
    lesslessequal,        //<<=
    spaceship,            //<=>
    greater,              //>
    greatergreater,       //>>
    greaterequal,         //>=
    greatergreaterequal,  //>>=
    caret,                //^
    caretequal,           //^=
    pipe,                 //|
    pipepipe,             //||
    pipeequal,            //|=
    question,             //?
    colon,                //:
    semi,                 //;
    equal,                //=
    equalequal,           //==
    comma,                //,
    // hash,                   // #
    // hashhash,               // ##
    // hashat,                 // #@
    // periodstar,             //.*
    // arrowstar,              //->*
    // coloncolon,             //::
    // at,                     //@
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
      case Token::KeywordType::Func:
        return "Func";
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
      case Token::KeywordType::Var:
        return "Var";
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
      /*case Token::OperatorType::arrow:
        return "->";*/
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
      /*case Token::OperatorType::hash:
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
        return "@";*/
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
  TRACE_FUNCTION;
  keyword_map.Insert("as", Token::KeywordType::As);
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
  keyword_map.Insert("func", Token::KeywordType::Func);
  keyword_map.Insert("float", Token::KeywordType::Float);
  keyword_map.Insert("for", Token::KeywordType::For);
  keyword_map.Insert("friend", Token::KeywordType::Friend);
  keyword_map.Insert("from", Token::KeywordType::From);
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
  keyword_map.Insert("var", Token::KeywordType::Var);
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
  // operator_map.Insert("->", Token::OperatorType::arrow);
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
  /*operator_map.Insert("#", Token::OperatorType::hash);
  operator_map.Insert("##", Token::OperatorType::hashhash);
  operator_map.Insert("#@", Token::OperatorType::hashat);
  operator_map.Insert(".*", Token::OperatorType::periodstar);
  operator_map.Insert("->*", Token::OperatorType::arrowstar);
  operator_map.Insert("::", Token::OperatorType::coloncolon);
  operator_map.Insert("@", Token::OperatorType::at);*/
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
        if (read_ptr + 2 <= buffer_end_) {
          char escaped_char = *(read_ptr + 1);
          if (escaped_char == 'x' && read_ptr + 3 <= buffer_end_ &&
              isxdigit(*(read_ptr + 2)) && isxdigit(*(read_ptr + 3))) {
            char hex_value[3] = {*(read_ptr + 2), *(read_ptr + 3), '\0'};
            *read_ptr = static_cast<char>(strtol(hex_value, nullptr, 16));
            read_ptr++;
            memmove(read_ptr, read_ptr + 3, buffer_end_ - read_ptr - 3);
            buffer_end_ -= 3;
          } else if (isdigit(escaped_char) && escaped_char < '8' &&
                     read_ptr + 3 <= buffer_end_ && isdigit(*(read_ptr + 2)) &&
                     isdigit(*(read_ptr + 3))) {
            char oct_value[4] = {escaped_char, *(read_ptr + 2), *(read_ptr + 3),
                                 '\0'};
            *read_ptr = static_cast<char>(strtol(oct_value, nullptr, 8));
            read_ptr++;
            memmove(read_ptr, read_ptr + 3, buffer_end_ - read_ptr - 3);
            buffer_end_ -= 3;
          } else {
            switch (escaped_char) {
              case 'n':
                *read_ptr = '\n';
                break;
              case 't':
                *read_ptr = '\t';
                break;
              case 'r':
                *read_ptr = '\r';
                break;
              case 'b':
                *read_ptr = '\b';
                break;
              case 'f':
                *read_ptr = '\f';
                break;
              case 'v':
                *read_ptr = '\v';
                break;
              case 'a':
                *read_ptr = '\a';
                break;
              case '\\':
                *read_ptr = '\\';
                break;
              case '\'':
                *read_ptr = '\'';
                break;
              case '\"':
                *read_ptr = '\"';
                break;
              case '\?':
                *read_ptr = '\?';
                break;
              case '0':
                *read_ptr = '\0';
                break;
              default:
                EXIT_COMPILER("Lexer::LexToken(Token,Token&)",
                              "Unknown escape character.");
                break;
            }
            read_ptr++;
            memmove(read_ptr, read_ptr + 1, buffer_end_ - read_ptr - 1);
            buffer_end_--;
          }
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
  enum class TypeType {
    NONE,
    kBase,
    kConst,
    kPointer,
    kArray,
    kReference,
    kClass
  };

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
    kString,
    kClass,
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

  static Type* CreateDoubleType();

  std::vector<uint8_t> GetVmType();

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
      case BaseType::kString:
      case BaseType::kClass:
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
        EXIT_COMPILER("Type::GetSize()", "Unknown type.");
    }
    return 0;
  }

  virtual operator std::string();

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
    kImport,
    kBreak,
    kCompound,
    kDecl,
    kExpr,
    kFuncDecl,
    kVarDecl,
    kClassDecl,
    kIf,
    kWhile,
    kDowhile,
    kFor,
    kSwitch,
    kCase,
    kLabel,
    kGoto,
    kValue,
    kIdentifier,
    kUnary,
    kBinary,
    kConditional,
    kFunc,
    // kCast,
    kArrayDecl,
    kArray,
    kReturn,
    kStatic
    // kArrow,
    // kMember
  };

  virtual StmtType GetType() { return type_; }

  StmtNode(const StmtNode&) = default;
  StmtNode& operator=(const StmtNode&) = default;

 protected:
  StmtType type_;
};

class ImportNode : public StmtNode {
 public:
  ImportNode() { type_ = StmtType::kImport; }
  virtual ~ImportNode() = default;

  void SetImportNode(std::string import_location, std::string name) {
    import_location_ = import_location;
    name_ = name;
  }

  void SetFromImport(std::string import_location,
                     std::vector<std::string> content,
                     std::vector<std::string> alias) {
    is_from_import_ = true;
  }

  bool IsFromImport() { return is_from_import_; }
  std::string GetImportLocation() { return import_location_; }
  std::string GetName() { return name_; }
  std::vector<std::string> GetContent() { return content_; };
  std::vector<std::string> GetAlias() { return alias_; };

  ImportNode(const ImportNode&) = default;
  ImportNode& operator=(const ImportNode&) = default;

 private:
  bool is_from_import_ = false;
  std::string import_location_;
  std::string name_;
  std::vector<std::string> content_;
  std::vector<std::string> alias_;
};

class BreakNode : public StmtNode {
 public:
  BreakNode() { type_ = StmtType::kBreak; }
  virtual ~BreakNode() = default;

  BreakNode(const BreakNode&) = default;
  BreakNode& operator=(const BreakNode&) = default;
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

class ExprNode : virtual public StmtNode {
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

  /*bool GetBoolValue() {
    if (value_.value.keyword == Token::KeywordType::True) {
      return true;
    } else if (value_.value.keyword == Token::KeywordType::False) {
      return false;
    }
  }*/
  int8_t GetByteValue() {
    if (value_.type == Token::Type::KEYWORD) {
      if (value_.value.keyword == Token::KeywordType::True) {
        return true;
      } else if (value_.value.keyword == Token::KeywordType::False) {
        return false;
      } else {
        EXIT_COMPILER("ValueNode::GetByteValue()", "Unexpected type.");
      }
    }
    return value_.value.character;
  }
  std::string GetStringValue() { return *value_.value.string; }
  int GetIntValue() {
    /*std::cout << "stoi: "
              << std::stoi(std::string(value_.value.number.location,
                                       value_.value.number.length))
              << std::endl;*/
    return std::stoi(
        std::string(value_.value.number.location, value_.value.number.length));
  }
  int64_t GetLongValue() {
    /*std::cout << "stoll: "
              << std::stoll(std::string(value_.value.number.location,
                                       value_.value.number.length))
              << std::endl;*/
    return std::stoll(
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
    /*std::cout << "stoull: "
              << std::stoull(std::string(value_.value.number.location,
                                         value_.value.number.length))
              << std::endl;*/
    return std::stoull(
        std::string(value_.value.number.location, value_.value.number.length));
  }

  /*std::variant<char,const char*, int, int64_t, float, double, uint64_t>
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
      int64_t int64_t_value = std::stol(str, &pos);
      if (pos == str.size()) {
        return int64_t_value;
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
    if (value_.type == Token::Type::KEYWORD) {
      if (value_.value.keyword == Token::KeywordType::True ||
          value_.value.keyword == Token::KeywordType::False) {
        return 0x01;
      } else {
        EXIT_COMPILER("ValueNode::GetVmType()", "Unexpected type.");
      }
    }
    if (value_.type == Token::Type::CHARACTER) {
      return 0x01;
    }
    if (value_.type == Token::Type::STRING) {
      return 0x05;
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
      (void)std::stoll(str, &pos);
      if (pos == str.size()) {
        return 0x02;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stoull(str, &pos);
      if (pos == str.size()) {
        return 0x04;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stof(str, &pos);
      if (pos == str.size()) {
        return 0x03;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    try {
      std::size_t pos;
      (void)std::stod(str, &pos);
      if (pos == str.size()) {
        return 0x03;
      }
    } catch (const std::invalid_argument&) {
    } catch (const std::out_of_range&) {
    }

    return 0x00;
  }

  Type* GetValueType();

  std::size_t GetSize() {
    if (value_.type == Token::Type::CHARACTER) {
      return 1;
    }
    if (value_.type == Token::Type::STRING) {
      return value_.value.string->size();
    }
    switch (GetVmType()) {
      case 0x01:
        return 1;
      case 0x02:
        return 8;
      case 0x03:
        return 8;
      case 0x04:
        return 8;
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
    kPostInc,     // ++
    kPostDec,     // --
    kPreInc,      // ++
    kPreDec,      // --
    kAddrOf,      // &
    kDeref,       // *
    kPlus,        // +
    kMinus,       // -
    kNot,         // !
    kBitwiseNot,  // ~
    ARRAY,        // []
    CONVERT       // ()
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
    kArrow,      // ->
    kMember,     // .
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

  void EnableVaFlag() { va_flag_ = true; }

  bool GetVaFlag() { return va_flag_; }

  FuncNode(const FuncNode&) = default;
  FuncNode& operator=(const FuncNode&) = default;

 private:
  ExprNode* name_;
  std::vector<ExprNode*> args_;
  bool va_flag_ = false;
};

class IdentifierNode : public ExprNode {
 public:
  IdentifierNode() { type_ = StmtType::kIdentifier; }
  void SetIdentifierNode(Token name) { name_ = name; }
  virtual ~IdentifierNode() = default;

  Token& GetNameToken() { return name_; }

  operator std::string() override {
    return std::string(name_.value.identifier.location,
                       name_.value.identifier.length);
  }

  IdentifierNode(const IdentifierNode&) = default;
  IdentifierNode& operator=(const IdentifierNode&) = default;

 private:
  Token name_;
};

class DeclNode : virtual public StmtNode {
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
    ExprNode::type_ = StmtType::kVarDecl;
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
    value_.clear();
  }
  void SetArrayDeclNode(Type* type, ExprNode* name, ExprNode* size,
                        std::vector<ExprNode*> value) {
    var_type_ = type;
    name_ = name;
    size_ = size;
    value_ = value;
  }

  virtual ~ArrayDeclNode() = default;

  ExprNode* GetSize() { return size_; }

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

class StaticNode : public DeclNode {
 public:
  StaticNode() { type_ = StmtType::kStatic; }
  ~StaticNode() = default;

  void SetDecl(DeclNode* decl) { decl_ = decl; }

  DeclNode* GetDecl() { return decl_; }

  StaticNode(const StaticNode&) = default;
  StaticNode& operator=(const StaticNode&) = default;

 private:
  DeclNode* decl_ = nullptr;
};

class ClassDeclNode : public DeclNode {
 public:
  ClassDeclNode() { type_ = StmtType::kClassDecl; }
  ~ClassDeclNode() = default;

  void SetClassDeclNode(IdentifierNode name,
                        std::vector<StaticNode*> static_members,
                        std::vector<VarDeclNode*> members,
                        std::vector<FuncDeclNode*> methods,
                        std::vector<ClassDeclNode*> class_decl) {
    name_ = name;
    static_members_ = static_members;
    members_ = members;
    methods_ = methods;
    class_ = class_decl;
  }

  IdentifierNode GetName() { return name_; }
  std::vector<StaticNode*> GetStaticMembers() { return static_members_; }
  std::vector<VarDeclNode*> GetMembers() { return members_; }
  std::vector<FuncDeclNode*> GetMethods() { return methods_; }
  std::vector<ClassDeclNode*> GetClasses() { return class_; }

  ClassDeclNode(const ClassDeclNode&) = default;
  ClassDeclNode& operator=(const ClassDeclNode&) = default;

 private:
  IdentifierNode name_;
  std::vector<StaticNode*> static_members_;
  std::vector<VarDeclNode*> members_;
  std::vector<FuncDeclNode*> methods_;
  std::vector<ClassDeclNode*> class_;
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

class DowhileNode : public StmtNode {
 public:
  DowhileNode() {
    type_ = StmtType::kDowhile;
    body_ = nullptr;
    condition_ = nullptr;
  }
  virtual ~DowhileNode() = default;

  void SetDowhileNode(ExprNode* condition, StmtNode* body) {
    condition_ = condition;
    body_ = body;
  }

  ExprNode* GetCondition() { return condition_; }
  StmtNode* GetBody() { return body_; }

  DowhileNode(const DowhileNode&) = default;
  DowhileNode& operator=(const DowhileNode&) = default;

 private:
  ExprNode* condition_;
  StmtNode* body_;
};

class ForNode : public StmtNode {
 public:
  ForNode() {
    type_ = StmtType::kFor;
    body_ = nullptr;
    start_ = nullptr;
    condition_ = nullptr;
    end_ = nullptr;
  }
  virtual ~ForNode() = default;

  void SetForNode(ExprNode* start, ExprNode* condition, ExprNode* end,
                  StmtNode* body) {
    body_ = body;
    start_ = start;
    condition_ = condition;
    end_ = end;
  }

  ExprNode* GetStart() { return start_; }
  ExprNode* GetCondition() { return condition_; }
  ExprNode* GetEnd() { return end_; }
  StmtNode* GetBody() { return body_; }

  ForNode(const ForNode&) = default;
  ForNode& operator=(const ForNode&) = default;

 private:
  ExprNode* start_;
  ExprNode* condition_;
  ExprNode* end_;
  StmtNode* body_;
};

class SwitchNode : public StmtNode {
 public:
  SwitchNode() { type_ = StmtType::kSwitch; }
  virtual ~SwitchNode() = default;

  void SetSwitchNode(ExprNode* expr, StmtNode* stmts) {
    expr_ = expr;
    stmts_ = stmts;
  }

  ExprNode* GetExpr() { return expr_; }

  StmtNode* GetStmts() { return stmts_; }

 private:
  ExprNode* expr_;
  StmtNode* stmts_;
};

class CaseNode : public StmtNode {
 public:
  CaseNode() { type_ = StmtType::kCase; }
  virtual ~CaseNode() = default;

  void SetCaseNode(ExprNode* expr, std::vector<StmtNode*> stmts) {
    expr_ = expr;
    stmts_ = stmts;
  }

  ExprNode* GetExpr() { return expr_; }

  std::vector<StmtNode*> GetStmts() { return stmts_; }

 private:
  ExprNode* expr_;
  std::vector<StmtNode*> stmts_;
};

class LabelNode : public StmtNode {
 public:
  LabelNode() { type_ = StmtType::kLabel; }
  void SetLabelNode(IdentifierNode label) { label_ = label; }
  virtual ~LabelNode() = default;

  IdentifierNode GetLabel() { return label_; }

  LabelNode(const LabelNode&) = default;
  LabelNode& operator=(const LabelNode&) = default;

 private:
  IdentifierNode label_;
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

/*class CastNode : public ExprNode {
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
};*/

class ReturnNode : public StmtNode {
 public:
  ReturnNode() { type_ = StmtType::kReturn; }
  void SetReturnNode(ExprNode* expr) { expr_ = expr; }
  virtual ~ReturnNode() = default;

  ExprNode* GetExpr() { return expr_; }

  ReturnNode(const ReturnNode&) = default;
  ReturnNode& operator=(const ReturnNode&) = default;

 private:
  ExprNode* expr_;
};

/*class ArrowNode : public ExprNode {
 public:
  ArrowNode() {
    type_ = StmtType::kArrow;
    expr_ = nullptr;
    member_ = nullptr;
  }

  void SetArrowNode(ExprNode* expr, ExprNode* member) {
    expr_ = expr;
    member_ = member;
  }

  virtual ~ArrowNode() = default;

  ExprNode* GetExpr() { return expr_; }
  ExprNode* GetMember() { return member_; }

  ArrowNode(const ArrowNode&) = default;
  ArrowNode& operator=(const ArrowNode&) = default;

 private:
  ExprNode* expr_;
  ExprNode* member_;
};

class MemberNode: public ExprNode{
  public:
  MemberNode() {
    type_ = StmtType::kMember;
    expr_ = nullptr;
    member_ = nullptr;
  }

  void SetMemberNode(ExprNode* expr, ExprNode* member) {
    expr_ = expr;
    member_ = member;
  }

  virtual ~MemberNode() = default;

  ExprNode* GetExpr() { return expr_; }
  ExprNode* GetMember() { return member_; }

  MemberNode(const MemberNode&) = default;
  MemberNode& operator=(const MemberNode&) = default;

 private:
  ExprNode* expr_;
  ExprNode* member_;
};*/

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

  static ExprNode* ParseExprWithoutComma(Token* token, std::size_t length,
                                         std::size_t& index);

  static ExprNode* ParsePrimaryExpr(Token* token, std::size_t length,
                                    std::size_t& index);

 private:
  static bool IsDecl(Token* token, std::size_t length, std::size_t index);
  static bool IsFuncDecl(Token* token, std::size_t length, std::size_t index);
  static bool IsClassDecl(Token* token, std::size_t length, std::size_t index);
  static StmtNode* ParseStmt(Token* token, std::size_t length,
                             std::size_t& index);
  static VarDeclNode* ParseVarDecl(Token* token, std::size_t length,
                                   std::size_t& index);
  static FuncDeclNode* ParseFuncDecl(Token* token, std::size_t length,
                                     std::size_t& index);
  static ClassDeclNode* ParseClassDecl(Token* token, std::size_t length,
                                       std::size_t& index);
  static StaticNode* ParseStatic(Token* token, std::size_t length,
                                 std::size_t& index);
  static ExprNode* ParseBinaryExpr(Token* token, std::size_t length,
                                   std::size_t& index, ExprNode* left,
                                   unsigned int priority);
  static ExprNode* ParseBinaryExprWithoutComma(Token* token, std::size_t length,
                                               std::size_t& index,
                                               ExprNode* left,
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

  operator std::string() override {
    if (this->GetSubType()->GetType() == TypeType::kPointer ||
        this->GetSubType()->GetType() == TypeType::kArray) {
      /*std::cout << std::string(*this->GetSubType()) + std::string(" const")
                << std::endl;*/
      return std::string(*this->GetSubType()) + std::string(" const");
    } else {
      /*std::cout << std::string("const ") + std::string(*this->GetSubType())
                << std::endl;*/
      return std::string("const ") + std::string(*this->GetSubType());
    }
  }

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

  operator std::string() override { return std::string(*type_data_) + "*"; }

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

  operator std::string() override {
    // return std::string(*type_data_) + "[" + std::string(*size_) + "]";
    return std::string(*type_data_) + "*";
  }

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

  operator std::string() override { return std::string(*type_data_) + "&"; }

  ReferenceType(const ReferenceType&) = default;
  ReferenceType& operator=(const ReferenceType&) = default;
};

class ClassType : public Type {
 public:
  ClassType() { type_ = TypeType::kClass; }
  void SetSubType(std::string class_name) {
    // type_ = TypeType::kClass;
    class_name_ = class_name;
  }
  virtual ~ClassType() = default;

  std::string GetClassName() { return class_name_; }

  std::vector<std::string>& GetNames() { return names_; }

  operator std::string() override { return class_name_; }

  ClassType(const ClassType&) = default;
  ClassType& operator=(const ClassType&) = default;

 private:
  std::string class_name_;
  std::vector<std::string> names_;
};

Type::operator std::string() {
  if (this->GetType() == TypeType::kBase) {
    switch (this->GetBaseType()) {
      case BaseType::kVoid:
        return "void";
      case BaseType::kBool:
        return "bool";
      case BaseType::kChar:
        return "char";
      case BaseType::kShort:
        return "short";
      case BaseType::kInt:
        return "int";
      case BaseType::kLong:
        return "int64_t";
      case BaseType::kFloat:
        return "float";
      case BaseType::kDouble:
        return "double";
      case BaseType::kString:
        return "string";
      case BaseType::kClass:
        return "class";
      case BaseType::kStruct:
        return "struct";
      case BaseType::kUnion:
        return "union";
      case BaseType::kEnum:
        return "enum";
      case BaseType::kPointer:
        return "*";
      case BaseType::kArray:
        return "[]";
      case BaseType::kFunction:
        return "()";
      case BaseType::kTypedef:
        return "typedef";
      case BaseType::kAuto:
        return "auto";
      default:
        EXIT_COMPILER("Type::operator std::string()", "Unknown base type.");
    }
  } else if (this->GetType() == TypeType::kConst) {
    return *dynamic_cast<ConstType*>(this);
  } else if (this->GetType() == TypeType::kPointer) {
    return *dynamic_cast<PointerType*>(this);
  } else if (this->GetType() == TypeType::kArray) {
    return *dynamic_cast<ArrayType*>(this);
  } else if (this->GetType() == TypeType::kReference) {
    return *dynamic_cast<ReferenceType*>(this);
  } else if (this->GetType() == TypeType::kClass) {
    return *dynamic_cast<ClassType*>(this);
  } else {
    EXIT_COMPILER("Type::operator std::string()", "Unknown type.");
  }
  return std::string();
}

Type* ValueNode::GetValueType() {
  if (value_.type == Token::Type::CHARACTER) {
    Type* type = new Type();
    type->SetType(Type::BaseType::kChar);
    return type;
  }
  if (value_.type == Token::Type::STRING) {
    Type* type = new Type();
    type->SetType(Type::BaseType::kChar);
    ConstType* const_type = new ConstType();
    const_type->SetSubType(type);
    PointerType* pointer_type = new PointerType();
    pointer_type->SetSubType(const_type);
    return pointer_type;
  }

  std::string str(value_.value.number.location, value_.value.number.length);
  /*try {
    std::size_t pos;
    (void)std::stoi(str, &pos);
    if (pos == str.size()) {
      Type* type = new Type();
      type->SetType(Type::BaseType::kInt);
      return type;
    }
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }*/

  try {
    std::size_t pos;
    (void)std::stoll(str, &pos);
    if (pos == str.size()) {
      Type* type = new Type();
      type->SetType(Type::BaseType::kLong);
      return type;
    }
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }

  try {
    std::size_t pos;
    (void)std::stoull(str, &pos);
    if (pos == str.size()) {
      Type* type = new Type();
      // TODO(Uint64t, etc.)
      type->SetType(Type::BaseType::kLong);
      return type;
    }
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }

  /*try {
    std::size_t pos;
    (void)std::stof(str, &pos);
    if (pos == str.size()) {
      Type* type = new Type();
      type->SetType(Type::BaseType::kFloat);
      return type;
    }
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }*/

  try {
    std::size_t pos;
    (void)std::stod(str, &pos);
    if (pos == str.size()) {
      Type* type = new Type();
      type->SetType(Type::BaseType::kDouble);
      return type;
    }
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }

  return nullptr;
}

Type* Type::CreateType(Token* token, std::size_t length, std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  Type* type = nullptr;

  bool is_read_base_type = false;

  while (index < length) {
    if (token[index].type == Token::Type::KEYWORD) {
      is_read_base_type = true;
      switch (token[index].value.keyword) {
        case Token::KeywordType::Const: {
          ConstType* const_type = new ConstType();
          if (const_type == nullptr)
            EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                          "const_type is nullptr.");

          if (index + 1 < length &&
              token[index + 1].type == Token::Type::KEYWORD) {
            index++;
            type = new Type();
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

              case Token::KeywordType::String:
                type->SetType(Type::BaseType::kString);
                break;

              case Token::KeywordType::Var:
              case Token::KeywordType::Auto:
                type->SetType(Type::BaseType::kAuto);
                break;

              case Token::KeywordType::Struct:
                type->SetType(Type::BaseType::kStruct);
                break;

              case Token::KeywordType::Class:
                type->SetType(Type::BaseType::kClass);
                break;

              case Token::KeywordType::Func:
                type->SetType(Type::BaseType::kFunction);
                break;

              default:
                break;
            }
          } else if (index + 1 < length &&
                     token[index + 1].type == Token::Type::IDENTIFIER) {
            index++;
            type = new ClassType();
            dynamic_cast<ClassType*>(type)->SetSubType(
                std::string(token[index].value.identifier.location,
                            token[index].value.identifier.length));

            dynamic_cast<ClassType*>(type)->GetNames().push_back(
                std::string(token[index].value.identifier.location,
                            token[index].value.identifier.length));

            while (index + 1 < length &&
                   token[index + 1].type == Token::Type::OPERATOR &&
                   token[index + 1].value._operator ==
                       Token::OperatorType::period) {
              index++;
              if (token[index].type == Token::Type::IDENTIFIER)
                EXIT_COMPILER(
                    "Type::CreateType(Token*,std::size_t,std::size_t&)",
                    "Unexpected scope name.");
              dynamic_cast<ClassType*>(type)->SetSubType(
                  dynamic_cast<ClassType*>(type)->GetClassName() + "." +
                  std::string(token[index].value.identifier.location,
                              token[index].value.identifier.length));
              dynamic_cast<ClassType*>(type)->GetNames().push_back(
                  std::string(token[index].value.identifier.location,
                              token[index].value.identifier.length));
            }
          }
          if (type == nullptr)
            EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                          "type is nullptr.");

          // std::cout << "ConstType" << std::endl;
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

        case Token::KeywordType::String:
          type = new Type();
          type->SetType(Type::BaseType::kString);
          break;

        case Token::KeywordType::Class:
          type = new Type();
          type->SetType(Type::BaseType::kClass);
          return type;

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

        case Token::KeywordType::Var:
        case Token::KeywordType::Auto:
          type = new Type();
          type->SetType(Type::BaseType::kAuto);
          break;

        case Token::KeywordType::Func:
          type = new Type();
          type->SetType(Type::BaseType::kFunction);
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
      if (!is_read_base_type) {
        type = new ClassType();
        is_read_base_type = true;
        dynamic_cast<ClassType*>(type)->SetSubType(
            std::string(token[index].value.identifier.location,
                        token[index].value.identifier.length));
        while (index + 1 < length &&
               token[index + 1].type == Token::Type::OPERATOR &&
               token[index + 1].value._operator ==
                   Token::OperatorType::period) {
          index += 2;
          if (token[index].type != Token::Type::IDENTIFIER)
            EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                          "Unexpected scope name.");
          dynamic_cast<ClassType*>(type)->SetSubType(
              dynamic_cast<ClassType*>(type)->GetClassName() + "." +
              std::string(token[index].value.identifier.location,
                          token[index].value.identifier.length));
        }
      } else {
        std::size_t index_temp = index;
        ExprNode* temp_expr =
            Parser::ParsePrimaryExpr(token, length, index_temp);
        if (temp_expr == nullptr)
          EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                        "ParsePrimaryExpr return nullptr.");

        if (temp_expr->GetType() == StmtNode::StmtType::kArray) {
          ArrayType* array_type = new ArrayType();
          array_type->SetSubType(
              type, dynamic_cast<ArrayNode*>(temp_expr)->GetIndex());
          type = array_type;
        }

        // TODO: Add support of custom types.

        return type;
      }
    }
    index++;
  }

  EXIT_COMPILER("Type::CreateType(Token*,std::size_t,std::size_t&)",
                "index is out of range.");
  return nullptr;
}

Type* Type::CreateDoubleType() {
  TRACE_FUNCTION;
  Type* type = new Type();
  type->SetType(Type::BaseType::kDouble);
  return type;
}

std::vector<uint8_t> Type::GetVmType() {
  Type* type = this;
  std::vector<uint8_t> vm_type;
  if (type->GetType() == Type::TypeType::NONE)
    EXIT_COMPILER("Type::GetVmType()", "Unexpected code.");

  bool is_end = false;
  while (!is_end) {
    if (type->GetType() == Type::TypeType::kBase) {
      switch (type->GetBaseType()) {
        case Type::BaseType::kAuto:
        case Type::BaseType::kVoid:
          vm_type.push_back(0x00);
          is_end = true;
          break;
        case Type::BaseType::kBool:
        case Type::BaseType::kChar:
          vm_type.push_back(0x01);
          is_end = true;
          break;
        case Type::BaseType::kShort:
        case Type::BaseType::kInt:
        case Type::BaseType::kLong:
          vm_type.push_back(0x02);
          is_end = true;
          break;
        case Type::BaseType::kFloat:
        case Type::BaseType::kDouble:
          vm_type.push_back(0x03);
          is_end = true;
          break;

          // TODO(uint64_t)

        case Type::BaseType::kString:
          vm_type.push_back(0x05);
          is_end = true;
          break;
        case Type::BaseType::kClass:
        case Type::BaseType::kStruct:
        case Type::BaseType::kUnion:
        case Type::BaseType::kEnum:
        case Type::BaseType::kPointer:
        case Type::BaseType::kArray:
        case Type::BaseType::kFunction:
        case Type::BaseType::kTypedef:
          // TODO
          EXIT_COMPILER("Type::GetVmType()", "Unsupported type.");
          break;
        default:
          EXIT_COMPILER("Type::GetVmType()", "Unexpected code.");
          break;
      }
    } else if (type->GetType() == Type::TypeType::kArray) {
      vm_type.push_back(0x06);
      type = dynamic_cast<ArrayType*>(type)->GetSubType();
    } else if (type->GetType() == Type::TypeType::kReference) {
      vm_type.push_back(0x07);
      type = dynamic_cast<ReferenceType*>(type)->GetSubType();
    } else if (type->GetType() == Type::TypeType::kConst) {
      vm_type.push_back(0x08);
      type = dynamic_cast<ConstType*>(type)->GetSubType();
    } else if (type->GetType() == Type::TypeType::kClass) {
      vm_type.push_back(0x09);
      is_end = true;
    }
  }

  /*std::vector<uint8_t> return_type;
  for (int64_t i = vm_type.size() - 1; i >= 0; i--) {
    return_type.push_back(vm_type[i]);
  }

  return return_type;*/
  return vm_type;
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

  // std::cout << token.size() << std::endl;

  // Delete the last NONE token.
  if (token_ptr[token.size() - 1].type == Token::Type::NONE) token.pop_back();

  while (index < token.size()) {
    if (IsDecl(token_ptr, length, index)) {
      if (IsFuncDecl(token_ptr, length, index)) {
        stmts.push_back(ParseFuncDecl(token_ptr, length, index));
      } else if (IsClassDecl(token_ptr, length, index)) {
        stmts.push_back(ParseClassDecl(token_ptr, length, index));
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
      stmts.push_back(ParseStmt(token_ptr, length, index));
      // EXIT_COMPILER("Parser::Parse(std::vector<Token>)", "Unexpected code.");
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
        token[index].value.keyword == Token::KeywordType::Var ||
        token[index].value.keyword == Token::KeywordType::Func ||
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
  } else {
    while (index < length) {
      if (token[index].type == Token::Type::IDENTIFIER &&
          token[index + 1].type == Token::Type::OPERATOR &&
          token[index + 1].value._operator == Token::OperatorType::period) {
        index += 2;
      } else {
        break;
      }
    }
    if ((token[index].type == Token::Type::IDENTIFIER &&
         token[index + 1].type == Token::Type::IDENTIFIER) ||
        (token[index].type == Token::Type::IDENTIFIER &&
         token[index + 1].type == Token::Type::OPERATOR &&
         (token[index + 1].value._operator == Token::OperatorType::star ||
          token[index + 1].value._operator == Token::OperatorType::amp ||
          token[index + 1].value._operator == Token::OperatorType::ampamp) &&
         token[index + 2].type == Token::Type::IDENTIFIER)) {
      // TODO: Change the processing logic of custom types and add support of
      // custom types.
      return true;
    }
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
    if (token[i].type == Token::Type::IDENTIFIER &&
        token[i + 1].type == Token::Type::OPERATOR &&
        token[i + 1].value._operator == Token::OperatorType::l_paren) {
      return true;
    }
    /*if (token[i].type == Token::Type::OPERATOR &&
        token[i].value._operator != Token::OperatorType::coloncolon &&
        token[i].value._operator != Token::OperatorType::period &&
        token[i].value._operator != Token::OperatorType::arrow) {
      return false;
    }*/
    if (token[i].type == Token::Type::OPERATOR &&
        token[i].value._operator != Token::OperatorType::period) {
      return false;
    }
  }
  return false;
}

bool Parser::IsClassDecl(Token* token, std::size_t length, std::size_t index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::IsClassDecl(Token*,std::size_t,std::size_t)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::IsClassDecl(Token*,std::size_t,std::size_t)",
                  "index is out of range.");

  if (token[index].type == Token::Type::KEYWORD &&
      (token[index].value.keyword == Token::KeywordType::Class ||
       token[index].value.keyword == Token::KeywordType::Struct)) {
    return true;
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

        case Token::KeywordType::Do: {
          index++;
          DowhileNode* result = new DowhileNode();

          StmtNode* stmt_ = ParseStmt(token, length, index);
          if (token[index].type != Token::Type::KEYWORD ||
              token[index].value.keyword != Token::KeywordType::While)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Do-while while keyword not found.");
          index++;
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Do-while condition l_paren not found.");
          ExprNode* condition = ParseExpr(token, length, ++index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Do-while condition r_paren not found.");

          index++;
          result->SetDowhileNode(condition, stmt_);
          return result;
        }

        case Token::KeywordType::For: {
          index++;
          ForNode* result = new ForNode();

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "For start l_paren not found.");
          ExprNode* start = nullptr;
          if (token[index + 1].type != Token::Type::OPERATOR ||
              token[index + 1].value._operator != Token::OperatorType::semi)
            start = ParseExpr(token, length, ++index);

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::semi)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "For start semi not found.");
          ExprNode* condition = nullptr;
          if (token[index + 1].type != Token::Type::OPERATOR ||
              token[index + 1].value._operator != Token::OperatorType::semi)
            condition = ParseExpr(token, length, ++index);

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::semi)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "For condition semi not found.");
          ExprNode* end = nullptr;
          if (token[index + 1].type != Token::Type::OPERATOR ||
              token[index + 1].value._operator != Token::OperatorType::semi)
            end = ParseExpr(token, length, ++index);

          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "For end r_paren not found.");

          result->SetForNode(start, condition, end,
                             ParseStmt(token, length, ++index));
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

        case Token::KeywordType::Switch: {
          index++;
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::l_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "switch l_paren not found.");
          ExprNode* expr = ParseExpr(token, length, ++index);
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::r_paren)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "switch r_paren not found.");
          index++;
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::l_brace)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "switch l_brace not found.");
          index++;

          SwitchNode* result = new SwitchNode();
          result->SetSwitchNode(expr, ParseStmt(token, length, index));

          return result;
        }

        case Token::KeywordType::Case: {
          index++;
          std::size_t temp_index = index;
          CaseNode* result = new CaseNode();
          ExprNode* expr = ParseExpr(token, length, temp_index);
          if (expr == nullptr)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "case expr is nullptr.");
          if (token[temp_index].type == Token::Type::OPERATOR &&
              token[temp_index].value._operator == Token::OperatorType::colon)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "case colon not found.");
          temp_index++;
          bool is_end = false;
          std::vector<StmtNode*> stmts;
          do {
            if (token[temp_index].type == Token::Type::OPERATOR &&
                token[temp_index].value._operator ==
                    Token::OperatorType::r_brace) {
              is_end = true;
              break;
            }
            if (token[temp_index].type == Token::Type::KEYWORD &&
                (token[temp_index].value.keyword == Token::KeywordType::Case ||
                 token[temp_index].value.keyword ==
                     Token::KeywordType::Default)) {
              is_end = true;
              break;
            }
            stmts.push_back(ParseStmt(token, length, temp_index));
            if (stmts.back() == nullptr)
              EXIT_COMPILER(
                  "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "case stmt is nullptr.");
            if (stmts.back()->GetType() == StmtNode::StmtType::kCase) {
              is_end = true;
              stmts.pop_back();
            }
          } while (is_end);
          index = temp_index;
          result->SetCaseNode(expr, stmts);
        }

        case Token::KeywordType::Default: {
          index++;
          CaseNode* result = new CaseNode();
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::colon)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "default colon not found.");
          index++;
          std::vector<StmtNode*> stmts;
          while (token[index].type != Token::Type::KEYWORD &&
                 token[index].value.keyword != Token::KeywordType::Case &&
                 token[index].value.keyword != Token::KeywordType::Default) {
            stmts.push_back(ParseStmt(token, length, index));
            if (stmts.back() == nullptr)
              EXIT_COMPILER(
                  "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "default stmt is nullptr.");
          }
          result->SetCaseNode(nullptr, stmts);
          return result;
        }

        case Token::KeywordType::Break:
          index++;
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::semi)
            index++;
          return new BreakNode();

        case Token::KeywordType::Return: {
          index++;
          ReturnNode* result = new ReturnNode();

          if (token[index].type == Token::Type::OPERATOR ||
              token[index].value._operator == Token::OperatorType::semi) {
            result->SetReturnNode(nullptr);
            index++;
            return result;
          }

          ExprNode* return_expr = ParseExpr(token, length, index);
          if (return_expr == nullptr)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "return_expr is nullptr.");
          result->SetReturnNode(return_expr);
          index++;
          return result;
        }

        case Token::KeywordType::From: {
          index++;
          if (token[index].type != Token::Type::STRING)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unsupported import location.");
          std::string import_location(*token[index].value.string);
          index++;
          if (token[index].type != Token::Type::KEYWORD ||
              token[index].value.keyword != Token::KeywordType::Import)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unexpected import behavior.");
          index++;
          std::vector<std::string> import_list;
          if (token[index].type != Token::Type::IDENTIFIER)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unexpected import identifier.");
          import_list.push_back(
              std::string(token[index].value.identifier.location,
                          token[index].value.identifier.length));
          index++;
          while (token[index].type == Token::Type::OPERATOR &&
                 token[index].value._operator == Token::OperatorType::comma) {
            index++;
            if (token[index].type != Token::Type::IDENTIFIER)
              EXIT_COMPILER(
                  "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "Unexpected import identifier.");
            import_list.push_back(
                std::string(token[index].value.identifier.location,
                            token[index].value.identifier.length));
            index++;
          }

          std::vector<std::string> alias_list;

          if (token[index].type == Token::Type::KEYWORD &&
              token[index].value.keyword == Token::KeywordType::As) {
            index++;
            if (token[index].type != Token::Type::IDENTIFIER)
              EXIT_COMPILER(
                  "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                  "Unexpected import alias.");
            alias_list.push_back(
                std::string(token[index].value.identifier.location,
                            token[index].value.identifier.length));
            index++;
            while (token[index].type == Token::Type::OPERATOR &&
                   token[index].value._operator == Token::OperatorType::comma) {
              index++;
              if (token[index].type != Token::Type::IDENTIFIER)
                EXIT_COMPILER(
                    "Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                    "Unexpected import alias.");
              alias_list.push_back(
                  std::string(token[index].value.identifier.location,
                              token[index].value.identifier.length));
              index++;
            }
          }

          ImportNode* result = new ImportNode();
          result->SetFromImport(import_location, import_list, alias_list);

          return result;
        }

        case Token::KeywordType::Import: {
          index++;
          if (token[index].type != Token::Type::STRING)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unsupported import location.");
          std::string import_location(*token[index].value.string);
          index++;
          if (token[index].type != Token::Type::IDENTIFIER)
            EXIT_COMPILER("Parser::ParseStmt(Token*,std::size_t,std::size_t&)",
                          "Unexpected import behavior.");
          std::string name(token[index].value.identifier.location,
                           token[index].value.identifier.length);
          ImportNode* result = new ImportNode();
          result->SetImportNode(import_location, name);
          index++;
          return result;
        }

          /*case Token::KeywordType::Static:
            index++;
            return ParseStatic(token, length, index);*/

        default:
          return nullptr;
      }

    default:
      if (token[index].type == Token::Type::IDENTIFIER &&
          token[index + 1].type == Token::Type::OPERATOR &&
          token[index + 1].value._operator == Token::OperatorType::colon) {
        // std::cout << "Label Parse" << std::endl;
        LabelNode* result = new LabelNode();
        IdentifierNode identifier_node;
        identifier_node.SetIdentifierNode(token[index]);
        result->SetLabelNode(identifier_node);
        index += 2;
        return result;
      }
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

  if (token[index].type == Token::Type::OPERATOR &&
      token[index].value._operator == Token::OperatorType::semi) {
    func_decl = new FuncDeclNode();
    func_decl->SetFuncDeclNode(type, dynamic_cast<FuncNode*>(stat), nullptr);
    index++;
    return func_decl;
  }

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

ClassDeclNode* Parser::ParseClassDecl(Token* token, std::size_t length,
                                      std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");
  if (token[index].type != Token::Type::KEYWORD ||
      (token[index].value.keyword != Token::KeywordType::Class &&
       token[index].value.keyword != Token::KeywordType::Struct))
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "Class or Struct not found.");

  index++;

  ClassDeclNode* class_decl = new ClassDeclNode();
  if (class_decl == nullptr)
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "class_decl is nullptr.");
  ExprNode* name = Parser::ParsePrimaryExpr(token, length, index);
  if (name == nullptr || name->GetType() != StmtNode::StmtType::kIdentifier)
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "name is not an identifier.");

  if (token[index].type != Token::Type::OPERATOR ||
      token[index].value._operator != Token::OperatorType::l_brace)
    EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                  "l_brace not found.");

  index++;

  std::vector<StaticNode*> static_decls;
  std::vector<VarDeclNode*> var_decls;
  std::vector<FuncDeclNode*> func_decls;
  std::vector<ClassDeclNode*> class_decls;

  while (index < length &&
         (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::r_brace)) {
    if (IsDecl(token, length, index)) {
      if (IsFuncDecl(token, length, index)) {
        func_decls.push_back(ParseFuncDecl(token, length, index));
      } else if (IsClassDecl(token, length, index)) {
        class_decls.push_back(ParseClassDecl(token, length, index));
      } else {
        var_decls.push_back(
            dynamic_cast<VarDeclNode*>(ParseVarDecl(token, length, index)));
        if (token[index].value._operator != Token::OperatorType::semi)
          EXIT_COMPILER(
              "Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
              "semi not found.");
        index++;
      }
    } else if (token[index].type == Token::Type::KEYWORD &&
               token[index].value.keyword == Token::KeywordType::Static) {
      static_decls.push_back(ParseStatic(token, length, index));
    } else {
      EXIT_COMPILER("Parser::ParseClassDecl(Token*,std::size_t,std::size_t&)",
                    "Unexpected code.");
    }
  }
  index++;

  class_decl->SetClassDeclNode(*dynamic_cast<IdentifierNode*>(name),
                               static_decls, var_decls, func_decls,
                               class_decls);
  return class_decl;
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

  Type* type = Type::CreateType(token, length, index);
  if (type == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "type is nullptr.");
  ExprNode* name = ParsePrimaryExpr(token, length, index);
  if (name == nullptr)
    EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                  "name is nullptr.");

  /*switch (token[index].value._operator) {
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
      // std::cout << "VarDecl has EQUAL." << std::endl;
      ExprNode* value = ParseExpr(token, length, ++index);
      var_decl->SetVarDeclNode(type, name, value);
      break;
    }

    default:
      return var_decl;
  }*/

  if (name->GetType() == StmtNode::StmtType::kArray) {
    // std::cout << "Array" << std::endl;
    ArrayDeclNode* array_decl = new ArrayDeclNode();
    if (array_decl == nullptr)
      EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                    "array_decl is nullptr.");

    ArrayNode* array = dynamic_cast<ArrayNode*>(name);
    array_decl->SetArrayDeclNode(type, array->GetExpr(), array->GetIndex());
    if (token[index].value._operator == Token::OperatorType::equal) {
      index++;
      if (token[index].type == Token::Type::OPERATOR &&
          token[index].value._operator == Token::OperatorType::l_brace) {
        std::vector<ExprNode*> values;
        while (true) {
          // Skip the l_brace or comma.
          values.push_back(ParseExprWithoutComma(token, length, ++index));
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::r_brace) {
            index++;
            break;
          }
        }
        array_decl->SetArrayDeclNode(type, array->GetExpr(), array->GetIndex(),
                                     values);
      } else {
        EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                      "l_brace not found.");
      }
    }
    return array_decl;
  } else {
    // std::cout << "Var" << std::endl;
    VarDeclNode* var_decl = new VarDeclNode();
    if (var_decl == nullptr)
      EXIT_COMPILER("Parser::ParseVarDecl(Token*,std::size_t,std::size_t&)",
                    "var_decl is nullptr.");

    var_decl->SetVarDeclNode(type, name);
    if (token[index].value._operator == Token::OperatorType::equal) {
      index++;
      ExprNode* value = ParseExprWithoutComma(token, length, index);
      var_decl->SetVarDeclNode(type, name, value);
    }
    return var_decl;
  }

  return nullptr;
}

StaticNode* Parser::ParseStatic(Token* token, std::size_t length,
                                std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                  "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                  "index is out of range.");

  if (token[index].type == Token::Type::KEYWORD &&
      token[index].value.keyword == Token::KeywordType::Static) {
    index++;
  } else {
    EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                  "Unexpected keyword.");
  }

  StaticNode* static_node = new StaticNode();

  if (IsDecl(token, length, index)) {
    if (IsFuncDecl(token, length, index)) {
      static_node->SetDecl(ParseFuncDecl(token, length, index));
    } else if (IsClassDecl(token, length, index)) {
      EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                    "Keyword static unsupprot class.");
      // static_node->SetDecl(ParseClassDecl(token, length, index));
    } else {
      static_node->SetDecl(
          dynamic_cast<DeclNode*>(ParseVarDecl(token, length, index)));
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::semi) {
        EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                      "not found semi.");
        return nullptr;
      }
      index++;
    }
  } else {
    EXIT_COMPILER("Parser::ParseStatic(Token*,std::size_t,std::size_t&)",
                  "Unexpected code.");
  }
  return static_node;
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
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        amp_node);
              preoper_expr = amp_node;
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
              preoper_expr = star_node;
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
              preoper_expr = plus_node;
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
              preoper_expr = minus_node;
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
              preoper_expr = not_node;
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
              preoper_expr = bitwisenot_node;
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
          if (token[index + 1].type == Token::Type::OPERATOR &&
              token[index + 1].value._operator == Token::OperatorType::period) {
            // continue
            index++;
          } else {
            state = State::kEnd;
          }
          break;
        case Token::OperatorType::l_paren:  // (
          if (state == State::kPreOper) {
            index++;
            if (full_expr == nullptr || preoper_expr == nullptr) {
              /*if (token[index].type == Token::Type::KEYWORD) {
                full_expr = new CastNode();
                dynamic_cast<CastNode*>(full_expr)->SetCastNode(
                    Type::CreateType(token, length, index), nullptr);
              } else {
                full_expr = ParseExpr(token, length, index);
                state = State::kPostOper;
              }*/

              full_expr = ParseExpr(token, length, index);
              state = State::kPostOper;
              preoper_expr = full_expr;
            } else {
              /*if (token[index].type == Token::Type::KEYWORD) {
                CastNode* convert_node = new CastNode();
                convert_node->SetCastNode(
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
              }*/

              ExprNode* full_expr_node = ParseExpr(token, length, index);
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        full_expr_node);
              state = State::kPostOper;
            }
            index++;
          } else if (state == State::kPostOper &&
                     token[index - 1].type == Token::Type::IDENTIFIER) {
            std::vector<ExprNode*> args;
            index++;
            bool va_flag = false;
            while (index < length && token[index].value._operator !=
                                         Token::OperatorType::r_paren) {
              if (token[index].type == Token::Type::OPERATOR &&
                  token[index].value._operator ==
                      Token::OperatorType::ellipsis &&
                  token[index + 1].type == Token::Type::OPERATOR &&
                  token[index + 1].value._operator ==
                      Token::OperatorType::r_paren) {
                va_flag = true;
                index++;
                break;
              }
              args.push_back(ParseExprWithoutComma(token, length, index));
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
            if (va_flag) func_node->EnableVaFlag();
            if (main_expr != nullptr &&
                main_expr->GetType() == StmtNode::StmtType::kBinary) {
              func_node->SetFuncNode(
                  dynamic_cast<BinaryNode*>(main_expr)->GetRightExpr(), args);
              dynamic_cast<BinaryNode*>(main_expr)->SetBinaryNode(
                  dynamic_cast<BinaryNode*>(main_expr)->GetOperator(),
                  dynamic_cast<BinaryNode*>(main_expr)->GetLeftExpr(),
                  func_node);

            } else {
              func_node->SetFuncNode(main_expr, args);
              if (full_expr == nullptr || preoper_expr == nullptr) {
                full_expr = main_expr = func_node;
              } else {
                if (preoper_expr != nullptr) {
                  UnaryNode* unary_node =
                      dynamic_cast<UnaryNode*>(preoper_expr);
                  if (unary_node == nullptr) {
                    return nullptr;
                  }
                  unary_node->SetUnaryNode(unary_node->GetOperator(),
                                           func_node);
                }
                main_expr = func_node;
              }
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
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPreInc, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              preoper_expr = full_expr = preinc_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        preinc_node);
              preoper_expr = preinc_node;
            }
          } else {
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc, full_expr);
            full_expr = preinc_node;
          }
          index++;
          break;
        }
        case Token::OperatorType::minusminus: {  // --
          UnaryNode* preinc_node = new UnaryNode();
          if (state == State::kPreOper) {
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPreInc, nullptr);
            if (full_expr == nullptr || preoper_expr == nullptr) {
              preoper_expr = full_expr = preinc_node;
            } else {
              if (preoper_expr != nullptr)
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        preinc_node);
              preoper_expr = preinc_node;
            }
          } else {
            preinc_node->SetUnaryNode(UnaryNode::Operator::kPostInc, full_expr);
            full_expr = preinc_node;
          }
          index++;
          break;
        }

        /*case Token::OperatorType::coloncolon:  // ::
          if (state == State::kPreOper) {
            if (main_expr->GetType() == StmtNode::StmtType::kIdentifier) {
              index++;
              if (token[index].type != Token::Type::IDENTIFIER)
                EXIT_COMPILER(
                    "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                    "Token isn't IDENTIFIER type.");
              Token& name_token =
                  dynamic_cast<IdentifierNode*>(main_expr)->GetNameToken();
              if (name_token.type != Token::Type::IDENTIFIER)
                EXIT_COMPILER(
                    "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                    "Name token isn't IDENTIFIER type.");

              name_token.value.identifier.length +=
                  token[index].value.identifier.length + 2;
              if (token[index + 1].type != Token::Type::OPERATOR ||
                  (token[index + 1].value._operator !=
                       Token::OperatorType::coloncolon &&
                   token[index + 1].value._operator !=
                       Token::OperatorType::arrow &&
                   token[index + 1].value._operator !=
                       Token::OperatorType::periodstar &&
                   token[index + 1].value._operator !=
                       Token::OperatorType::arrowstar))
                state = State::kPostOper;
              index++;
            } else {
              EXIT_COMPILER(
                  "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                  "Before coloncolon isn't identifier node.");
            }
          } else {
            EXIT_COMPILER(
                "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "Before coloncolon isn't identifier node.");
          }
          break;*/
        case Token::OperatorType::period: {  // .
                                             // if (state == State::kPreOper) {
          BinaryNode* binary_node = new BinaryNode();
          index++;
          IdentifierNode* identifier_node = new IdentifierNode();
          identifier_node->SetIdentifierNode(token[index]);
          index++;
          binary_node->SetBinaryNode(BinaryNode::Operator::kMember, main_expr,
                                     identifier_node);
          if (full_expr == main_expr) {
            // std::cout << "Point B" << std::endl;
            full_expr = main_expr = binary_node;
          } else {
            if (preoper_expr != nullptr) {
              dynamic_cast<UnaryNode*>(preoper_expr)
                  ->SetUnaryNode(
                      dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                      binary_node);
              preoper_expr = main_expr = binary_node;
            } else {
              EXIT_COMPILER(
                  "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                  "Before period isn't main node.");
            }
          }
          if (token[index].type != Token::Type::OPERATOR ||

              token[index].value._operator != Token::OperatorType::period)
            state = State::kPostOper;
          /*} else {
            EXIT_COMPILER(
                "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "Before period isn't pre_oper node.");
          }*/
          break;
        }

        /*case Token::OperatorType::ellipsis:
          if(state!=State::kPreOper)EXIT_COMPILER("Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "ellipsis isn't pre_oper node.");
          if(main_expr==NULL||main_expr->GetType()!=StmtNode::StmtType::kFunc)EXIT_COMPILER("Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "main expr isn't func node.");
          if(token[index+1].type!=Token::Type::OPERATOR||token[index+1].value._operator!=Token::OperatorType::r_paren)EXIT_COMPILER("Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "Next token isn't r_paren.");
          dynamic_cast<FuncNode*>(main_expr)->EnableVaFlag();
          index++;*/
        /*case Token::OperatorType::arrow:  // ->
          if (state == State::kPreOper) {
            BinaryNode* binary_node = new BinaryNode();
            index++;
            IdentifierNode* identifier_node = new IdentifierNode();
            identifier_node->SetIdentifierNode(token[index]);
            index++;
            binary_node->SetBinaryNode(BinaryNode::Operator::kArrow, main_expr,
                                       identifier_node);

            if (full_expr == main_expr) {
              full_expr = main_expr = binary_node;
            } else {
              if (preoper_expr != nullptr) {
                dynamic_cast<UnaryNode*>(preoper_expr)
                    ->SetUnaryNode(
                        dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                        binary_node);
                preoper_expr = binary_node;
              } else {
                EXIT_COMPILER(
                    "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                    "Before period isn't main node.");
              }
            }
            if (token[index + 1].type != Token::Type::OPERATOR ||
                (token[index + 1].value._operator !=
                     Token::OperatorType::coloncolon &&
                 token[index + 1].value._operator !=
                     Token::OperatorType::arrow &&
                 token[index + 1].value._operator !=
                     Token::OperatorType::periodstar &&
                 token[index + 1].value._operator !=
                     Token::OperatorType::arrowstar &&
                 token[index + 1].value._operator !=
                     Token::OperatorType::period))
              state = State::kPostOper;
          } else {
            EXIT_COMPILER(
                "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
                "Before period isn't pre_oper node.");
          }
          break;*/
        // TODO(Parser): Advanced syntax awaits subsequent development.
        /*case Token::OperatorType::l_brace:  // {
          break;
        case Token::OperatorType::r_brace:  // }
          break;
        case Token::OperatorType::question:  // ?
          break;
        case Token::OperatorType::colon:  // :
          break;
        case Token::OperatorType::periodstar:  // .*
          break;
        case Token::OperatorType::arrowstar:  // ->*
          break;
        */
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
          (token[index + 1].value._operator != Token::OperatorType::period))
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
    } else if (token[index].type == Token::Type::KEYWORD) {
      switch (token[index].value.keyword) {
        case Token::KeywordType::True:
        case Token::KeywordType::False: {
          ValueNode* bool_node = new ValueNode();
          bool_node->SetValueNode(token[index]);
          if (full_expr == nullptr || preoper_expr == nullptr) {
            full_expr = main_expr = bool_node;
          } else {
            dynamic_cast<UnaryNode*>(preoper_expr)
                ->SetUnaryNode(
                    dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                    bool_node);
            main_expr = bool_node;
          }
          index++;
          state = State::kEnd;
          break;
        }
        default:
          EXIT_COMPILER(
              "Parser::ParsePrimaryExpr(Token*,std::size_t,std::size_t&)",
              "Unexpected keyword.");
      }
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

ExprNode* Parser::ParseExprWithoutComma(Token* token, std::size_t length,
                                        std::size_t& index) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER(
        "Parser::ParseExprWithoutComma(Token*,std::size_t,std::size_t&)",
        "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER(
        "Parser::ParseExprWithoutComma(Token*,std::size_t,std::size_t&)",
        "index is out of range.");

  if (IsDecl(token, length, index)) return ParseVarDecl(token, length, index);
  ExprNode* expr = ParsePrimaryExpr(token, length, index);
  if (expr == nullptr)
    EXIT_COMPILER(
        "Parser::ParseExprWithoutComma(Token*,std::size_t,std::size_t&)",
        "expr is nullptr.");
  expr = ParseBinaryExprWithoutComma(token, length, index, expr, 0);
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
        /*case Token::OperatorType::period: {
          BinaryNode* period_node = new BinaryNode();
          index++;
          period_node->SetBinaryNode(
              BinaryNode::Operator::kMember, expr,
              ParseBinaryExpr(token, length, index,
                              ParsePrimaryExpr(token, length, index), 14));
          expr = period_node;
          break;
        }
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
        }*/

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
        // std::cout << "plus BinaryNode" << std::endl;
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

ExprNode* Parser::ParseBinaryExprWithoutComma(Token* token, std::size_t length,
                                              std::size_t& index,
                                              ExprNode* left,
                                              unsigned int priority) {
  TRACE_FUNCTION;
  if (token == nullptr)
    EXIT_COMPILER(
        "Parser::ParseBinaryExprWithoutComma(Token*,std::size_t,std::size_t&,"
        "ExprNode*,unsigned int)",
        "token is nullptr.");
  if (index >= length)
    EXIT_COMPILER(
        "Parser::ParseBinaryExprWithoutComma(Token*,std::size_t,std::size_t&,"
        "ExprNode*,unsigned int)",
        "index is out of range.");
  if (left == nullptr)
    EXIT_COMPILER(
        "Parser::ParseBinaryExprWithoutComma(Token*,std::size_t,std::size_t&,"
        "ExprNode*,unsigned int)",
        "left is nullptr.");

  ExprNode* expr = left;
  while (index < length && GetPriority(token[index]) > priority) {
    if (token[index].type != Token::Type::OPERATOR)
      EXIT_COMPILER(
          "Parser::ParseBinaryExprWithoutComma(Token*,std::size_t,std::size_t&,"
          "ExprNode*,unsigned int)",
          "Unexpected code.");
    switch (token[index].value._operator) {
        /*case Token::OperatorType::periodstar: {
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
        }*/

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

        /*case Token::OperatorType::comma: {
          BinaryNode* comma_node = new BinaryNode();
          index++;
          comma_node->SetBinaryNode(
              BinaryNode::Operator::kComma, expr,
              ParseBinaryExpr(token, length, index,
                              ParsePrimaryExpr(token, length, index), 1));
          expr = comma_node;
          break;
        }*/

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
      /*case Token::OperatorType::periodstar:
      case Token::OperatorType::arrowstar:
        return 14;*/
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

  void GenerateBytecode(CompoundNode* stmt, const char* output_file);

 private:
  /*class Memory {
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

    std::size_t GetSize() {
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

    int64_t SwapLong(int64_t x) {
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
      return (int64_t)ux;
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
  };*/

  class Bytecode {
   public:
    // Bytecode(std::size_t oper) { oper_ = oper; }
    Bytecode(std::size_t oper, std::size_t args_count, ...) {
      TRACE_FUNCTION;
      oper_ = oper;
      va_list args;
      va_start(args, args_count);
      for (std::size_t i = 0; i < args_count; i++) {
        arg_.push_back(va_arg(args, std::size_t));
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
    void SetArgs(std::size_t args_count, ...) {
      TRACE_FUNCTION;
      va_list args;
      va_start(args, args_count);
      for (std::size_t i = 0; i < args_count; i++) {
        arg_.push_back(va_arg(args, std::size_t));
      }
      va_end(args);
    }

   private:
    uint8_t oper_ = 0x00;
    std::vector<std::size_t> arg_;
  };

  class Function {
   public:
    Function(std::string name, std::vector<std::size_t> args,
             std::vector<Bytecode> code) {
      TRACE_FUNCTION;
      name_ = name;
      args_ = args;
      code_ = code;
    }
    ~Function() = default;

    std::string GetName() {
      TRACE_FUNCTION;
      return name_;
    }

    std::vector<std::size_t> GetArgs() {
      TRACE_FUNCTION;
      return args_;
    }

    std::vector<Bytecode> GetCode() {
      TRACE_FUNCTION;
      return code_;
    }

    void EnableVaFlag() { va_flag_ = true; }

    bool GetVaFlag() { return va_flag_; }

   private:
    std::string name_;
    std::vector<std::size_t> args_;
    std::vector<Bytecode> code_;
    bool va_flag_ = false;
  };

  class Memory {
   public:
    Memory() {
      TRACE_FUNCTION;
      uint16_t test_data = 0x0011;
      is_big_endian_ = *(uint8_t*)&test_data == 0x00;
    }
    ~Memory() = default;

    void SetCode(std::vector<Bytecode>* code) { code_ = code; }

    virtual std::size_t Add(std::size_t size) {
      // std::cout << "Add" << std::endl;
      TRACE_FUNCTION;
      std::size_t index = memory_size_;
      for (size_t i = 0; i < size; i++) {
        memory_type_.push_back(0x00);
        memory_size_++;
      }

      return index;
    }

    virtual std::size_t AddWithType(std::vector<uint8_t> type) {
      // std::cout << "Add" << std::endl;
      TRACE_FUNCTION;
      std::size_t index = memory_size_;
      /*std::cout<<"AddWithType: ";
      for(size_t i = 0;i<type.size();i++){
        printf("%02x ",type[i]);
      }
      std::cout<<std::endl;*/
      // memory_type_.insert(memory_type_.end(), type.begin(), type.end());
      for (std::size_t i = 0; i < type.size(); i++) {
        memory_type_.push_back(type[i]);
      }
      memory_size_++;

      return index;
    }

    virtual std::size_t AddByte(int8_t value) {
      // std::cout << "AddByte" << std::endl;
      TRACE_FUNCTION;
      const_table_.push_back(0x01);
      const_table_.push_back(value);
      const_table_size_++;

      memory_type_.push_back(0x01);
      memory_size_++;
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    virtual std::size_t AddLong(int64_t value) {
      // std::cout << "AddLong: " << value << std::endl;
      TRACE_FUNCTION;
      const_table_.push_back(0x02);
      // uint64_t uint64t_value = *reinterpret_cast<uint64_t*>(&value);
      // std::cout << "AddLong: " << value << std::endl;
      // uint64t_value = is_big_endian_ ? uint64t_value :
      // SwapUint64t(uint64t_value);
      value = is_big_endian_ ? value : SwapLong(value);
      // std::cout << "int and long sizeof: " << sizeof(long) <<" "<<
      // sizeof(int) << std::endl;
      for (int i = 0; i < 8; ++i) {
        const_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
      }
      const_table_size_++;

      memory_type_.push_back(0x02);
      memory_size_++;
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    virtual std::size_t AddDouble(double value) {
      // std::cout << "AddDouble" << std::endl;
      TRACE_FUNCTION;
      const_table_.push_back(0x03);
      value = is_big_endian_ ? value : SwapDouble(value);
      uint64_t int_value;
      std::memcpy(&int_value, &value, sizeof(double));
      for (int i = 0; i < 8; ++i) {
        const_table_.push_back(
            static_cast<uint8_t>((int_value >> (i * 8)) & 0xFF));
      }
      const_table_size_++;

      memory_type_.push_back(0x03);
      memory_size_++;
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    virtual std::size_t AddUint64t(uint64_t value) {
      // std::cout << "AddUint64t" << std::endl;
      TRACE_FUNCTION;
      const_table_.push_back(0x04);
      value = is_big_endian_ ? value : SwapUint64t(value);
      for (int i = 0; i < 8; ++i) {
        const_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
      }
      const_table_size_++;

      memory_type_.push_back(0x04);
      memory_size_++;
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    std::size_t AddUint64tWithoutValue(std::size_t& code) {
      TRACE_FUNCTION;
      memory_type_.push_back(0x04);
      memory_size_++;
      code_->push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1, 0));
      code = code_->size() - 1;
      return memory_size_ - 1;
    }

    void SetUint64tValue(std::size_t code, uint64_t value) {
      TRACE_FUNCTION;
      const_table_.push_back(0x04);
      value = is_big_endian_ ? value : SwapUint64t(value);
      for (int i = 0; i < 8; ++i) {
        const_table_.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
      }
      const_table_size_++;

      auto args = (*code_)[code].GetArgs();
      args[1] = const_table_size_ - 1;
      (*code_)[code].SetArgs(args);
    }

    virtual std::size_t AddString(std::string value) {
      // std::cout << "AddString" << std::endl;
      TRACE_FUNCTION;
      const_table_.push_back(0x05);
      EncodeUleb128(value.size() + 1, const_table_);
      for (std::size_t i = 0; i < value.size(); i++) {
        const_table_.push_back(value[i]);
      }
      const_table_.push_back(0x00);
      const_table_size_++;

      memory_type_.push_back(0x05);
      memory_size_++;
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    virtual std::size_t AddPtr(std::uintptr_t ptr, std::vector<uint8_t> type) {
      TRACE_FUNCTION;
      // std::cout << "AddPtr" << std::endl;
      const_table_.push_back(0x06);
      for (int i = 0; i < 8; ++i) {
        const_table_.push_back(static_cast<uint8_t>((ptr >> (i * 8)) & 0xFF));
      }
      const_table_size_++;

      memory_type_.push_back(0x06);
      memory_size_++;
      // memory_type_.insert(memory_type_.end(), type.begin(), type.end());
      for (std::size_t i = 0; i < type.size(); i++) {
        memory_type_.push_back(type[i]);
      }
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, memory_size_ - 1,
                                const_table_size_ - 1));
      return memory_size_ - 1;
    }

    /*std::vector<Bytecode>& GetCode() {
      TRACE_FUNCTION;
      return code_;
    }*/

    /*uint8_t GetType(size_t index) {
      if (index >= memory_size_)
        EXIT_COMPILER("BytecodeGenerator::Memory::GetType(size_t)",
                      "index is out of range.");
      return memory_type_[index];
    }*/

    std::vector<uint8_t>& GetMemoryType() {
      TRACE_FUNCTION;
      return memory_type_;
    }

    std::vector<uint8_t>& GetConstTable() {
      TRACE_FUNCTION;
      return const_table_;
    }

    std::size_t& GetConstTableSize() {
      TRACE_FUNCTION;
      // std::cout << "Const Table Size: " << const_table_size_ << std::endl;
      return const_table_size_;
    }

    std::size_t GetMemorySize() {
      TRACE_FUNCTION;
      return memory_size_;
    }

   protected:
    int64_t SwapLong(int64_t x) {
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
      return (int64_t)ux;
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
      // std::cout << "SwapUint64t: " << x << std::endl;
      x = ((x << 56) & 0xFF00000000000000ULL) |
          ((x << 40) & 0x00FF000000000000ULL) |
          ((x << 24) & 0x0000FF0000000000ULL) |
          ((x << 8) & 0x000000FF00000000ULL) |
          ((x >> 8) & 0x00000000FF000000ULL) |
          ((x >> 24) & 0x0000000000FF0000ULL) |
          ((x >> 40) & 0x000000000000FF00ULL) |
          ((x >> 56) & 0x00000000000000FFULL);
      // std::cout << "SwapUint64t: " << x << std::endl;
      return x;
    }

    bool is_big_endian_ = false;
    std::vector<Bytecode>* code_;
    std::vector<uint8_t> const_table_;
    std::size_t const_table_size_ = 0;
    std::vector<uint8_t> memory_type_;
    std::size_t memory_size_ = 0;
  };

  class ClassMemory : public Memory {
   public:
    ClassMemory() {
      TRACE_FUNCTION;
      uint16_t test_data = 0x0011;
      is_big_endian_ = *(uint8_t*)&test_data == 0x00;
    }
    ~ClassMemory() = default;

    void SetGlobalMemory(Memory* global_memory) {
      global_memory_ = global_memory;
    }

    std::size_t Add(std::string name) {
      // std::cout << "Add" << std::endl;
      TRACE_FUNCTION;
      std::size_t index = memory_size_;
      memory_type_.push_back(0x00);
      memory_size_++;
      var_name_.push_back(name);
      return index;
    }

    std::size_t AddWithType(std::string name, std::vector<uint8_t> type) {
      // std::cout << "Add" << std::endl;
      TRACE_FUNCTION;
      std::size_t index = memory_size_;
      /*std::cout<<"AddWithType: ";
      for(size_t i = 0;i<type.size();i++){
        printf("%02x ",type[i]);
      }
      std::cout<<std::endl;*/
      // memory_type_.insert(memory_type_.end(), type.begin(), type.end());
      for (std::size_t i = 0; i < type.size(); i++) {
        memory_type_.push_back(type[i]);
      }
      memory_size_++;
      var_name_.push_back(name);

      return index;
    }

    std::size_t AddByte(std::string name, int8_t value) {
      // std::cout << "AddByte" << std::endl;
      TRACE_FUNCTION;
      memory_type_.push_back(0x01);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddByte(value)));
      return memory_size_ - 1;
    }

    std::size_t AddLong(std::string name, int64_t value) {
      TRACE_FUNCTION;
      memory_type_.push_back(0x02);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddLong(value)));
      return memory_size_ - 1;
    }

    std::size_t AddDouble(std::string name, double value) {
      // std::cout << "AddDouble" << std::endl;
      TRACE_FUNCTION;
      memory_type_.push_back(0x03);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddDouble(value)));
      return memory_size_ - 1;
    }

    std::size_t AddUint64t(std::string name, uint64_t value) {
      // std::cout << "AddUint64t" << std::endl;
      TRACE_FUNCTION;
      memory_type_.push_back(0x04);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddUint64t(value)));
      return memory_size_ - 1;
    }

    std::size_t AddString(std::string name, std::string value) {
      // std::cout << "AddString" << std::endl;
      TRACE_FUNCTION;
      memory_type_.push_back(0x05);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddString(value)));
      return memory_size_ - 1;
    }

    std::size_t AddPtr(std::string name, std::uintptr_t ptr,
                       std::vector<uint8_t> type) {
      TRACE_FUNCTION;
      memory_type_.push_back(0x06);
      memory_size_++;
      std::size_t index = global_memory_->Add(1);
      var_name_.push_back(name);
      code_->push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, index, 0,
                                global_memory_->AddString(name)));
      code_->push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, index,
                                global_memory_->AddPtr(ptr, type)));
      return memory_size_ - 1;
    }

    std::vector<std::string>& GetVarName() { return var_name_; }

   private:
    Memory* global_memory_ = nullptr;
    std::vector<std::string> var_name_;
  };

  class Class {
   public:
    Class() = default;
    ~Class() = default;

    void SetClass(ClassDeclNode* class_decl) {
      TRACE_FUNCTION;
      if (class_decl == nullptr)
        EXIT_COMPILER("Class::SetClass(ClassDeclNode*)",
                      "class_decl is nullptr.");
      class_decl_ = class_decl;
      // TODO(Class)
      /*for (std::size_t i = 0; i < class_decl->GetMembers().size(); i++) {
        if (var_decl_map_.find(class_decl->GetMembers()[i].GetName()) !=
            var_decl_map_.end())
          EXIT_COMPILER("Class::SetClass(ClassDeclNode*)",
                        "Has same name var decl.");
        var_decl_map_.emplace(class_name,
                              std::pair<VarDeclNode*, std::size_t>(
                                  class_decl->GetMembers()[i], i + 1));
      }*/
    }

    /*std::size_t GetVar(std::string var_name) {
      if (var_decl_map_.find(var_name) == var_decl_map_.end())
        EXIT_COMPILER("Class::GetVar(ClassDeclNode*)", "Not found var decl.");

      return var_decl_map_[var_name].second;
    }*/

    bool GetVar(std::string var_name, std::size_t& index) {
      if (var_decl_map_.find(var_name) == var_decl_map_.end()) return false;

      index = var_decl_map_[var_name].second;
      return true;
    }

    std::unordered_map<std::string, std::vector<FuncDeclNode>>&
    GetFuncDeclMap() {
      return func_decl_map_;
    }

    std::vector<Function>& GetFuncList() { return func_list_; }

    ClassMemory& GetMemory() { return memory_; }

    std::vector<Bytecode>& GetCode() { return code_; }

    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
    GetVarDeclMap() {
      return var_decl_map_;
    }

    ClassDeclNode* GetClassDecl() { return class_decl_; }

    void SetName(std::string name) { name_ = name; }

    std::string GetName() { return name_; }

   private:
    std::string name_;
    ClassDeclNode* class_decl_;
    std::unordered_map<std::string, std::vector<FuncDeclNode>> func_decl_map_;
    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
        var_decl_map_;
    std::vector<Function> func_list_;
    ClassMemory memory_;
    std::vector<Bytecode> code_;
    std::size_t name_index = 0;
  };

  void PreProcessDecl(CompoundNode* stmt);
  void PreProcessFuncDecl(FuncDeclNode* stmt);
  void PreProcessClassDecl(ClassDeclNode* stmt);
  // void PreProcessClassConstructor(FuncDeclNode* func_decl);
  // void PreProcessVarDecl(VarDeclNode* stmt);
  void PreProcessStaticDecl(ClassDeclNode* stmt);
  // void PreProcessArrayDecl(ArrayDeclNode* stmt);
  // void PreProcessStaticArrayDecl(ArrayDeclNode* stmt);

  void HandleImport(ImportNode* import_stmt);
  void HandleFuncDecl(FuncDeclNode* func_decl);
  void HandleClassFuncDecl(FuncDeclNode* func_decl);
  void HandleClassConstructor(FuncDeclNode* func_decl);
  void HandleClassDecl(ClassDeclNode* class_decl);
  std::size_t HandleVarDecl(VarDeclNode* var_decl, std::vector<Bytecode>& code);
  std::size_t HandleStartVarDecl(VarDeclNode* var_decl,
                                 std::vector<Bytecode>& code);
  std::size_t HandleStaticVarDecl(VarDeclNode* var_decl,
                                  std::vector<Bytecode>& code);
  std::size_t HandleClassVarDecl(
      ClassMemory& memory,
      std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
          var_decl_map,
      VarDeclNode* var_decl, std::vector<Bytecode>& code);
  std::size_t HandleArrayDecl(ArrayDeclNode* array_decl,
                              std::vector<Bytecode>& code);
  std::size_t HandleStartArrayDecl(ArrayDeclNode* array_decl,
                                   std::vector<Bytecode>& code);
  std::size_t HandleStaticArrayDecl(ArrayDeclNode* array_decl,
                                    std::vector<Bytecode>& code);
  std::size_t HandleClassArrayDecl(
      ClassMemory& memory,
      std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
          var_decl_map,
      ArrayDeclNode* array_decl, std::vector<Bytecode>& code);
  void HandleStmt(StmtNode* stmt, std::vector<Bytecode>& code);
  void HandleBreakStmt(std::vector<Bytecode>& code);
  void HandleSwitchStmt(SwitchNode* stmt, std::vector<Bytecode>& code);
  void HandleClassStmt(StmtNode* stmt, std::vector<Bytecode>& code);
  void HandleReturn(ReturnNode* stmt, std::vector<Bytecode>& code);
  void HandleCompoundStmt(CompoundNode* stmt, std::vector<Bytecode>& code);
  void HandleIfStmt(IfNode* stmt, std::vector<Bytecode>& code);
  void HandleWhileStmt(WhileNode* stmt, std::vector<Bytecode>& code);
  void HandleDowhileStmt(DowhileNode* stmt, std::vector<Bytecode>& code);
  void HandleForStmt(ForNode* stmt, std::vector<Bytecode>& code);
  std::size_t HandleExpr(ExprNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleUnaryExpr(UnaryNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleBinaryExpr(BinaryNode* expr, std::vector<Bytecode>& code);
  std::size_t HandlePeriodExpr(BinaryNode* expr, std::vector<Bytecode>& code);
  std::size_t HandleFuncInvoke(FuncNode* func, std::vector<Bytecode>& code);
  std::size_t HandleClassFuncInvoke(FuncNode* func,
                                    std::vector<Bytecode>& code);
  void HandleLabel(LabelNode* label, std::vector<Bytecode>& code);
  void HandleGoto(GotoNode* label, std::vector<Bytecode>& code);
  void HandleStartGoto(GotoNode* label, std::vector<Bytecode>& code);
  std::size_t GetIndex(ExprNode* expr, std::vector<Bytecode>& code);
  std::size_t GetClassIndex(ExprNode* expr, std::vector<Bytecode>& code);
  std::size_t AddConstInt8t(int8_t value);
  // [[deprecated]] uint8_t GetExprVmType(ExprNode* expr);
  // [[deprecated]] uint8_t GetExprPtrValueVmType(ExprNode* expr);
  // [[deprecated]] uint8_t ConvertTypeToVmType(Type* type);
  // [[deprecated]] std::size_t GetExprVmSize(uint8_t type);
  // [[deprecated]] int SwapInt(int x);
  int64_t SwapLong(int64_t x);
  // [[deprecated]] float SwapFloat(float x);
  double SwapDouble(double x);
  uint64_t SwapUint64t(uint64_t x);
  void InsertUint64ToCode(uint64_t value);
  static std::size_t EncodeUleb128(std::size_t value,
                                   std::vector<uint8_t>& output);
  void GenerateBytecodeFile(const char* output_file);
  void GenerateMnemonicFile();
  Type* GetExprType(ExprNode* expr);
  std::string GetExprTypeString(ExprNode* expr);
  bool IsDereferenced(ExprNode* expr);
  void AddBuiltInFuncDecl(std::string name);
  void InitBuiltInFuncDecl();

  bool is_big_endian_ = false;
  Class start_class_;
  // std::vector<std::pair<std::string, std::string>> import_list_;
  std::unordered_map<std::string, BytecodeGenerator*> import_generator_map_;
  std::vector<std::string> from_list_;
  std::vector<std::pair<std::string, std::string>> from_import_list_;
  std::unordered_map<std::string, std::vector<FuncDeclNode>> func_decl_map_;
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
      var_decl_map_;
  std::unordered_map<std::string, Class*> class_decl_map_;
  std::vector<Function> func_list_;
  std::vector<Class> class_list_;
  Memory global_memory_;
  std::vector<Bytecode> global_code_;
  std::vector<uint8_t> code_;
  // std::size_t dereference_ptr_index_;
  std::vector<std::string> current_scope_;
  std::vector<std::string> single_scope_;
  std::size_t current_func_index_ = 0;
  Class* current_class_ = nullptr;
  std::vector<std::pair<std::string, std::size_t>> goto_map_;
  std::vector<std::pair<std::string, std::size_t>> start_goto_map_;
  std::unordered_map<std::string, std::size_t> label_map_;
  std::vector<std::size_t> exit_index_;
  std::vector<int64_t> loop_break_index_;
  std::size_t undefined_count_ = 0;
};

// std::vector<std::pair<std::string,BytecodeGenerator*>> import_generator;
std::unordered_map<std::string, BytecodeGenerator*> import_generator_map;

BytecodeGenerator::BytecodeGenerator() {
  TRACE_FUNCTION;
  InitBuiltInFuncDecl();
  uint16_t test_data = 0x0011;
  is_big_endian_ = *(uint8_t*)&test_data == 0x00;
}

void BytecodeGenerator::AddBuiltInFuncDecl(std::string name) {
  TRACE_FUNCTION;
  if (func_decl_map_.find(name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*new FuncDeclNode());
    func_decl_map_.emplace(name, func_decl_vector);
  } else {
    func_decl_map_[name].push_back(*new FuncDeclNode());
  }
}

void BytecodeGenerator::InitBuiltInFuncDecl() {
  TRACE_FUNCTION;
  AddBuiltInFuncDecl("__builtin_print");
  AddBuiltInFuncDecl("__builtin_vaprint");
  AddBuiltInFuncDecl("__builtin_remove");
  AddBuiltInFuncDecl("__builtin_rename");
  AddBuiltInFuncDecl("__builtin_getchar");
  AddBuiltInFuncDecl("__builtin_putchar");
  AddBuiltInFuncDecl("__builtin_puts");
  AddBuiltInFuncDecl("__builtin_perror");
}

void BytecodeGenerator::PreProcessDecl(CompoundNode* stmt) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER("BytecodeGenerator::PreProcessDecl(CompoundNode*)",
                  "stmt is nullptr.");

  std::vector<StmtNode*> stmts;

  for (std::size_t i = 0; i < stmt->GetStmts().size(); i++) {
    switch (stmt->GetStmts()[i]->GetType()) {
      case StmtNode::StmtType::kClassDecl:
        PreProcessClassDecl(dynamic_cast<ClassDeclNode*>(stmt->GetStmts()[i]));
        stmts.push_back(stmt->GetStmts()[i]);
        break;

      case StmtNode::StmtType::kFuncDecl:
        PreProcessFuncDecl(dynamic_cast<FuncDeclNode*>(stmt->GetStmts()[i]));
        break;

      case StmtNode::StmtType::kVarDecl:
        // PreProcessVarDecl(dynamic_cast<VarDeclNode*>(stmt->GetStmts()[i]));
        stmts.push_back(stmt->GetStmts()[i]);
        break;

      case StmtNode::StmtType::kArrayDecl:
        // PreProcessArrayDecl(dynamic_cast<ArrayDeclNode*>(stmt->GetStmts()[i]));
        stmts.push_back(stmt->GetStmts()[i]);
        break;

      case StmtNode::StmtType::kImport:
        HandleImport(dynamic_cast<ImportNode*>(stmt->GetStmts()[i]));
        break;
      default:
        break;
    }
  }

  for (std::size_t i = 0; i < stmts.size(); i++) {
    switch (stmts[i]->GetType()) {
      case StmtNode::StmtType::kClassDecl:
        PreProcessStaticDecl(dynamic_cast<ClassDeclNode*>(stmts[i]));
        break;
      case StmtNode::StmtType::kVarDecl:
        var_decl_map_.emplace(
            current_scope_.back() + "." +
                static_cast<std::string>(
                    *dynamic_cast<VarDeclNode*>(stmts[i])->GetName()),
            std::pair<VarDeclNode*, std::size_t>(
                dynamic_cast<VarDeclNode*>(stmts[i]),
                HandleStartVarDecl(dynamic_cast<VarDeclNode*>(stmts[i]),
                                   global_code_)));
        ;
        break;
      case StmtNode::StmtType::kArrayDecl:
        var_decl_map_.emplace(
            current_scope_.back() + "." +
                static_cast<std::string>(
                    *dynamic_cast<ArrayDeclNode*>(stmts[i])->GetName()),
            std::pair<VarDeclNode*, std::size_t>(
                dynamic_cast<ArrayDeclNode*>(stmts[i]),
                HandleStartVarDecl(dynamic_cast<ArrayDeclNode*>(stmts[i]),
                                   global_code_)));
        ;
        // HandleStartArrayDecl(dynamic_cast<ArrayDeclNode*>(stmts[i]),
        // global_code_);
        break;
      default:
        EXIT_COMPILER("BytecodeGenerator::PreProcessDecl(CompoundNode*)",
                      "Unexpected stmt type.");
    }
  }
}

void BytecodeGenerator::PreProcessFuncDecl(FuncDeclNode* stmt) {
  TRACE_FUNCTION;
  std::string func_name =
      current_scope_.back() + "." + (std::string)(*stmt->GetStat()->GetName());
  // std::cout<<"FN: "<<func_name<<std::endl;
  if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*stmt);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*stmt);
  }
}
void BytecodeGenerator::PreProcessClassDecl(ClassDeclNode* stmt) {
  TRACE_FUNCTION;
  std::string class_name =
      current_scope_.back() + "." + std::string(stmt->GetName());
  current_scope_.push_back(class_name);

  Class* current_class = new Class();
  current_class->SetName(class_name);
  current_class->SetClass(stmt);

  current_class->GetMemory().SetCode(&current_class->GetCode());
  current_class->GetMemory().SetGlobalMemory(&global_memory_);

  if (class_decl_map_.find(class_name) != class_decl_map_.end())
    EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                  "Has same name class.");
  class_decl_map_.emplace(class_name, current_class);

  for (std::size_t i = 0; i < stmt->GetClasses().size(); i++) {
    if (stmt->GetClasses()[i]->GetType() == StmtNode::StmtType::kClassDecl) {
      PreProcessClassDecl(dynamic_cast<ClassDeclNode*>(stmt->GetClasses()[i]));
    } else {
      EXIT_COMPILER("BytecodeGenerator::PreProcessClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < stmt->GetStaticMembers().size(); i++) {
    if (stmt->GetStaticMembers()[i]->GetDecl()->GetType() ==
        StmtNode::StmtType::kFuncDecl) {
      PreProcessFuncDecl(
          dynamic_cast<FuncDeclNode*>(stmt->GetStaticMembers()[i]->GetDecl()));
    }
  }

  // auto iterator = func_decl_map_.find(current_scope_.back());
  // if (iterator == func_decl_map_.end()) {
  std::vector<FuncDeclNode> func_decl_vector;
  FuncDeclNode* func_decl = new FuncDeclNode();
  func_decl->SetFuncDeclNode(nullptr, nullptr, nullptr);
  std::string func_name = current_scope_.back();

  if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }
  //}
  current_scope_.pop_back();
}
void BytecodeGenerator::PreProcessStaticDecl(ClassDeclNode* stmt) {
  TRACE_FUNCTION;
  std::string class_name =
      current_scope_.back() + "." + std::string(stmt->GetName());
  current_scope_.push_back(class_name);

  if (class_decl_map_.find(class_name) == class_decl_map_.end())
    EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                  "Not found class decl.");
  Class* current_class = class_decl_map_[class_name];

  for (std::size_t i = 0; i < stmt->GetClasses().size(); i++) {
    if (stmt->GetClasses()[i]->GetType() == StmtNode::StmtType::kClassDecl) {
      PreProcessStaticDecl(dynamic_cast<ClassDeclNode*>(stmt->GetClasses()[i]));
    } else {
      EXIT_COMPILER("BytecodeGenerator::PreProcessStaticDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < stmt->GetStaticMembers().size(); i++) {
    if (stmt->GetStaticMembers()[i]->GetDecl()->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      HandleVarDecl(
          dynamic_cast<VarDeclNode*>(stmt->GetStaticMembers()[i]->GetDecl()),
          current_class->GetCode());
    } else if (stmt->GetStaticMembers()[i]->GetDecl()->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      HandleArrayDecl(
          dynamic_cast<ArrayDeclNode*>(stmt->GetStaticMembers()[i]->GetDecl()),
          current_class->GetCode());
    }
  }

  // auto iterator = func_decl_map_.find(current_scope_.back());
  // if (iterator == func_decl_map_.end()) {
  std::vector<FuncDeclNode> func_decl_vector;
  FuncDeclNode* func_decl = new FuncDeclNode();
  func_decl->SetFuncDeclNode(nullptr, nullptr, nullptr);
  std::string func_name = current_scope_.back();

  if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }
  //}
  current_scope_.pop_back();
}

void BytecodeGenerator::GenerateBytecode(CompoundNode* stmt,
                                         const char* output_file) {
  TRACE_FUNCTION;
  global_memory_.SetCode(&global_code_);

  // Main program return value.
  global_memory_.Add(1);
  global_memory_.Add(1);

  // Bytecode Running class.
  std::vector<uint8_t> bytecode_class_vm_type;
  bytecode_class_vm_type.push_back(0x09);
  global_memory_.AddWithType(bytecode_class_vm_type);

  global_code_.push_back(
      Bytecode(_AQVM_OPERATOR_EQUAL, 2, 0, global_memory_.AddString("(void)")));
  // std::size_t return_value_ptr = global_memory_.Add(1);
  // global_code_.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, 0,
  // return_value_ptr));
  global_code_.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, 1, 0));
  // current_scope_.push_back("global");
  current_scope_.push_back("");
  // single_scope_.push_back("global");
  current_func_index_ = current_scope_.size() - 1;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::GenerateBytecode(CompoundNode*,const char*)",
        "stmt is nullptr.");

  start_class_.SetName(".__start");
  start_class_.GetMemory().Add("@name");
  start_class_.GetMemory().Add("@size");

  PreProcessDecl(stmt);

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

      case StmtNode::StmtType::kGoto:
        HandleStartGoto(dynamic_cast<GotoNode*>(stmt->GetStmts()[i]),
                        global_code_);
        break;

      default:
        HandleStmt(stmt->GetStmts()[i], global_code_);
        /*EXIT_COMPILER(
            "BytecodeGenerator::GenerateBytecode(CompoundNode*,const char*)",
            "Unexpected code.");*/
    }
  }

  while (start_goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator = label_map_.find(current_scope_[i] + "$" +
                                      start_goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == 0)
        EXIT_COMPILER(
            "BytecodeGenerator::GenerateBytecode(CompoundNode*,const char*)",
            "Label not found.");
    }
    // std::cout << global_code_[start_goto_map_.back().second].GetOper() <<
    // std::endl;
    global_code_[start_goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    start_goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;

  std::vector<std::size_t> constructor_args;
  std::vector<Bytecode> start_code;
  constructor_args.push_back(global_memory_.Add(1));
  std::size_t start_func_name = global_memory_.Add(1);
  std::string name_str = ".!__start";
  global_memory_.GetConstTable().push_back(0x05);
  EncodeUleb128(name_str.size() + 1, global_memory_.GetConstTable());
  for (std::size_t i = 0; i < name_str.size(); i++) {
    global_memory_.GetConstTable().push_back(name_str[i]);
  }
  global_memory_.GetConstTable().push_back(0x00);
  global_memory_.GetConstTableSize()++;
  std::size_t name_const_index = global_memory_.GetConstTableSize() - 1;
  start_code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_CONST, 2, start_func_name,
                                name_const_index));
  std::vector<std::size_t> invoke_start_args = {2, start_func_name, 1, 1};
  start_code.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_start_args));
  Function constructor_func("@constructor", constructor_args, start_code);
  func_list_.push_back(constructor_func);

  std::vector<std::size_t> args;
  args.push_back(1);
  // std::vector<Bytecode> start_code;
  // std::size_t main_func = global_memory_.AddString("global.main");
  std::size_t main_func = global_memory_.AddString(".main");
  /*for (size_t i = 0; i < global_memory_.GetCode().size(); i++) {
    start_code.push_back(global_memory_.GetCode()[i]);
  }
  for (size_t i = 0; i < global_code_.size(); i++) {
    start_code.push_back(global_code_[i]);
  }*/
  /*start_code.insert(start_code.end(), global_memory_.GetCode().begin(),
                    global_memory_.GetCode().end());*/
  // start_code.insert(start_code.end(), global_code_.begin(),
  // global_code_.end());
  std::vector<std::size_t> invoke_main_args = {2, main_func, 1, 1};
  // start_code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_main_args));
  global_code_.push_back(
      Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_main_args));
  // Function start_func("__start", args, start_code);
  Function start_func(".!__start", args, global_code_);
  func_list_.push_back(start_func);

  if (loop_break_index_.size() != 0)
    EXIT_COMPILER(
        "BytecodeGenerator::GenerateBytecode(CompoundNode*,const char*)",
        "Break cannot be used outside of loops and switches.");

  GenerateBytecodeFile(output_file);
}

void BytecodeGenerator::GenerateBytecodeFile(const char* output_file) {
  TRACE_FUNCTION;

  code_.push_back(0x41);
  code_.push_back(0x51);
  code_.push_back(0x42);
  code_.push_back(0x43);

  // Version
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x00);
  code_.push_back(0x03);

  /*InsertUint64ToCode(is_big_endian_ ? import_list_.size()
                                    : SwapUint64t(import_list_.size()));

  for (std::size_t i = 0; i < import_list_.size(); i++) {
    import_list_[i].first = import_list_[i].first+"bc";
    code_.insert(
        code_.end(),
        reinterpret_cast<const uint8_t*>(import_list_[i].first.c_str()),
        reinterpret_cast<const uint8_t*>(import_list_[i].first.c_str() +
                                         import_list_[i].first.size() + 1));
    code_.insert(
        code_.end(),
        reinterpret_cast<const uint8_t*>(import_list_[i].second.c_str()),
        reinterpret_cast<const uint8_t*>(import_list_[i].second.c_str() +
                                         import_list_[i].second.size() + 1));
  }*/

  InsertUint64ToCode(is_big_endian_
                         ? global_memory_.GetConstTableSize()
                         : SwapUint64t(global_memory_.GetConstTableSize()));
  for (std::size_t i = 0; i < global_memory_.GetConstTable().size(); i++) {
    code_.push_back(global_memory_.GetConstTable()[i]);
  }
  std::size_t memory_size = global_memory_.GetMemorySize();
  InsertUint64ToCode(is_big_endian_ ? memory_size : SwapUint64t(memory_size));
  for (std::size_t i = 0; i < global_memory_.GetMemoryType().size(); i++) {
    code_.push_back(global_memory_.GetMemoryType()[i]);
  }
  // std::cout<<"Size: "<<code_.size()<<std::endl;

  for (std::size_t i = 0; i < class_list_.size(); i++) {
    std::string class_name_str = class_list_[i].GetName();

    const char* class_name = class_name_str.c_str();
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(class_name),
                 reinterpret_cast<const uint8_t*>(class_name +
                                                  class_name_str.size() + 1));

    // std::cout << "Point A" << std::endl;

    std::size_t memory_size = class_list_[i].GetMemory().GetMemorySize();
    // std::cout << "Size: " << memory_size << std::endl;
    memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);

    // std::cout << "Point E" << std::endl;

    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
                 reinterpret_cast<const uint8_t*>(&memory_size + 1));

    // std::cout << "Size: " << memory_size << std::endl;

    // std::cout << "Point D" << std::endl;

    for (std::size_t j = 0;
         j < class_list_[i].GetMemory().GetMemoryType().size(); j++) {
      for (std::size_t k = 0;
           k < class_list_[i].GetMemory().GetVarName()[j].size() + 1; k++) {
        code_.push_back(class_list_[i].GetMemory().GetVarName()[j].c_str()[k]);
      }
      code_.push_back(class_list_[i].GetMemory().GetMemoryType()[j]);
    }

    // std::cout << "Point B" << std::endl;

    std::size_t methods_size = class_list_[i].GetFuncList().size();
    methods_size = is_big_endian_ ? methods_size : SwapUint64t(methods_size);
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&methods_size),
                 reinterpret_cast<const uint8_t*>(&methods_size + 1));

    std::vector<Function> func_list = class_list_[i].GetFuncList();

    // std::cout << "Point C" << std::endl;

    for (std::size_t z = 0; z < func_list.size(); z++) {
      // std::cout << func_list.size() << std::endl;
      //  Function name (with '\0')
      std::string func_name_str = func_list[z].GetName();
      const char* func_name = func_name_str.c_str();
      // std::cout << func_name_str << std::endl;
      code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                   reinterpret_cast<const uint8_t*>(
                       func_name + func_list[z].GetName().size() + 1));

      if (func_list[z].GetVaFlag()) code_.push_back(0xFF);

      std::vector<uint8_t> args_buffer;

      EncodeUleb128(func_list[z].GetArgs().size(), args_buffer);
      code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
      for (std::size_t j = 0; j < func_list[z].GetArgs().size(); j++) {
        args_buffer.clear();
        EncodeUleb128(func_list[z].GetArgs()[j], args_buffer);
        code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
      }

      uint64_t value = is_big_endian_
                           ? func_list[z].GetCode().size()
                           : SwapUint64t(func_list[z].GetCode().size());
      code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                   reinterpret_cast<uint8_t*>(&value) + 8);

      for (std::size_t j = 0; j < func_list[z].GetCode().size(); j++) {
        std::vector<uint8_t> buffer;
        switch (func_list[z].GetCode()[j].GetOper()) {
          case _AQVM_OPERATOR_NOP:
            code_.push_back(_AQVM_OPERATOR_NOP);
            break;

          case _AQVM_OPERATOR_LOAD:
            code_.push_back(_AQVM_OPERATOR_LOAD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected LOAD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_STORE:
            code_.push_back(_AQVM_OPERATOR_STORE);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected STORE args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEW:
            code_.push_back(_AQVM_OPERATOR_NEW);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected NEW args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ARRAY:
            code_.push_back(_AQVM_OPERATOR_ARRAY);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected ARRAY args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_PTR:
            code_.push_back(_AQVM_OPERATOR_PTR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected PTR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_ADD:
            code_.push_back(_AQVM_OPERATOR_ADD);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected ADD args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SUB:
            code_.push_back(_AQVM_OPERATOR_SUB);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected SUB args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_MUL:
            code_.push_back(_AQVM_OPERATOR_MUL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected MUL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_DIV:
            code_.push_back(_AQVM_OPERATOR_DIV);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected DIV args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REM:
            code_.push_back(_AQVM_OPERATOR_REM);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected REM args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_NEG:
            code_.push_back(_AQVM_OPERATOR_NEG);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected NEG args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHL:
            code_.push_back(_AQVM_OPERATOR_SHL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected SHL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_SHR:
            code_.push_back(_AQVM_OPERATOR_SHR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected SHR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_REFER:
            code_.push_back(_AQVM_OPERATOR_REFER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected REFER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            break;

          case _AQVM_OPERATOR_IF:
            code_.push_back(_AQVM_OPERATOR_IF);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected IF args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_AND:
            code_.push_back(_AQVM_OPERATOR_AND);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected AND args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_OR:
            code_.push_back(_AQVM_OPERATOR_OR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected OR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_XOR:
            code_.push_back(_AQVM_OPERATOR_XOR);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected XOR args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CMP:
            code_.push_back(_AQVM_OPERATOR_CMP);

            if (func_list[z].GetCode()[j].GetArgs().size() != 4)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected CMP args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[3], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE:
            code_.push_back(_AQVM_OPERATOR_INVOKE);

            if (func_list[z].GetCode()[j].GetArgs().size() < 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected INVOKE args size.");

            for (std::size_t k = 0;
                 k != func_list[z].GetCode()[j].GetArgs()[1] + 2; k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code_.insert(code_.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_EQUAL:
            code_.push_back(_AQVM_OPERATOR_EQUAL);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected EQUAL args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_GOTO:
            code_.push_back(_AQVM_OPERATOR_GOTO);

            if (func_list[z].GetCode()[j].GetArgs().size() != 1)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected GOTO args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_LOAD_CONST:
            code_.push_back(_AQVM_OPERATOR_LOAD_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected LOAD_CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONVERT:
            code_.push_back(_AQVM_OPERATOR_CONVERT);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected CONVERT args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_CONST:
            code_.push_back(_AQVM_OPERATOR_CONST);

            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected CONST args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_INVOKE_METHOD:
            code_.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

            if (func_list[z].GetCode()[j].GetArgs().size() < 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected INVOKE_METHOD args size.");

            /*std::cout << "INVOKE_METHOD SIZE: "
                      << func_list[z].GetCode()[j].GetArgs().size()
                      << std::endl;*/

            for (std::size_t k = 0;
                 k < func_list[z].GetCode()[j].GetArgs().size(); k++) {
              EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[k], buffer);
              code_.insert(code_.end(), buffer.begin(), buffer.end());
              buffer.clear();
            }
            break;

          case _AQVM_OPERATOR_LOAD_MEMBER:
            code_.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER(
                  "BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                  "Unexpected LOAD_MEMBER args size.");

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[0], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[1], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();

            EncodeUleb128(func_list[z].GetCode()[j].GetArgs()[2], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
            break;

          case _AQVM_OPERATOR_WIDE:
            code_.push_back(_AQVM_OPERATOR_WIDE);
            break;

          default:
            break;
        }
      }
    }
  }

  // std::cout << "SUCCESS BEFORE __start" << std::endl;

  std::string class_name_str = ".!__start";

  const char* class_name = class_name_str.c_str();
  code_.insert(
      code_.end(), reinterpret_cast<const uint8_t*>(class_name),
      reinterpret_cast<const uint8_t*>(class_name + class_name_str.size() + 1));

  /*memory_size = 0;
  memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);
  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
               reinterpret_cast<const uint8_t*>(&memory_size + 1));
  for (std::size_t i = 0;
       i < class_list_[i].GetMemory().GetMemoryType().size(); i++) {
    code_.push_back(class_list_[i].GetMemory().GetMemoryType()[i]);
  }*/

  memory_size = start_class_.GetMemory().GetMemorySize();
  memory_size = is_big_endian_ ? memory_size : SwapUint64t(memory_size);

  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&memory_size),
               reinterpret_cast<const uint8_t*>(&memory_size + 1));

  for (std::size_t j = 0; j < start_class_.GetMemory().GetMemoryType().size();
       j++) {
    for (std::size_t k = 0;
         k < start_class_.GetMemory().GetVarName()[j].size() + 1; k++) {
      code_.push_back(start_class_.GetMemory().GetVarName()[j].c_str()[k]);
    }
    code_.push_back(start_class_.GetMemory().GetMemoryType()[j]);
  }

  std::size_t methods_size = func_list_.size();
  methods_size = is_big_endian_ ? methods_size : SwapUint64t(methods_size);
  code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(&methods_size),
               reinterpret_cast<const uint8_t*>(&methods_size + 1));

  std::vector<Function> func_list = func_list_;

  for (std::size_t i = 0; i < func_list.size(); i++) {
    // std::cout << func_list.size() << std::endl;
    //  Function name (with '\0')
    std::string func_name_str = func_list[i].GetName();
    const char* func_name = func_name_str.c_str();
    // std::cout << func_name_str << std::endl;
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                 reinterpret_cast<const uint8_t*>(
                     func_name + func_list[i].GetName().size() + 1));

    if (func_list[i].GetVaFlag()) code_.push_back(0xFF);

    std::vector<uint8_t> args_buffer;

    EncodeUleb128(func_list[i].GetArgs().size(), args_buffer);
    code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    for (std::size_t j = 0; j < func_list[i].GetArgs().size(); j++) {
      args_buffer.clear();
      EncodeUleb128(func_list[i].GetArgs()[j], args_buffer);
      code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    }

    uint64_t value = is_big_endian_
                         ? func_list[i].GetCode().size()
                         : SwapUint64t(func_list[i].GetCode().size());
    code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                 reinterpret_cast<uint8_t*>(&value) + 8);

    for (std::size_t j = 0; j < func_list[i].GetCode().size(); j++) {
      std::vector<uint8_t> buffer;
      switch (func_list[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          code_.push_back(_AQVM_OPERATOR_NOP);
          break;

        case _AQVM_OPERATOR_LOAD:
          code_.push_back(_AQVM_OPERATOR_LOAD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_STORE:
          code_.push_back(_AQVM_OPERATOR_STORE);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected STORE args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEW:
          code_.push_back(_AQVM_OPERATOR_NEW);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEW args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ARRAY:
          code_.push_back(_AQVM_OPERATOR_ARRAY);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected ARRAY args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_PTR:
          code_.push_back(_AQVM_OPERATOR_PTR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected PTR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_ADD:
          code_.push_back(_AQVM_OPERATOR_ADD);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected ADD args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SUB:
          code_.push_back(_AQVM_OPERATOR_SUB);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SUB args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_MUL:
          code_.push_back(_AQVM_OPERATOR_MUL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected MUL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_DIV:
          code_.push_back(_AQVM_OPERATOR_DIV);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected DIV args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REM:
          code_.push_back(_AQVM_OPERATOR_REM);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected REM args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_NEG:
          code_.push_back(_AQVM_OPERATOR_NEG);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected NEG args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHL:
          code_.push_back(_AQVM_OPERATOR_SHL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_SHR:
          code_.push_back(_AQVM_OPERATOR_SHR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected SHR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_REFER:
          code_.push_back(_AQVM_OPERATOR_REFER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected REFER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          break;

        case _AQVM_OPERATOR_IF:
          code_.push_back(_AQVM_OPERATOR_IF);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected IF args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_AND:
          code_.push_back(_AQVM_OPERATOR_AND);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected AND args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_OR:
          code_.push_back(_AQVM_OPERATOR_OR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected OR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_XOR:
          code_.push_back(_AQVM_OPERATOR_XOR);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected XOR args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CMP:
          code_.push_back(_AQVM_OPERATOR_CMP);

          if (func_list[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CMP args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[3], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE:
          code_.push_back(_AQVM_OPERATOR_INVOKE);

          if (func_list[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected INVOKE args size.");

          for (std::size_t k = 0;
               k != func_list[i].GetCode()[j].GetArgs()[1] + 2; k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_EQUAL:
          code_.push_back(_AQVM_OPERATOR_EQUAL);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected EQUAL args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_GOTO:
          code_.push_back(_AQVM_OPERATOR_GOTO);

          if (func_list[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected GOTO args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_LOAD_CONST:
          code_.push_back(_AQVM_OPERATOR_LOAD_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD_CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONVERT:
          code_.push_back(_AQVM_OPERATOR_CONVERT);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CONVERT args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONST:
          code_.push_back(_AQVM_OPERATOR_CONST);

          if (func_list[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CONST args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          code_.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

          if (func_list[i].GetCode()[j].GetArgs().size() < 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected INVOKE_METHOD args size.");

          /*std::cout << "INVOKE_METHOD SIZE: "
                    << func_list[i].GetCode()[j].GetArgs().size() <<
             std::endl;*/

          for (std::size_t k = 0;
               k < func_list[i].GetCode()[j].GetArgs().size(); k++) {
            EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          code_.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

          if (func_list[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD_MEMBER args size.");

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list[i].GetCode()[j].GetArgs()[2], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_WIDE:
          code_.push_back(_AQVM_OPERATOR_WIDE);
          break;

        default:
          break;
      }
    }
  }

  /*for (std::size_t i = 0; i < func_list_.size(); i++) {
    std::cout << func_list_.size() << std::endl;
    // Function name (with '\0')
    std::string func_name_str = func_list_[i].GetName();
    const char* func_name = func_name_str.c_str();
    std::cout << func_name_str << std::endl;
    code_.insert(code_.end(), reinterpret_cast<const uint8_t*>(func_name),
                 reinterpret_cast<const uint8_t*>(
                     func_name + func_list_[i].GetName().size() + 1));

    std::vector<uint8_t> args_buffer;
    EncodeUleb128(func_list_[i].GetArgs().size(), args_buffer);
    code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    for (std::size_t j = 0; j < func_list_[i].GetArgs().size(); j++) {
      args_buffer.clear();
      EncodeUleb128(func_list_[i].GetArgs()[j], args_buffer);
      code_.insert(code_.end(), args_buffer.begin(), args_buffer.end());
    }

    uint64_t value = is_big_endian_
                         ? func_list_[i].GetCode().size()
                         : SwapUint64t(func_list_[i].GetCode().size());
    code_.insert(code_.end(), reinterpret_cast<uint8_t*>(&value),
                 reinterpret_cast<uint8_t*>(&value) + 8);

    for (std::size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
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

        case _AQVM_OPERATOR_ARRAY:
          code_.push_back(_AQVM_OPERATOR_ARRAY);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected ARRAY args size.");

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

        case _AQVM_OPERATOR_REFER:
          code_.push_back(_AQVM_OPERATOR_REFER);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected REFER args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
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

          for (std::size_t k = 0;
               k != func_list_[i].GetCode()[j].GetArgs()[1] + 2; k++) {
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

        case _AQVM_OPERATOR_LOAD_CONST:
          code_.push_back(_AQVM_OPERATOR_LOAD_CONST);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD_CONST args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONVERT:
          code_.push_back(_AQVM_OPERATOR_CONVERT);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CONVERT args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_CONST:
          code_.push_back(_AQVM_OPERATOR_CONST);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected CONST args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          code_.push_back(_AQVM_OPERATOR_INVOKE_METHOD);

          if (func_list_[i].GetCode()[j].GetArgs().size() < 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected INVOKE_METHOD args size.");

          for (std::size_t k = 0;
               k != func_list_[i].GetCode()[j].GetArgs()[1] + 3; k++) {
            EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[k], buffer);
            code_.insert(code_.end(), buffer.begin(), buffer.end());
            buffer.clear();
          }
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          code_.push_back(_AQVM_OPERATOR_LOAD_MEMBER);

          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateBytecode(CompoundNode*)",
                          "Unexpected LOAD_MEMBER args size.");

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[0], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();

          EncodeUleb128(func_list_[i].GetCode()[j].GetArgs()[1], buffer);
          code_.insert(code_.end(), buffer.begin(), buffer.end());
          buffer.clear();
          break;

        case _AQVM_OPERATOR_WIDE:
          code_.push_back(_AQVM_OPERATOR_WIDE);
          break;

        default:
          break;
      }
    }
  }*/

  std::string filename(output_file);
  std::ofstream outFile(filename, std::ios::binary);
  if (!outFile) {
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecodeFile(const char*)",
                  "Can't open file.");
    return;
  }

  outFile.write(reinterpret_cast<const char*>(code_.data()), code_.size());
  outFile.close();

  if (!outFile) {
    EXIT_COMPILER("BytecodeGenerator::GenerateBytecodeFile(const char*)",
                  "Failed to write file.");
  } else {
    std::cout << "[INFO] " << "Write file success: " << filename << std::endl;
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
    EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                  "Can't open file.");
  }

  std::streambuf* cout_buffer = std::cout.rdbuf();
  std::cout.rdbuf(output_file.rdbuf());

  /*for (std::size_t i = 0; i < class_list_.size(); i++) {
      std::string class_name_str = class_list_[i].GetName();

      const char* class_name = class_name_str.c_str();
      std::cout<<"Class Name: "<<class_name<<std::endl;


      std::size_t memory_size = class_list_[i].GetMemory().GetMemorySize();
      std::cout << "Memory Size: " << memory_size << std::endl;


      for (std::size_t j = 0;
           j < class_list_[i].GetMemory().GetMemoryType().size(); j++) {
        for (std::size_t k = 0;
             k < class_list_[i].GetMemory().GetVarName()[j].size() + 1; k++) {
          code_.push_back(class_list_[i].GetMemory().GetVarName()[j].c_str()[k]);
        }
        code_.push_back(class_list_[i].GetMemory().GetMemoryType()[j]);
      }


      std::size_t methods_size = class_list_[i].GetFuncList().size();
      std::cout<<"Method Size: "<<methods_size<<std::endl;

      std::vector<Function> func_list = class_list_[i].GetFuncList();


      for (std::size_t z = 0; z < func_list.size(); z++) {
        std::cout << func_list.size() << std::endl;
        // Function name (with '\0')
        std::string func_name_str = func_list[z].GetName();
        const char* func_name = func_name_str.c_str();
        std::cout << func_name_str << std::endl;

        std::vector<uint8_t> args_buffer;
        EncodeUleb128(func_list[z].GetArgs().size(), args_buffer);
        for (std::size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
        switch (func_list_[i].GetCode()[j].GetOper()) {
          case _AQVM_OPERATOR_NOP:
            std::cout << "NOP" << std::endl;
            break;

          case _AQVM_OPERATOR_LOAD:
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected LOAD args size.");
            std::cout << "LOAD: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_STORE:
            std::cout << "STORE: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected STORE args size.");
            break;

          case _AQVM_OPERATOR_NEW:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected NEW args size.");
            std::cout << "NEW: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_ARRAY:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected ARRAY args size.");
            std::cout << "ARRAY: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_PTR:
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected PTR args size.");
            std::cout << "PTR: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_ADD:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected ADD args size.");
            std::cout << "ADD: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_SUB:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected SUB args size.");
            std::cout << "SUB: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_MUL:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected MUL args size.");
            std::cout << "MUL: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_DIV:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected DIV args size.");
            std::cout << "DIV: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_REM:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected REM args size.");
            std::cout << "REM: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_NEG:
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected NEG args size.");
            std::cout << "NEG: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_SHL:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected SHL args size.");
            std::cout << "SHL: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_SHR:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected SHR args size.");
            std::cout << "SHR: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_REFER:
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected REFER args size.");
            std::cout << "REFER: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_IF:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected IF args size.");
            std::cout << "IF: " << func_list[z].GetCode()[j].GetArgs()[0] << "
    ,"
                      << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_AND:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected AND args size.");
            std::cout << "AND: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_OR:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected OR args size.");
            std::cout << "OR: " << func_list[z].GetCode()[j].GetArgs()[0] << "
    ,"
                      << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_XOR:
            if (func_list[z].GetCode()[j].GetArgs().size() != 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected XOR args size.");
            std::cout << "XOR: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_CMP:

            if (func_list[z].GetCode()[j].GetArgs().size() != 4)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected CMP args size.");
            std::cout << "CMP: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[3] << std::endl;
            break;

          case _AQVM_OPERATOR_INVOKE:
            if (func_list[z].GetCode()[j].GetArgs().size() < 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected INVOKE args size.");
            std::cout << "INVOKE: ";
            for (std::size_t k = 0;
                 k != func_list[z].GetCode()[j].GetArgs()[1] + 2; k++) {
              std::cout << func_list[z].GetCode()[j].GetArgs()[k] << " ,";
            }
            std::cout << std::endl;
            break;

          case _AQVM_OPERATOR_EQUAL:
            if (func_list[z].GetCode()[j].GetArgs().size() != 2)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected GOTO args size.");
            std::cout << "EQUAL: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_GOTO:
            if (func_list[z].GetCode()[j].GetArgs().size() != 1)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected GOTO args size.");
            std::cout << "GOTO: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_LOAD_CONST:
            std::cout << "LOAD_CONST: " <<
    func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_CONVERT:
            std::cout << "CONVERT: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_CONST:
            std::cout << "CONST: " << func_list[z].GetCode()[j].GetArgs()[0]
                      << " ," << func_list[z].GetCode()[j].GetArgs()[1]
                      << std::endl;
            break;

          case _AQVM_OPERATOR_INVOKE_METHOD:
            if (func_list[z].GetCode()[j].GetArgs().size() < 3)
              EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                            "Unexpected INVOKE_METHOD args size.");
            std::cout << "INVOKE_METHOD: ";
            for (std::size_t k = 0;
                 k < func_list[z].GetCode()[j].GetArgs().size(); k++) {
              std::cout << func_list[z].GetCode()[j].GetArgs()[k] << " ,";
            }
            std::cout << std::endl;
            break;

          case _AQVM_OPERATOR_LOAD_MEMBER:
            std::cout << "LOAD_MEMBER: "
                      << func_list[z].GetCode()[j].GetArgs()[0] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[1] << " ,"
                      << func_list[z].GetCode()[j].GetArgs()[2] << std::endl;
            break;

          case _AQVM_OPERATOR_WIDE:
            std::cout << "WIDE" << std::endl;
            break;

          default:
            break;
        }
        }
      }
    }*/

  std::size_t memory_size = global_memory_.GetMemorySize();
  std::cout << "Memory Size: " << memory_size << std::endl;
  std::cout << std::endl << std::endl << std::endl;

  /*for (size_t i = 0; i < global_memory_.GetMemoryType().size(); i++) {
    printf("%i ", global_memory_.GetMemoryType()[i]);
  }*/

  for (std::size_t i = 0; i < func_list_.size(); i++) {
    std::cout << "Function Name: " << func_list_[i].GetName()
              << ", Size: " << func_list_[i].GetCode().size() << std::endl;

    for (std::size_t j = 0; j < func_list_[i].GetCode().size(); j++) {
      switch (func_list_[i].GetCode()[j].GetOper()) {
        case _AQVM_OPERATOR_NOP:
          std::cout << "NOP" << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
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
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected STORE args size.");
          break;

        case _AQVM_OPERATOR_NEW:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected NEW args size.");
          std::cout << "NEW: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_ARRAY:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected ARRAY args size.");
          std::cout << "ARRAY: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_PTR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected PTR args size.");
          std::cout << "PTR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_ADD:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected ADD args size.");
          std::cout << "ADD: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SUB:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected SUB args size.");
          std::cout << "SUB: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_MUL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected MUL args size.");
          std::cout << "MUL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_DIV:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected DIV args size.");
          std::cout << "DIV: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_REM:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected REM args size.");
          std::cout << "REM: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_NEG:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected NEG args size.");
          std::cout << "NEG: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_SHL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected SHL args size.");
          std::cout << "SHL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_SHR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected SHR args size.");
          std::cout << "SHR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_REFER:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected REFER args size.");
          std::cout << "REFER: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_IF:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected IF args size.");
          std::cout << "IF: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_AND:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected AND args size.");
          std::cout << "AND: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_OR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected OR args size.");
          std::cout << "OR: " << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_XOR:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected XOR args size.");
          std::cout << "XOR: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
          break;

        case _AQVM_OPERATOR_CMP:

          if (func_list_[i].GetCode()[j].GetArgs().size() != 4)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected CMP args size.");
          std::cout << "CMP: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[3] << std::endl;
          break;

        case _AQVM_OPERATOR_INVOKE:
          if (func_list_[i].GetCode()[j].GetArgs().size() < 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected INVOKE args size.");
          std::cout << "INVOKE: ";
          for (std::size_t k = 0;
               k != func_list_[i].GetCode()[j].GetArgs()[1] + 2; k++) {
            std::cout << func_list_[i].GetCode()[j].GetArgs()[k] << " ,";
          }
          std::cout << std::endl;
          break;

        case _AQVM_OPERATOR_EQUAL:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 2)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected GOTO args size.");
          std::cout << "EQUAL: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_GOTO:
          if (func_list_[i].GetCode()[j].GetArgs().size() != 1)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected GOTO args size.");
          std::cout << "GOTO: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD_CONST:
          std::cout << "LOAD_CONST: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_CONVERT:
          std::cout << "CONVERT: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_CONST:
          std::cout << "CONST: " << func_list_[i].GetCode()[j].GetArgs()[0]
                    << " ," << func_list_[i].GetCode()[j].GetArgs()[1]
                    << std::endl;
          break;

        case _AQVM_OPERATOR_INVOKE_METHOD:
          if (func_list_[i].GetCode()[j].GetArgs().size() < 3)
            EXIT_COMPILER("BytecodeGenerator::GenerateMnemonicFile()",
                          "Unexpected INVOKE_METHOD args size.");
          std::cout << "INVOKE_METHOD: ";
          for (std::size_t k = 0;
               k < func_list_[i].GetCode()[j].GetArgs().size(); k++) {
            std::cout << func_list_[i].GetCode()[j].GetArgs()[k] << " ,";
          }
          std::cout << std::endl;
          break;

        case _AQVM_OPERATOR_LOAD_MEMBER:
          std::cout << "LOAD_MEMBER: "
                    << func_list_[i].GetCode()[j].GetArgs()[0] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[1] << " ,"
                    << func_list_[i].GetCode()[j].GetArgs()[2] << std::endl;
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

void BytecodeGenerator::HandleImport(ImportNode* import_stmt) {
  TRACE_FUNCTION;
  if (import_stmt == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleImport(ImportNode*)",
                  "import_stmt is nullptr.");

  if (import_stmt->IsFromImport()) {
    EXIT_COMPILER("BytecodeGenerator::HandleImport(ImportNode*)",
                  "Unsupported import type now.");
  } else {
    std::string import_location = import_stmt->GetImportLocation();
    std::string name = import_stmt->GetName();

    // import_list_.push_back(std::pair<std::string,
    // std::string>(import_location, name));

    if (import_generator_map.find(import_location) ==
        import_generator_map.end()) {
      // TODO(IMPORTANT): Unknown error, but can be fixed with this statement. Serious
      // issue, awaiting repair.
      std::cout << import_generator_map[import_location] << std::endl;

      std::cout << "Import A NEW FILE." << import_location << std::endl;
      const char* filename = import_location.c_str();
      std::ifstream file;
      file.open(filename);
      if (!file.is_open()) {
        printf("Error: Could not open file %s\n", filename);
        return;
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
      lexer.LexToken(first_token_buffer, first_token_buffer);
      token.push_back(first_token_buffer);
      while (true) {
        Aq::Compiler::Token token_buffer;
        lexer.LexToken(token.back(), token_buffer);
        token.push_back(token_buffer);
        if (lexer.IsReadEnd()) {
          break;
        }
      }

      Aq::Compiler::CompoundNode* ast = Aq::Compiler::Parser::Parse(token);

      if (ast == nullptr)
        EXIT_COMPILER("BytecodeGenerator::HandleImport(ImportNode*)",
                      "ast is nullptr\n");
      BytecodeGenerator* bytecode_generator = new BytecodeGenerator();
      std::string bytecode_file = import_location + "bc";
      bytecode_generator->GenerateBytecode(ast, bytecode_file.c_str());
      import_generator_map.emplace(import_location, bytecode_generator);

      // import_generator.push_back(std::pair<std::string,
      // BytecodeGenerator*>(import_location, bytecode_generator));
    }
    if (import_generator_map_.find(name) != import_generator_map_.end())
      EXIT_COMPILER("BytecodeGenerator::HandleImport(ImportNode*)",
                    "Has same name bytecode file.");
    import_generator_map_.emplace(name, import_generator_map[import_location]);

    // uint8_t vm_type[] ={0x09};
    // std::vector<uint8_t> vm_type;
    // vm_type.push_back(0x09);
    std::size_t class_array_index = start_class_.GetMemory().Add(name);
    std::size_t array_index = global_memory_.Add(1);
    global_code_.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index,
                                    2, global_memory_.AddString(name)));
    global_code_.push_back(Bytecode(
        _AQVM_OPERATOR_NEW, 3, array_index, global_memory_.AddUint64t(0),
        global_memory_.AddString("~" + import_location + "bc~.!__start")));
    global_code_.push_back(
        Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, array_index,
                 global_memory_.AddString("@constructor"), 1, array_index));
    start_class_.GetVarDeclMap().emplace(
        static_cast<std::string>(name),
        std::pair<VarDeclNode*, std::size_t>(nullptr, class_array_index));
    // std::cout << "Import Name: "<< current_scope_.back() + "#" +
    // static_cast<std::string>(name)<< std::endl;
    var_decl_map_.emplace(
        current_scope_.back() + "#" + static_cast<std::string>(name),
        std::pair<VarDeclNode*, std::size_t>(nullptr, array_index));
    // global_code_.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4,
    // array_index,global_memory_.AddString("!__start"),1,global_memory_.Add(1)));
  }
}

void BytecodeGenerator::HandleFuncDecl(FuncDeclNode* func_decl) {
  TRACE_FUNCTION;
  /*current_scope_.push_back(
      static_cast<std::string>(*func_decl->GetStat()->GetName()));*/
  if (func_decl == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                  "func_decl is nullptr.");

  std::vector<Bytecode> code;
  std::string func_name;
  // for (std::size_t i = 0; i < current_scope_.size(); i++) {
  func_name += current_scope_.back();
  func_name += ".";
  //}
  // std::string single_scope_name = *func_decl->GetStat()->GetName();
  func_name += *func_decl->GetStat()->GetName();

  std::string scope_name = func_name;
  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStat()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      // single_scope_name += "@";
      scope_name += "@";
    } else {
      // single_scope_name += ",";
      scope_name += ",";
    }

    if (args[i]->GetType() != StmtNode::StmtType::kVarDecl &&
        args[i]->GetType() != StmtNode::StmtType::kArrayDecl) {
      EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      // single_scope_name +=
      // *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      // single_scope_name +=
      // *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      scope_name += ",...";
    }
  }
  if (args.size() == 0 && func_decl->GetStat()->GetVaFlag()) {
    scope_name += "@...";
  }

  goto_map_.clear();
  current_scope_.push_back(scope_name);
  // single_scope_.push_back(single_scope_name);
  current_func_index_ = current_scope_.size() - 1;
  // std::cout << "func_name: " << func_name << std::endl;
  /*if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }*/

  if (func_decl->GetStmts() == nullptr) {
    current_scope_.pop_back();
    // single_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();

  std::size_t return_value_index = global_memory_.AddWithType(vm_type);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));
  args_index.push_back(return_value_reference_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i]->GetType() == StmtNode::StmtType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  if (args.size() == 0 && func_decl->GetStat()->GetVaFlag()) {
    va_array_index = global_memory_.Add(1);
    args_index.push_back(va_array_index);
  }

  for (size_t i = 0; i < func_decl->GetStat()->GetArgs().size(); i++) {
    if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.AddWithType(vm_type)));
    } else if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(
              array_decl, global_memory_.AddWithType(vm_type)));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStat()->GetArgs().size() - 1 &&
        func_decl->GetStat()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  if (func_decl->GetStat()->GetArgs().size() == 0 &&
      func_decl->GetStat()->GetVaFlag()) {
    var_decl_map_.emplace(
        current_scope_.back() + "#args",
        std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
  }

  exit_index_.clear();
  HandleStmt(func_decl->GetStmts(), code);
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStat()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  func_list_.push_back(func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("BytecodeGenerator::HandleFuncDecl(FuncDeclNode*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode* func_decl) {
  TRACE_FUNCTION;
  if (func_decl == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                  "func_decl is nullptr.");
  if (current_class_ == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                  "current_class_ is nullptr.");

  std::vector<Bytecode> code;
  std::string scope_name;
  scope_name += current_scope_.back();
  scope_name += ".";
  // std::string single_scope_name = *func_decl->GetStat()->GetName();
  scope_name += *func_decl->GetStat()->GetName();

  std::string func_name = *func_decl->GetStat()->GetName();

  if (std::string(current_class_->GetClassDecl()->GetName()) == func_name) {
    HandleClassConstructor(func_decl);
    return;
  }

  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStat()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      // single_scope_name += "@";
      scope_name += "@";
    } else {
      // single_scope_name += ",";
      scope_name += ",";
    }

    if (args[i]->GetType() != StmtNode::StmtType::kVarDecl &&
        args[i]->GetType() != StmtNode::StmtType::kArrayDecl) {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      // single_scope_name +=
      // *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      // single_scope_name +=
      // *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      scope_name += ",...";
    }
  }

  goto_map_.clear();
  // single_scope_.push_back(single_scope_name);
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;
  if (current_class_->GetFuncDeclMap().find(func_name) ==
      current_class_->GetFuncDeclMap().end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    current_class_->GetFuncDeclMap().emplace(func_name, func_decl_vector);
  } else {
    current_class_->GetFuncDeclMap()[func_name].push_back(*func_decl);
  }

  if (func_decl->GetStmts() == nullptr) {
    current_scope_.pop_back();
    // single_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();

  std::size_t return_value_index = global_memory_.AddWithType(vm_type);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));
  args_index.push_back(return_value_reference_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i]->GetType() == StmtNode::StmtType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  for (size_t i = 0; i < func_decl->GetStat()->GetArgs().size(); i++) {
    if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.AddWithType(vm_type)));
    } else if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(
              array_decl, global_memory_.AddWithType(vm_type)));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStat()->GetArgs().size() - 1 &&
        func_decl->GetStat()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  exit_index_.clear();
  HandleClassStmt(func_decl->GetStmts(), code);
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStat()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  current_class_->GetFuncList().push_back(func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void BytecodeGenerator::HandleClassConstructor(FuncDeclNode* func_decl) {
  TRACE_FUNCTION;
  if (func_decl == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                  "func_decl is nullptr.");
  if (current_class_ == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                  "current_class_ is nullptr.");

  // std::cout << "HandleClassConstructor" << std::endl;

  std::vector<Bytecode> code;
  std::string class_name = current_class_->GetName();
  std::string scope_name;
  scope_name += current_scope_.back();
  // scope_name += ".";
  // scope_name += *func_decl->GetStat()->GetName();

  // std::string original_func_name = *func_decl->GetStat()->GetName();
  std::string original_func_name = "@constructor";
  // std::cout << "Class Name: " << original_func_name << std::endl;
  std::string func_name = scope_name;

  // std::cout << "func_name: " << func_name << std::endl;
  std::vector<ExprNode*> args = func_decl->GetStat()->GetArgs();
  for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      scope_name += "@";
    } else {
      scope_name += ",";
    }

    if (args[i]->GetType() != StmtNode::StmtType::kVarDecl &&
        args[i]->GetType() != StmtNode::StmtType::kArrayDecl) {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      scope_name += *dynamic_cast<VarDeclNode*>(args[i])->GetVarType();
    } else {
      scope_name += *dynamic_cast<ArrayDeclNode*>(args[i])->GetVarType();
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      scope_name += ",...";
    }
  }

  goto_map_.clear();
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;
  if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    func_decl_map_.emplace(func_name, func_decl_vector);
  } else {
    func_decl_map_[func_name].push_back(*func_decl);
  }

  if (func_decl->GetStmts() == nullptr) {
    current_scope_.pop_back();
    current_func_index_ = 0;
    return;
  }

  std::vector<std::size_t> args_index;

  // std::vector<uint8_t> vm_type = func_decl->GetReturnType()->GetVmType();
  // std::vector<uint8_t> vm_type;
  // vm_type.push_back(0x09);

  std::size_t return_value_index = global_memory_.Add(1);
  var_decl_map_.emplace(
      scope_name + "#!return",
      std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

  /*std::size_t return_value_reference_index = global_memory_.Add(1);
  var_decl_map_.emplace(scope_name + "#!return_reference",
                        std::pair<VarDeclNode*, std::size_t>(
                            nullptr, return_value_reference_index));*/
  args_index.push_back(return_value_index);

  std::size_t va_array_index = 0;

  for (std::size_t i = 0; i < args.size(); i++) {
    if (args[i]->GetType() == StmtNode::StmtType::kVarDecl) {
      args_index.push_back(
          HandleVarDecl(dynamic_cast<VarDeclNode*>(args[i]), code));
    } else if (args[i]->GetType() == StmtNode::StmtType::kArrayDecl) {
      args_index.push_back(
          HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(args[i]), code));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == args.size() - 1 && func_decl->GetStat()->GetVaFlag()) {
      va_array_index = global_memory_.Add(1);
      args_index.push_back(va_array_index);
    }
  }

  for (size_t i = 0; i < func_decl->GetStat()->GetArgs().size(); i++) {
    if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      VarDeclNode* var_decl =
          dynamic_cast<VarDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(current_scope_.back() + "#" +
                                static_cast<std::string>(*var_decl->GetName()),
                            std::pair<VarDeclNode*, std::size_t>(
                                var_decl, global_memory_.Add(1)));
    } else if (func_decl->GetStat()->GetArgs()[i]->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      ArrayDeclNode* array_decl =
          dynamic_cast<ArrayDeclNode*>(func_decl->GetStat()->GetArgs()[i]);
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*array_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(array_decl,
                                               global_memory_.Add(1)));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                    "args is not VarDeclNode or ArrayDeclNode.");
    }
    if (i == func_decl->GetStat()->GetArgs().size() - 1 &&
        func_decl->GetStat()->GetVaFlag()) {
      var_decl_map_.emplace(
          current_scope_.back() + "#args",
          std::pair<VarDeclNode*, std::size_t>(NULL, va_array_index));
    }
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_value_index,
                          global_memory_.AddUint64t(0),
                          global_memory_.AddString(class_name)));

  if (current_class_->GetFuncDeclMap().find(original_func_name) ==
      current_class_->GetFuncDeclMap().end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    func_decl_vector.push_back(*func_decl);
    current_class_->GetFuncDeclMap().emplace(original_func_name,
                                             func_decl_vector);
  } else {
    current_class_->GetFuncDeclMap()[original_func_name].push_back(*func_decl);
  }

  std::vector<std::size_t> invoke_class_args;
  invoke_class_args.push_back(return_value_index);
  invoke_class_args.push_back(global_memory_.AddString(original_func_name));
  invoke_class_args.push_back(args_index.size());
  invoke_class_args.push_back(global_memory_.Add(1));
  invoke_class_args.insert(invoke_class_args.end(), args_index.begin() + 1,
                           args_index.end());
  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_class_args));

  // code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  Function func_decl_bytecode(func_name, args_index, code);
  if (func_decl->GetStat()->GetVaFlag()) func_decl_bytecode.EnableVaFlag();
  func_list_.push_back(func_decl_bytecode);

  current_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();

  code.clear();
  current_scope_.push_back(scope_name);
  current_func_index_ = current_scope_.size() - 1;

  exit_index_.clear();

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  for (std::size_t i = 0; i < current_class_->GetCode().size(); i++) {
    code.push_back(current_class_->GetCode()[i]);
  }

  HandleClassStmt(func_decl->GetStmts(), code);
  // code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t return_location = code.size();
  for (std::size_t i = 0; i < exit_index_.size(); i++) {
    code[exit_index_[i]].SetArgs(1, global_memory_.AddUint64t(return_location));
  }
  Function new_func_decl_bytecode(original_func_name, args_index, code);
  if (func_decl->GetStat()->GetVaFlag()) new_func_decl_bytecode.EnableVaFlag();
  current_class_->GetFuncList().push_back(new_func_decl_bytecode);
  exit_index_.clear();

  while (goto_map_.size() > 0) {
    std::size_t goto_location = 0;
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      auto iterator =
          label_map_.find(current_scope_[i] + "$" + goto_map_.back().first);
      if (iterator != label_map_.end()) {
        goto_location = iterator->second;
        break;
      }
      if (i == current_func_index_)
        EXIT_COMPILER("BytecodeGenerator::HandleClassFuncDecl(FuncDeclNode*)",
                      "Label not found.");
    }
    code[goto_map_.back().second].SetArgs(
        1, global_memory_.AddUint64t(goto_location));
    goto_map_.pop_back();
  }
  current_scope_.pop_back();
  current_func_index_ = 0;
  goto_map_.clear();
}

void BytecodeGenerator::HandleClassDecl(ClassDeclNode* class_decl) {
  TRACE_FUNCTION;
  if (class_decl == nullptr)
    EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                  "class_decl is nullptr.");

  std::string class_name =
      current_scope_.back() + "." + std::string(class_decl->GetName());

  // std::string single_scope_name = std::string(class_decl->GetName());
  current_scope_.push_back(class_name);
  // single_scope_.push_back(single_scope_name);
  /*for (std::size_t i = 0; i < class_decl->GetMembers().size(); i++) {
    if (class_decl->GetMembers()[i]->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      HandleVarDecl(dynamic_cast<VarDeclNode*>(class_decl->GetMembers()[i]),
                    global_code_);
    } else if (class_decl->GetMembers()[i]->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      HandleArrayDecl(dynamic_cast<ArrayDeclNode*>(class_decl->GetMembers()[i]),
                      global_code_);
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }*/

  if (class_decl_map_.find(class_name) == class_decl_map_.end())
    EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                  "Not found class decl.");
  Class* current_class = class_decl_map_[class_name];
  current_class_ = current_class;
  /*Class* current_class = new Class();
  current_class->SetName(class_name);
  current_class->SetClass(class_decl);
  current_class_ = current_class;

  current_class->GetMemory().SetCode(&current_class->GetCode());
  current_class->GetMemory().SetGlobalMemory(&global_memory_);

  if (class_decl_map_.find(class_name) != class_decl_map_.end())
    EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                  "Has same name class.");
  class_decl_map_.emplace(class_name, current_class);*/

  current_class->GetMemory().Add("@name");
  current_class->GetMemory().Add("@size");

  for (std::size_t i = 0; i < class_decl->GetClasses().size(); i++) {
    if (class_decl->GetClasses()[i]->GetType() ==
        StmtNode::StmtType::kClassDecl) {
      HandleClassDecl(
          dynamic_cast<ClassDeclNode*>(class_decl->GetClasses()[i]));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }
  current_class_ = current_class;

  for (std::size_t i = 0; i < class_decl->GetStaticMembers().size(); i++) {
    // class_decl->GetStaticMembers()[i]->GetDecl()
    // std::cout << "HANDLE STATIC MEMBERS." << std::endl;
    if (class_decl->GetStaticMembers()[i]->GetDecl()->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      HandleStaticVarDecl(dynamic_cast<VarDeclNode*>(
                              class_decl->GetStaticMembers()[i]->GetDecl()),
                          current_class->GetCode());
    } else if (class_decl->GetStaticMembers()[i]->GetDecl()->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      HandleStaticArrayDecl(dynamic_cast<ArrayDeclNode*>(
                                class_decl->GetStaticMembers()[i]->GetDecl()),
                            current_class->GetCode());
    } else if (class_decl->GetStaticMembers()[i]->GetDecl()->GetType() ==
               StmtNode::StmtType::kFuncDecl) {
      HandleFuncDecl(dynamic_cast<FuncDeclNode*>(
          class_decl->GetStaticMembers()[i]->GetDecl()));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < class_decl->GetMembers().size(); i++) {
    // std::cout << "Handle var in HandleClassDecl" << std::endl;
    if (class_decl->GetMembers()[i]->GetType() ==
        StmtNode::StmtType::kVarDecl) {
      HandleClassVarDecl(
          current_class->GetMemory(), current_class->GetVarDeclMap(),
          dynamic_cast<VarDeclNode*>(class_decl->GetMembers()[i]),
          current_class->GetCode());
    } else if (class_decl->GetMembers()[i]->GetType() ==
               StmtNode::StmtType::kArrayDecl) {
      HandleClassArrayDecl(
          current_class->GetMemory(), current_class->GetVarDeclMap(),
          dynamic_cast<ArrayDeclNode*>(class_decl->GetMembers()[i]),
          current_class->GetCode());
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }

  for (std::size_t i = 0; i < class_decl->GetMethods().size(); i++) {
    if (class_decl->GetMethods()[i]->GetType() ==
        StmtNode::StmtType::kFuncDecl) {
      HandleClassFuncDecl(
          dynamic_cast<FuncDeclNode*>(class_decl->GetMethods()[i]));
    } else {
      EXIT_COMPILER("BytecodeGenerator::HandleClassDecl(ClassDeclNode*)",
                    "Unexpected code.");
    }
  }

  auto iterator = func_decl_map_.find(current_scope_.back());
  if (iterator == func_decl_map_.end()) {
    std::vector<FuncDeclNode> func_decl_vector;
    FuncDeclNode* func_decl = new FuncDeclNode();
    func_decl->SetFuncDeclNode(nullptr, nullptr, nullptr);
    std::vector<Bytecode> code;
    std::string class_name = current_class_->GetName();
    std::string scope_name;
    scope_name += current_scope_.back();
    std::string original_func_name = "@constructor";
    std::string func_name = scope_name;

    goto_map_.clear();
    current_scope_.push_back(scope_name);
    current_func_index_ = current_scope_.size() - 1;
    if (func_decl_map_.find(func_name) == func_decl_map_.end()) {
      std::vector<FuncDeclNode> func_decl_vector;
      func_decl_vector.push_back(*func_decl);
      func_decl_map_.emplace(func_name, func_decl_vector);
    } else {
      func_decl_map_[func_name].push_back(*func_decl);
    }

    std::vector<std::size_t> args_index;

    std::size_t return_value_index = global_memory_.Add(1);
    var_decl_map_.emplace(
        scope_name + "#!return",
        std::pair<VarDeclNode*, std::size_t>(nullptr, return_value_index));

    args_index.push_back(return_value_index);

    std::size_t va_array_index = 0;

    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, return_value_index,
                            global_memory_.AddUint64t(0),
                            global_memory_.AddString(class_name)));

    if (current_class_->GetFuncDeclMap().find(original_func_name) ==
        current_class_->GetFuncDeclMap().end()) {
      func_decl_vector.push_back(*func_decl);
      current_class_->GetFuncDeclMap().emplace(original_func_name,
                                               func_decl_vector);
    } else {
      current_class_->GetFuncDeclMap()[original_func_name].push_back(
          *func_decl);
    }

    std::vector<std::size_t> invoke_class_args;
    invoke_class_args.push_back(return_value_index);
    invoke_class_args.push_back(global_memory_.AddString(original_func_name));
    invoke_class_args.push_back(args_index.size());
    invoke_class_args.push_back(global_memory_.Add(1));
    invoke_class_args.insert(invoke_class_args.end(), args_index.begin() + 1,
                             args_index.end());
    code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, invoke_class_args));

    Function func_decl_bytecode(func_name, args_index, code);
    func_list_.push_back(func_decl_bytecode);

    current_scope_.pop_back();
    current_func_index_ = 0;
    goto_map_.clear();

    code.clear();
    current_scope_.push_back(scope_name);
    current_func_index_ = current_scope_.size() - 1;

    exit_index_.clear();

    code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

    Function new_func_decl_bytecode(original_func_name, args_index, code);
    current_class_->GetFuncList().push_back(new_func_decl_bytecode);
    exit_index_.clear();

    current_scope_.pop_back();
    current_func_index_ = 0;
    goto_map_.clear();
  }

  // current_class_ = current_class;

  current_scope_.pop_back();
  // single_scope_.pop_back();
  current_class_ = nullptr;
  class_list_.push_back(*current_class);
}

std::size_t BytecodeGenerator::HandleVarDecl(VarDeclNode* var_decl,
                                             std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
        "var_decl is nullptr.");

  /*Type* var_type = var_decl->GetVarType();
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
          "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&)",
          "Unexpected code.");
    if (var_type->GetType() == Type::TypeType::kConst)
      var_type = dynamic_cast<ConstType*>(var_type)->GetSubType();
  }

  uint8_t vm_type = 0x00;
  if (var_type->GetType() == Type::TypeType::kBase) {
    switch (var_type->GetBaseType()) {
      case Type::BaseType::kAuto:
      case Type::BaseType::kVoid:
        vm_type = 0x00;
        break;
      case Type::BaseType::kBool:
      case Type::BaseType::kChar:
        vm_type = 0x01;
        break;
      case Type::BaseType::kShort:
      case Type::BaseType::kInt:
      case Type::BaseType::kLong:
        vm_type = 0x02;
        break;
      case Type::BaseType::kFloat:
      case Type::BaseType::kDouble:
        vm_type = 0x03;
        break;
        // TODO(uint64_t)
        vm_type = 0x04;
        break;
      case Type::BaseType::kString:
        vm_type = 0x05;
        break;
      case Type::BaseType::kClass:
      case Type::BaseType::kStruct:
      case Type::BaseType::kUnion:
      case Type::BaseType::kEnum:
      case Type::BaseType::kPointer:
      case Type::BaseType::kArray:
      case Type::BaseType::kFunction:
      case Type::BaseType::kTypedef:
        // TODO
        vm_type = 0x06;
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }
  } else if (var_type->GetType() == Type::TypeType::kPointer) {
    vm_type = 0x06;
  } else if (var_type->GetType() == Type::TypeType::kReference) {
    vm_type = 0x07;
  }*/

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
    // return_type.insert(return_type.begin(), 0x08);
    vm_type.erase(vm_type.begin());
  }

  // TODO(Class)

  if (var_decl->GetValue()[0] == nullptr) {
    // std::cout << "None Value" << std::endl;
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, var_index_reference,
                   global_memory_.AddString("@constructor"), 1, 0));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    /*if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&"
          ")",
          "Const doesn't have value.");
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }*/
    /*if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<Bytecode>&"
          ")",
          "Reference doesn't have value.");*/
    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    // std::cout << "Has Value" << std::endl;
    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);
      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, value_index, value_ptr_index));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, value_ptr_index));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  }
}

std::size_t BytecodeGenerator::HandleStaticVarDecl(
    VarDeclNode* var_decl, std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
        "Bytecode>&)",
        "var_decl is nullptr.");

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "const var without value not support.");
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, var_index_reference,
                   global_memory_.AddString("@constructor"), 1, 0));
    }
    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    std::size_t var_index = global_memory_.AddWithType(vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStaticVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));

      global_code_.push_back(Bytecode(
          _AQVM_OPERATOR_NEW, 3, var_index_reference, global_memory_.AddByte(0),
          global_memory_.AddString(func_name)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map_.emplace(
          current_scope_.back() + "." +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    global_code_.push_back(
        Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map_.emplace(
          current_scope_.back() + "." +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  }
}

std::size_t BytecodeGenerator::HandleClassVarDecl(
    ClassMemory& memory,
    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
        var_decl_map,
    VarDeclNode* var_decl, std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
        "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
        "VarDeclNode*,std::vector<Bytecode>&)",
        "var_decl is nullptr.");

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  /*std::cout << "HandleClassVarDecl: "
            << static_cast<std::string>(*var_decl->GetName()) << std::endl;*/

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = memory.AddWithType(
        static_cast<std::string>(*var_decl->GetName()), vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
              "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
              "VarDeclNode*,std::vector<Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 0,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      /*code.push_back(
      Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference, var_index));*/

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    /*if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
      std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }*/
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Reference doesn't have value.");
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    std::size_t original_var_index = 0;
    std::size_t var_index = 0;

    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      var_index = global_memory_.AddWithType(vm_type);
    } else {
      original_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), vm_type);
      var_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index, 0,
                              global_memory_.AddString(static_cast<std::string>(
                                  *var_decl->GetName()))));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      /*std::string func_name = (std::string)*var_decl->GetVarType() + "." +
                              (std::string)*var_decl->GetVarType();*/
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
                  << std::endl;*/
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
              "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
              "VarDeclNode*,std::vector<Bytecode>&)",
              "Function not found.");
      }

      // std::size_t var_index_ptr = global_memory_.Add(1);
      std::size_t var_index_reference = global_memory_.Add(1);

      /*global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, var_index_ptr));*/

      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 0,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      /*code.push_back(
      Bytecode(_AQVM_OPERATOR_REFER, 2, var_index_reference,
      reference_index));*/

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
      /*std::vector<std::size_t> invoke_args;
      invoke_args.push_back(global_memory_.AddString(func_name));
      invoke_args.push_back(1);
      invoke_args.push_back(var_index_reference);

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, invoke_args));*/
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassVarDecl(ClassMemory&,std::"
          "unordered_map<std::string,std::pair<VarDeclNode*,std::size_t>>,"
          "VarDeclNode*,std::vector<Bytecode>&)",
          "Const doesn't have value.");
      /*std::size_t const_var_index = global_memory_.AddWithType(return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      std::size_t value_ptr_index = global_memory_.AddWithType(value_ptr);
      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, var_index,
      value_ptr_index)); code.push_back( Bytecode(_AQVM_OPERATOR_CONST, 2,
      const_var_index, value_ptr_index)); var_decl_map_.emplace(
          current_scope_.back() + "#" +
              static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;*/
    }

    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = memory.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, value_index, value_ptr_index));*/
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      std::size_t const_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      // std::size_t value_ptr_index = memory.AddWithType(value_ptr);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, 2, var_index, value_ptr_index));*/
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, original_var_index));
    return original_var_index;
  }
}

std::size_t BytecodeGenerator::HandleStartVarDecl(VarDeclNode* var_decl,
                                                  std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (var_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
        "Bytecode>&)",
        "var_decl is nullptr.");

  ClassMemory& memory = start_class_.GetMemory();
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
      var_decl_map = start_class_.GetVarDeclMap();

  std::vector<uint8_t> vm_type = var_decl->GetVarType()->GetVmType();
  std::vector<uint8_t> return_type = vm_type;
  if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
    vm_type.erase(vm_type.begin());
  }

  if (var_decl->GetValue()[0] == nullptr) {
    std::size_t var_index = memory.AddWithType(
        static_cast<std::string>(*var_decl->GetName()), vm_type);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 2,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "Const doesn't have value.");
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "Reference doesn't have value.");
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
    return var_index;
  } else {
    std::size_t original_var_index = 0;
    std::size_t var_index = 0;

    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      var_index = global_memory_.AddWithType(vm_type);
    } else {
      original_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), vm_type);
      var_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index, 2,
                              global_memory_.AddString(static_cast<std::string>(
                                  *var_decl->GetName()))));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kClass) {
      std::string func_name = (std::string)*var_decl->GetVarType();
      for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
        auto iterator = func_decl_map_.find(func_name);
        if (i != -1)
          iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
        if (iterator != func_decl_map_.end()) {
          func_name = func_name;
          if (i != -1) func_name = current_scope_[i] + "." + func_name;
          break;
        }
        if (i == -1)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
              "Bytecode>&)",
              "Function not found.");
      }

      std::size_t var_index_reference = global_memory_.Add(1);

      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, var_index_reference, 2,
                   global_memory_.AddString(
                       static_cast<std::string>(*var_decl->GetName()))));

      code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, var_index_reference,
                              global_memory_.AddByte(0),
                              global_memory_.AddString(func_name)));
    }
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStartVarDecl(VarDeclNode*,std::vector<"
          "Bytecode>&)",
          "Const doesn't have value.");
    }

    std::size_t value_index = HandleExpr(var_decl->GetValue()[0], code);
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kReference) {
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.erase(value_ptr.begin());
      value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, var_index, value_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, var_index));
      return var_index;
    }
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, var_index, value_index));
    if (var_decl->GetVarType()->GetType() == Type::TypeType::kConst) {
      std::size_t const_var_index = memory.AddWithType(
          static_cast<std::string>(*var_decl->GetName()), return_type);
      std::vector<uint8_t> value_ptr = vm_type;
      value_ptr.insert(value_ptr.begin(), 0x06);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_CONST, 2, const_var_index, var_index));
      var_decl_map.emplace(
          static_cast<std::string>(*var_decl->GetName()),
          std::pair<VarDeclNode*, std::size_t>(var_decl, const_var_index));
      return const_var_index;
    }
    var_decl_map.emplace(
        static_cast<std::string>(*var_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(var_decl, original_var_index));
    return original_var_index;
  }
}

std::size_t BytecodeGenerator::HandleArrayDecl(ArrayDeclNode* array_decl,
                                               std::vector<Bytecode>& code) {
  // std::cout << "Array DECL Handle. 1" << std::endl;
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
    EXIT_COMPILER(
        "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "const array not support.");

  // std::cout << "Array DECL Handle. 2" << std::endl;

  /*if (array_type->GetType() == Type::TypeType::kConst)
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
      case Type::BaseType::kString:
      case Type::BaseType::kClass:
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
  }*/
  if (array_decl->GetValue().empty()) {
    // std::cout << "Array DECL Handle." << std::endl;
    /*if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");*/
    /*std::size_t array_size = 0;
    std::size_t value_vm_type =
        dynamic_cast<ValueNode*>(array_decl->GetSize())->GetVmType();
    switch (value_vm_type) {
      case 0x01:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetByteValue();
        break;
      case 0x02:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetIntValue();
        break;
      case 0x03:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetLongValue();
        break;
      case 0x04:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetFloatValue();
        break;
      case 0x05:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetDoubleValue();
        break;
      case 0x06:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetUInt64Value();
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }*/
    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    // std::size_t array_ptr_index = global_memory_.Add(1);
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            global_memory_.AddByte(1)
                            /*HandleExpr(array_decl->GetSize(), code)*/,
                            array_type_index));

    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddByte(0)));

      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                              global_memory_.AddString("@constructor"), 1, 0));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  } else {
    // std::cout << "Array DECL Handle. value" << std::endl;
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");
    /*std::size_t array_size = 0;
    std::size_t value_vm_type =
        dynamic_cast<ValueNode*>(array_decl->GetSize())->GetVmType();
    switch (value_vm_type) {
      case 0x01:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetByteValue();
        break;
      case 0x02:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetIntValue();
        break;
      case 0x03:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetLongValue();
        break;
      case 0x04:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetFloatValue();
        break;
      case 0x05:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetDoubleValue();
        break;
      case 0x06:
        array_size =
            dynamic_cast<ValueNode*>(array_decl->GetSize())->GetUInt64Value();
        break;
      default:
        EXIT_COMPILER(
            "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected code.");
        break;
    }*/

    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    // std::cout << "array type .1" << std::endl;
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    // std::cout << "array type ." << std::endl;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    std::size_t size_index =
        global_memory_.AddUint64t(array_decl->GetValue().size());
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            size_index
                            /*HandleExpr(array_decl->GetSize(), code)*/,
                            array_type_index));

    // std::size_t array_ptr_index = global_memory_.Add(1);
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      // std::cout << "Handle Array DECL with value." << std::endl;
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "#" +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  }
}

std::size_t BytecodeGenerator::HandleStaticArrayDecl(
    ArrayDeclNode* array_decl, std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_type->GetType() == Type::TypeType::kConst)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "const array not support.");

  if (array_decl->GetValue().empty()) {
    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    global_code_.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                                    global_memory_.AddByte(1)
                                    /*HandleExpr(array_decl->GetSize(), code)*/,
                                    array_type_index));

    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::size_t current_index = global_memory_.Add(1);
      global_code_.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                                      array_index, global_memory_.AddByte(0)));

      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, 4, current_index,
                   global_memory_.AddString("@constructor"), 1, 0));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  } else {
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t array_index =
        global_memory_.AddWithType(array_type->GetVmType());
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStaticArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    std::size_t size_index =
        global_memory_.AddUint64t(array_decl->GetValue().size());
    global_code_.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                                    size_index
                                    /*HandleExpr(array_decl->GetSize(), code)*/,
                                    array_type_index));

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      global_code_.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                                      array_index,
                                      global_memory_.AddUint64t(i)));
      global_code_.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map_.emplace(
        current_scope_.back() + "." +
            static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), array_index));
    return array_index;
  }
}

std::size_t BytecodeGenerator::HandleClassArrayDecl(
    ClassMemory& memory,
    std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>
        var_decl_map,
    ArrayDeclNode* array_decl, std::vector<Bytecode>& code) {
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

  if (array_decl->GetValue().empty()) {
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());

    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 0,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  } else {
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());
    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 0,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            HandleExpr(array_decl->GetSize(), code),
                            array_type_index));
    /*code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, 2, array_index, array_ptr_index));*/

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  }
}

std::size_t BytecodeGenerator::HandleStartArrayDecl(
    ArrayDeclNode* array_decl, std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (array_decl == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_decl is nullptr.");

  ClassMemory& memory = start_class_.GetMemory();
  std::unordered_map<std::string, std::pair<VarDeclNode*, std::size_t>>&
      var_decl_map = start_class_.GetVarDeclMap();

  Type* array_type = array_decl->GetVarType();
  if (array_type == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
        "Bytecode>&)",
        "array_type is nullptr.");

  if (array_decl->GetValue().empty()) {
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());

    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 2,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  } else {
    if (array_decl->GetSize()->GetType() != StmtNode::StmtType::kValue)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::vector<"
          "Bytecode>&)",
          "array_decl->GetSize() is not ValueNode.");

    std::size_t class_array_index =
        memory.AddWithType(static_cast<std::string>(*array_decl->GetName()),
                           array_type->GetVmType());
    std::size_t array_index = global_memory_.Add(1);
    code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, array_index, 2,
                            global_memory_.AddString(static_cast<std::string>(
                                *array_decl->GetName()))));
    std::size_t array_type_index = 0;
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetType() ==
        Type::TypeType::kClass) {
      std::string expr_type_string =
          *dynamic_cast<ArrayType*>(array_type)->GetSubType();
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleStartArrayDecl(ArrayDeclNode*,std::"
              "vector<"
              "Bytecode>&)",
              "Not found class.");
      }
      array_type_index = global_memory_.AddString(expr_type_string);

    } else {
      array_type_index = global_memory_.AddWithType(
          dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType());
    }
    if (dynamic_cast<ArrayType*>(array_type)->GetSubType()->GetVmType()[0] ==
        0x00)
      array_type_index = 0;
    code.push_back(Bytecode(_AQVM_OPERATOR_NEW, 3, array_index,
                            HandleExpr(array_decl->GetSize(), code),
                            array_type_index));

    for (std::size_t i = 0; i < array_decl->GetValue().size(); i++) {
      std::size_t value_index = HandleExpr(array_decl->GetValue()[i], code);
      std::size_t current_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, current_index,
                              array_index, global_memory_.AddUint64t(i)));
      code.push_back(
          Bytecode(_AQVM_OPERATOR_EQUAL, 2, current_index, value_index));
    }

    var_decl_map.emplace(
        static_cast<std::string>(*array_decl->GetName()),
        std::pair<VarDeclNode*, std::size_t>(
            dynamic_cast<VarDeclNode*>(array_decl), class_array_index));
    return class_array_index;
  }
}

std::size_t BytecodeGenerator::HandleExpr(ExprNode* expr,
                                          std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleExpr(ExprNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  if (expr->GetType() == StmtNode::StmtType::kUnary ||
      expr->GetType() == StmtNode::StmtType::kArray) {
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
      // uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      /*if (IsDereferenced(expr->GetExpr()))
        code.push_back(Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_,
                                sub_expr));*/
      return new_index;
    }
    case UnaryNode::Operator::kPostDec: {  // -- (postfix)
      // uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, new_index, sub_expr));
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      /*if (IsDereferenced(expr->GetExpr()))
        code.push_back(Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_,
                                sub_expr));*/
      return new_index;
    }
    case UnaryNode::Operator::kPreInc:  // ++ (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      /*if (IsDereferenced(expr->GetExpr()))
        code.push_back(Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_,
                                sub_expr));*/
      return sub_expr;
    case UnaryNode::Operator::kPreDec:  // -- (prefix)
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, sub_expr, sub_expr,
                              AddConstInt8t(1)));
      /*if (IsDereferenced(expr->GetExpr()))
        code.push_back(Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_,
                                sub_expr));*/
      return sub_expr;
    /*case UnaryNode::Operator::kAddrOf: {  // & (address of)
      std::size_t ptr_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, sub_expr, ptr_index));
      return ptr_index;
    }*/
    /*case UnaryNode::Operator::kDeref: {  // * (dereference)
      // uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index = global_memory_.Add(1);
      // dereference_ptr_index_ = sub_expr;

      // code.push_back(Bytecode(_AQVM_OPERATOR_LOAD, 2, sub_expr, new_index));
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, new_index, sub_expr));
      return new_index;
    }*/
    case UnaryNode::Operator::kPlus:  // + (unary plus)
      return sub_expr;
    case UnaryNode::Operator::kMinus: {  // - (unary minus)
      // uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }
    case UnaryNode::Operator::kNot: {  // ! (logical NOT)
      // uint8_t vm_type = GetExprVmType(expr->GetExpr());
      std::size_t new_index = global_memory_.Add(1);

      code.push_back(Bytecode(_AQVM_OPERATOR_NEG, 2, new_index, sub_expr));
      return new_index;
    }
      /*case UnaryNode::Operator::CONVERT: {  // ()
        std::size_t new_index = global_memory_.AddWithType(
            dynamic_cast<CastNode*>(expr)->GetCastType()->GetVmType());
        code.push_back(Bytecode(_AQVM_OPERATOR_CONVERT, 2, new_index,
      sub_expr)); return new_index;
      }*/

    case UnaryNode::Operator::ARRAY: {  // []
      // std::cout << "ARRAY" << std::endl;
      // Type* array_type =
      // GetExprType(dynamic_cast<ArrayNode*>(expr)->GetExpr()); uint8_t vm_type
      // = 0;
      // std::size_t offset_expr = global_memory_.Add(1);
      std::size_t offset =
          HandleExpr(dynamic_cast<ArrayNode*>(expr)->GetIndex(), code);

      /*if (array_type->GetType() == Type::TypeType::kArray) {
        vm_type = ConvertTypeToVmType(
            dynamic_cast<ArrayType*>(array_type)->GetSubType());
        code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, offset_expr, offset,
                                AddConstInt8t(GetExprVmSize(vm_type))));
      } else if (array_type->GetType() == Type::TypeType::kPointer) {
        vm_type = ConvertTypeToVmType(
            dynamic_cast<PointerType*>(array_type)->GetSubType());
        code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, offset_expr, offset,
                                AddConstInt8t(GetExprVmSize(vm_type))));
      } else {
        EXIT_COMPILER(
            "BytecodeGenerator::HandleUnaryExpr(UnaryNode*,std::vector<"
            "Bytecode>&)",
            "Unsupported type.");
      }*/

      // dereference_ptr_index_ = global_memory_.Add(1);
      /*code.push_back(
          Bytecode(_AQVM_OPERATOR_ADD, 3, offset_expr, sub_expr, offset));*/

      std::size_t new_index = global_memory_.Add(1);

      // code.push_back(Bytecode(_AQVM_OPERATOR_LOAD, 2, dereference_ptr_index_,
      // new_index));

      code.push_back(
          Bytecode(_AQVM_OPERATOR_ARRAY, 3, new_index, sub_expr, offset));
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

  // std::cout << "HandleBinaryExpr RUNNING" << std::endl;

  ExprNode* right_expr = expr->GetRightExpr();
  ExprNode* left_expr = expr->GetLeftExpr();

  std::size_t left = 0;
  std::size_t right = 0;
  if (expr->GetOperator() != BinaryNode::Operator::kMember &&
      expr->GetOperator() != BinaryNode::Operator::kArrow)
    right = HandleExpr(right_expr, code);
  if (expr->GetOperator() != BinaryNode::Operator::kMember)
    left = HandleExpr(left_expr, code);
  // uint8_t right_type = GetExprVmType(right_expr);
  // uint8_t left_type = GetExprVmType(left_expr);
  // uint8_t result_type = left_type > right_type ? left_type : right_type;

  switch (expr->GetOperator()) {
    case BinaryNode::Operator::kAdd: {  // +
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kSub: {  // -
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kMul: {  // *
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kDiv: {  // /
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kRem: {  // %
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAnd: {  // &
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kOr: {  // |
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kXor: {  // ^
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShl: {  // <<
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kShr: {  // >>
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLT: {  // <
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x04,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kGT: {  // >
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x02,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kLE: {  // <=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x05,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kGE: {  // >=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x03,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kEQ: {  // ==
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x00,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kNE: {  // !=
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_CMP, 4, result, (std::size_t)0x01,
                              left, right));
      return result;
    }
    case BinaryNode::Operator::kLAnd: {  // &&
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kLOr: {  // ||
      std::size_t result = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, result, left, right));
      return result;
    }
    case BinaryNode::Operator::kAssign:  // =
      code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kAddAssign:  // +=
      code.push_back(Bytecode(_AQVM_OPERATOR_ADD, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kSubAssign:  // -=
      code.push_back(Bytecode(_AQVM_OPERATOR_SUB, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kMulAssign:  // *=
      code.push_back(Bytecode(_AQVM_OPERATOR_MUL, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kDivAssign:  // /=
      code.push_back(Bytecode(_AQVM_OPERATOR_DIV, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kRemAssign:  // %=
      code.push_back(Bytecode(_AQVM_OPERATOR_REM, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kAndAssign:  // &=
      code.push_back(Bytecode(_AQVM_OPERATOR_AND, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kOrAssign:  // |=
      code.push_back(Bytecode(_AQVM_OPERATOR_OR, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kXorAssign:  // ^=
      code.push_back(Bytecode(_AQVM_OPERATOR_XOR, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kShlAssign:  // <<=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHL, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kShrAssign:  // >>=
      code.push_back(Bytecode(_AQVM_OPERATOR_SHR, 3, left, left, right));
      /*if (IsDereferenced(left_expr)) {
        // std::cout << "dereferenced" << std::endl;
        code.push_back(
            Bytecode(_AQVM_OPERATOR_STORE, 2, dereference_ptr_index_, left));
      }*/
      return left;
    case BinaryNode::Operator::kMember: {
      return HandlePeriodExpr(expr, code);
      // std::cout << "Point A" << std::endl;
      //  std::string expr_type_string = GetExprTypeString(expr->GetLeftExpr());

      /*if (expr->GetRightExpr()->GetType() != StmtNode::StmtType::kIdentifier)
        EXIT_COMPILER(
            "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected expr type.");
      std::string full_name;

      // ExprNode* wait_handle_expr = nullptr;
      ExprNode* handle_expr = expr->GetLeftExpr();
      bool is_end = false;
      bool is_array = false;
      bool is_func = false;
      while(handle_expr != nullptr){
        if (dynamic_cast<BinaryNode*>(handle_expr)->GetOperator() !=
              BinaryNode::Operator::kMember){
            EXIT_COMPILER(
                "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                "Bytecode>&)",
                "Unexpected right expr type.");
          full_name =  std::string(*dynamic_cast<IdentifierNode*>(
              dynamic_cast<BinaryNode*>(handle_expr)->GetRightExpr()))+std::string(".")
      + full_name; handle_expr =
              dynamic_cast<BinaryNode*>(handle_expr)->GetLeftExpr();
        }else if(handle_expr->GetType() == StmtNode::StmtType::kIdentifier){
          full_name =
      (std::string)*dynamic_cast<IdentifierNode*>(handle_expr)+std::string(".")
      +full_name; handle_expr = nullptr;
        }
      }

      std::vector<ExprNode*> exprs;
      exprs.push_back(expr->GetRightExpr());
      while (handle_expr != nullptr) {
        if (handle_expr->GetType() == StmtNode::StmtType::kBinary) {
          if (dynamic_cast<BinaryNode*>(handle_expr)->GetOperator() !=
              BinaryNode::Operator::kMember)
            EXIT_COMPILER(
                "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                "Bytecode>&)",
                "Unexpected expr type.");
          exprs.insert(exprs.begin(),
                       dynamic_cast<BinaryNode*>(handle_expr)->GetRightExpr());
          handle_expr = dynamic_cast<BinaryNode*>(handle_expr)->GetLeftExpr();
        } else {
          exprs.insert(exprs.begin(), handle_expr);
          handle_expr = nullptr;
        }
      }

      std::size_t value_index = 0;
      std::size_t not_handle_index = 0;
      ExprNode* array_index = nullptr;
      for (std::size_t i = 0; i < exprs.size() && !is_end; i++) {
        if (exprs[i]->GetType() == StmtNode::StmtType::kArray) {
          ExprNode* current_expr = exprs[i];
          while (current_expr != nullptr) {
            if (current_expr->GetType() == StmtNode::StmtType::kArray) {
            } else {
            }
          }

          if (i == 0) {
            full_name += (std::string) *
                         dynamic_cast<IdentifierNode*>(
                             dynamic_cast<UnaryNode*>(exprs[i])->GetExpr());
          } else {
            full_name += std::string(".") +
                         (std::string) *
                             dynamic_cast<IdentifierNode*>(
                                 dynamic_cast<UnaryNode*>(exprs[i])->GetExpr());
          }
          for (int64_t k = current_scope_.size() - 1; k >= 0; k--) {
            auto iterator =
                var_decl_map_.find(current_scope_[k] + "#" + full_name);
            if (iterator != var_decl_map_.end()) {
              full_name = current_scope_[k] + "#" + full_name;
              is_end = true;
              not_handle_index = i + 1;
              break;
            }
            if (k == 0)
              EXIT_COMPILER(
                  "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                  "Bytecode>&)",
                  "Not found array.");
          }
          is_array = true;
          is_end = true;
          array_index = dynamic_cast<ArrayNode*>(exprs[i])->GetIndex();
          break;

        } else if (exprs[i]->GetType() == StmtNode::StmtType::kIdentifier) {
          if (i == 0) {
            full_name +=
                (std::string) * dynamic_cast<IdentifierNode*>(exprs[i]);
          } else {
            full_name +=
                std::string(".") +
                (std::string) * dynamic_cast<IdentifierNode*>(exprs[i]);
          }
        } else if (exprs[i]->GetType() == StmtNode::StmtType::kFunc) {
          if (i == 0) {
            full_name +=
                (std::string) * (dynamic_cast<FuncNode*>(exprs[i])->GetName());
          } else {
            full_name +=
                std::string(".") +
                (std::string) * (dynamic_cast<FuncNode*>(exprs[i])->GetName());
          }
          full_name +=
              std::string(".") +
              (std::string) * (dynamic_cast<FuncNode*>(exprs[i])->GetName());
          for (int64_t k = current_scope_.size() - 1; k >= 0; k--) {
            auto iterator =
                func_decl_map_.find(current_scope_[i] + "." + full_name);
            if (iterator != func_decl_map_.end()) {
              full_name = current_scope_[i] + "." + full_name;
              break;
            }
            if (k == 0)
              EXIT_COMPILER(
                  "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                  "Bytecode>&)",
                  "Function not found.");
          }
          is_end = true;
          not_handle_index = i + 1;
          is_func = true;
          break;
        } else {
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Unexpected expr.");
        }
        for (int64_t k = current_scope_.size() - 1; k >= 0; k--) {
          auto iterator =
              var_decl_map_.find(current_scope_[k] + "#" + full_name);
          if (iterator != var_decl_map_.end()) {
            full_name = current_scope_[k] + "#" + full_name;
            is_end = true;
            not_handle_index = i + 1;
          }
        }
      }*/

      // ExprNode* handle_expr = expr->GetRightExpr();

      /*while (handle_expr != nullptr && !is_end) {
        if (handle_expr->GetType() == StmtNode::StmtType::kBinary) {
          if (dynamic_cast<BinaryNode*>(handle_expr)->GetOperator() !=
              BinaryNode::Operator::kMember)
            EXIT_COMPILER(
                "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                "Bytecode>&)",
                "Unexpected right expr type.");
          full_name += std::string(".") +
      std::string(*dynamic_cast<IdentifierNode*>(
              dynamic_cast<BinaryNode*>(handle_expr)->GetLeftExpr()));
          handle_expr = dynamic_cast<IdentifierNode*>(
              dynamic_cast<BinaryNode*>(handle_expr)->GetRightExpr());
        } else if (handle_expr->GetType() == StmtNode::StmtType::kIdentifier) {
          full_name += std::string(".") +
      (std::string)*dynamic_cast<IdentifierNode*>(handle_expr); handle_expr =
      nullptr;

        } else if (handle_expr->GetType() == StmtNode::StmtType::kFunc) {
          full_name +=std::string(".") +
              (std::string) * (dynamic_cast<FuncNode*>(handle_expr)->GetName());
          for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
            auto iterator =
                func_decl_map_.find(current_scope_[i] + "." + full_name);
            if (iterator != func_decl_map_.end()) {
              full_name = current_scope_[i] + "." + full_name;
              break;
            }
            if (i == 0)
              EXIT_COMPILER(
                  "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                  "Bytecode>&)",
                  "Function not found.");
          }
          is_end = true;
          is_func = true;
          handle_expr = nullptr;
          break;
        }

        for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
          auto iterator =
              var_decl_map_.find(current_scope_[i] + "#" + full_name);
          if (iterator != var_decl_map_.end()) {
            full_name = current_scope_[i] + "#" + full_name;
            is_end = true;
          }
        }
      }
      if (!is_end)
        EXIT_COMPILER(
            "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
            "Bytecode>&)",
            "Unexpected Error.");

      // TODO(Scope)
      if (is_array) {
        if (not_handle_index < exprs.size()) {
        } else {
          std::size_t array_reference_index = global_memory_.Add(1);
          code.push_back(Bytecode(
              _AQVM_OPERATOR_ARRAY, 3, array_reference_index,
              var_decl_map_[full_name].second, HandleExpr(array_index, code)));
        }
      } else if (is_func) {
        if (not_handle_index < exprs.size()) {
          std::vector<ExprNode*> args =
              dynamic_cast<FuncNode*>(exprs[not_handle_index - 1])->GetArgs();
          std::vector<std::size_t> vm_args;
          std::size_t func_name_index = global_memory_.AddString(full_name);
          vm_args.push_back(func_name_index);
          vm_args.push_back(args.size() + 1);
          std::size_t return_value_index = global_memory_.Add(1);
          std::size_t return_value_reference_index = global_memory_.Add(1);
          code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                  return_value_reference_index,
                                  return_value_index));
          vm_args.push_back(return_value_reference_index);
          for (std::size_t i = 0; i < args.size(); i++) {
            vm_args.push_back(HandleExpr(args[i], code));
          }

          code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, vm_args));

          std::size_t current_index = return_value_index;
          while (not_handle_index < exprs.size()) {
            if (exprs[not_handle_index]->GetType() ==
                StmtNode::StmtType::kArray) {
              std::size_t array_reference_index = 0;
              code.push_back(Bytecode(_AQVM_OPERATOR_ARRAY, 3, ));
            } else if (exprs[not_handle_index]->GetType() ==
                       StmtNode::StmtType::kIdentifier) {
              std::size_t new_index = global_memory_.Add(1);
              code.push_back(Bytecode(
                  _AQVM_OPERATOR_LOAD_MEMBER, 3, new_index, current_index,
                  global_memory_.AddString(
                      (std::string) *
                      dynamic_cast<IdentifierNode*>(exprs[not_handle_index]))));
              current_index = new_index;
            } else if (exprs[current_index]->GetType() ==
                       StmtNode::StmtType::kFunc) {
              std::vector<ExprNode*> new_args =
                  dynamic_cast<FuncNode*>(exprs[not_handle_index])->GetArgs();
              std::vector<std::size_t> new_vm_args;
              std::size_t new_func_name_index = global_memory_.AddString(
                  (std::string) *
                  (dynamic_cast<FuncNode*>(exprs[not_handle_index])
                       ->GetName()));
              new_vm_args.push_back(current_index);
              new_vm_args.push_back(new_func_name_index);
              new_vm_args.push_back(new_args.size() + 1);
              std::size_t new_return_value_index = global_memory_.Add(1);
              std::size_t new_return_value_reference_index =
                  global_memory_.Add(1);
              code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                      new_return_value_reference_index,
                                      new_return_value_index));
              new_vm_args.push_back(new_return_value_reference_index);
              for (std::size_t i = 0; i < new_args.size(); i++) {
                new_vm_args.push_back(HandleExpr(new_args[i], code));
              }
              current_index = new_return_value_index;
              code.push_back(
                  Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, new_vm_args));
            } else {
              EXIT_COMPILER(
                  "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                  "Bytecode>&)",
                  "Unexpected expr type.");
            }
            not_handle_index++;
          }
          return current_index;

        } else {
          std::vector<ExprNode*> args =
              dynamic_cast<FuncNode*>(exprs[not_handle_index - 1])->GetArgs();
          std::vector<std::size_t> vm_args;
          std::size_t func_name_index = global_memory_.AddString(full_name);
          vm_args.push_back(func_name_index);
          vm_args.push_back(args.size() + 1);
          std::size_t return_value_index = global_memory_.Add(1);
          std::size_t return_value_reference_index = global_memory_.Add(1);
          code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                  return_value_reference_index,
                                  return_value_index));
          vm_args.push_back(return_value_reference_index);
          for (std::size_t i = 0; i < args.size(); i++) {
            vm_args.push_back(HandleExpr(args[i], code));
          }

          code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, vm_args));
          return return_value_index;
        }
      } else {
        if (not_handle_index < exprs.size()) {
          std::size_t current_index = var_decl_map_[full_name].second;
          while (not_handle_index < exprs.size()) {
            if (exprs[not_handle_index]->GetType() ==
                StmtNode::StmtType::kIdentifier) {
              std::size_t new_index = global_memory_.Add(1);
              code.push_back(Bytecode(
                  _AQVM_OPERATOR_LOAD_MEMBER, 3, new_index, current_index,
                  global_memory_.AddString(
                      (std::string) *
                      dynamic_cast<IdentifierNode*>(exprs[not_handle_index]))));
              current_index = new_index;
            } else if (exprs[current_index]->GetType() ==
                       StmtNode::StmtType::kFunc) {
              std::vector<ExprNode*> new_args =
                  dynamic_cast<FuncNode*>(exprs[not_handle_index])->GetArgs();
              std::vector<std::size_t> new_vm_args;
              std::size_t new_func_name_index = global_memory_.AddString(
                  (std::string) *
                  (dynamic_cast<FuncNode*>(exprs[not_handle_index])
                       ->GetName()));
              new_vm_args.push_back(current_index);
              new_vm_args.push_back(new_func_name_index);
              new_vm_args.push_back(new_args.size() + 1);
              std::size_t new_return_value_index = global_memory_.Add(1);
              std::size_t new_return_value_reference_index =
                  global_memory_.Add(1);
              code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                      new_return_value_reference_index,
                                      new_return_value_index));
              new_vm_args.push_back(new_return_value_reference_index);
              for (std::size_t i = 0; i < new_args.size(); i++) {
                new_vm_args.push_back(HandleExpr(new_args[i], code));
              }
              current_index = new_return_value_index;
              code.push_back(
                  Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, new_vm_args));
            } else {
              EXIT_COMPILER(
                  "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
                  "Bytecode>&)",
                  "Unexpected expr type.");
            }
            not_handle_index++;
          }
          return current_index;

        } else {
          return var_decl_map_[full_name].second;
        }
      }*/

      /*for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator =
            class_decl_map_.find(current_scope_[i] + "." + expr_type_string);
        if (iterator != class_decl_map_.end()) {
          expr_type_string = current_scope_[i] + "." + expr_type_string;
          break;
        }
        if (i == 0)
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class.");
      }

      if (expr->GetRightExpr()->GetType() == StmtNode::StmtType::kFunc) {
        std::cout << "Point A" << std::endl;
        std::string func_name =
            (std::string) *
            dynamic_cast<IdentifierNode*>(
                dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetName());
        std::vector<ExprNode*> args =
            dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs();
        std::vector<std::size_t> func_args;
        func_args.push_back(HandleExpr(expr->GetLeftExpr(), code));
        func_args.push_back(global_memory_.AddString(func_name));
        func_args.push_back(args.size() + 1);

        std::size_t return_value_index = global_memory_.Add(1);
        std::size_t return_value_ptr_index = global_memory_.Add(1);
        std::size_t return_value_reference_index = global_memory_.Add(1);
        code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, return_value_index,
                                return_value_ptr_index));
        code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                return_value_reference_index,
                                return_value_index));
        func_args.push_back(return_value_reference_index);

        for (std::size_t i = 0; i < args.size(); i++) {
          func_args.push_back(HandleExpr(args[i], code));
        }

        code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, func_args));
        return return_value_index;
      } else if (expr->GetRightExpr()->GetType() ==
                 StmtNode::StmtType::kIdentifier) {
        std::cout << "Point A1" << std::endl;
        std::cout << (std::string)(
                         *dynamic_cast<IdentifierNode*>(expr->GetRightExpr()))
                  << std::endl;
        std::size_t index = 0;
        if (class_decl_map_[expr_type_string] != nullptr &&
            class_decl_map_[expr_type_string]->GetVar(
                (std::string)(
                    *dynamic_cast<IdentifierNode*>(expr->GetRightExpr())),
                index)) {
          std::size_t return_index = global_memory_.Add(1);
          code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_index,
                                  HandleExpr(expr->GetLeftExpr(), code),
                                  index));

          return return_index;
        } else {
          std::cout << (std::string)(
                           *dynamic_cast<IdentifierNode*>(expr->GetRightExpr()))
                    << std::endl;
          EXIT_COMPILER(
              "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
              "Bytecode>&)",
              "Not found class member.");
        }
      } else {
        EXIT_COMPILER(
            "BytecodeGenerator::HandleBinaryExpr(BinaryNode*,std::vector<"
            "Bytecode>&)",
            "Unsupported stmt type.");
      }*/
      break;
    }

    case BinaryNode::Operator::kArrow:

    case BinaryNode::Operator::kComma:    // ,
                                          // std::cout << "Comma" << std::endl;
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

std::size_t BytecodeGenerator::HandlePeriodExpr(BinaryNode* expr,
                                                std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  std::cout << "HandlePeriodExpr CALLED." << std::endl;
  if (expr->GetOperator() != BinaryNode::Operator::kMember)
    EXIT_COMPILER(
        "BytecodeGenerator::HandlePeriodExpr(BinaryNode*,std::vector<Bytecode>&"
        ")",
        "Not period expr.");

  ExprNode* handle_expr = expr;
  std::vector<ExprNode*> exprs;
  // exprs.push_back(expr->GetRightExpr());
  while (handle_expr != nullptr) {
    if (handle_expr->GetType() == StmtNode::StmtType::kBinary) {
      if (dynamic_cast<BinaryNode*>(handle_expr)->GetOperator() !=
          BinaryNode::Operator::kMember)
        break;
      exprs.insert(exprs.begin(),
                   dynamic_cast<BinaryNode*>(handle_expr)->GetRightExpr());
      handle_expr = dynamic_cast<BinaryNode*>(handle_expr)->GetLeftExpr();
    } else {
      exprs.insert(exprs.begin(), handle_expr);
      handle_expr = nullptr;
    }
  }

  bool is_end = true;
  std::string full_name;
  for (std::size_t i = 0; i < exprs.size() - 1; i++) {
    if (exprs[i]->GetType() != StmtNode::StmtType::kIdentifier) {
      is_end = false;
      break;
    }
    full_name += (std::string) * dynamic_cast<IdentifierNode*>(exprs[i]) +
                 std::string(".");
  }
  if (is_end) {
    if (exprs.back()->GetType() == StmtNode::StmtType::kFunc) {
      full_name += *dynamic_cast<FuncNode*>(exprs.back())->GetName();
      for (int64_t k = current_scope_.size() - 1; k >= 0; k--) {
        auto iterator =
            func_decl_map_.find(current_scope_[k] + "." + full_name);
        if (iterator != func_decl_map_.end()) {
          full_name = current_scope_[k] + "." + full_name;
          std::size_t return_value_index = global_memory_.Add(1);
          std::size_t return_value_reference_index = global_memory_.Add(1);
          code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                                  return_value_reference_index,
                                  return_value_index));
          std::vector<std::size_t> args;
          args.push_back(2);
          args.push_back(global_memory_.AddString(full_name));
          args.push_back(
              dynamic_cast<FuncNode*>(exprs.back())->GetArgs().size() + 1);
          args.push_back(return_value_reference_index);
          for (std::size_t i = 0;
               i < dynamic_cast<FuncNode*>(exprs.back())->GetArgs().size();
               i++) {
            args.push_back(HandleExpr(
                dynamic_cast<FuncNode*>(exprs.back())->GetArgs()[i], code));
          }
          code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, args));
          return return_value_index;
        }
      }
    } else if (exprs.back()->GetType() == StmtNode::StmtType::kIdentifier) {
      full_name += *dynamic_cast<IdentifierNode*>(exprs.back());
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(current_scope_[i] + "." + full_name);
        if (iterator != var_decl_map_.end()) {
          return iterator->second.second;
        }
      }
    } else {
      EXIT_COMPILER(
          "BytecodeGenerator::HandlePeriodExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unsupported stmt type.");
    }
  }

  switch (expr->GetRightExpr()->GetType()) {
    case StmtNode::StmtType::kFunc: {
      std::size_t return_value_index = global_memory_.Add(1);
      std::size_t return_value_reference_index = global_memory_.Add(1);
      code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2,
                              return_value_reference_index,
                              return_value_index));
      std::vector<std::size_t> args;
      args.push_back(HandleExpr(expr->GetLeftExpr(), code));
      args.push_back(global_memory_.AddString(
          *dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetName()));
      args.push_back(
          dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs().size() + 1);
      args.push_back(return_value_reference_index);
      for (std::size_t i = 0;
           i < dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs().size();
           i++) {
        args.push_back(HandleExpr(
            dynamic_cast<FuncNode*>(expr->GetRightExpr())->GetArgs()[i], code));
      }
      code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, args));
      break;
    }
    case StmtNode::StmtType::kIdentifier: {
      std::size_t return_value_index = global_memory_.Add(1);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_value_index,
                   HandleExpr(expr->GetLeftExpr(), code),
                   global_memory_.AddString(
                       *dynamic_cast<IdentifierNode*>(expr->GetRightExpr()))));
      return return_value_index;
    }
    default:
      EXIT_COMPILER(
          "BytecodeGenerator::HandlePeriodExpr(BinaryNode*,std::vector<"
          "Bytecode>&)",
          "Unsupported expr.");
      break;
  }
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
    case StmtNode::StmtType::kImport:
      // HandleImport(dynamic_cast<ImportNode*>(stmt));
      break;

    case StmtNode::StmtType::kBreak:
      HandleBreakStmt(code);
      break;

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

    case StmtNode::StmtType::kDowhile:
      HandleDowhileStmt(dynamic_cast<DowhileNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kFor:
      HandleForStmt(dynamic_cast<ForNode*>(stmt), code);
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

    case StmtNode::StmtType::kClassDecl:
      HandleClassDecl(dynamic_cast<ClassDeclNode*>(stmt));
      break;

    case StmtNode::StmtType::kFunc:
      HandleFuncInvoke(dynamic_cast<FuncNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kReturn:
      HandleReturn(dynamic_cast<ReturnNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kLabel:
      HandleLabel(dynamic_cast<LabelNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kGoto:
      HandleGoto(dynamic_cast<GotoNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kSwitch:
      HandleSwitchStmt(dynamic_cast<SwitchNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kStmt:
      // std::cout << "STMT WARNING." << std::endl;
      break;

    default:
      EXIT_COMPILER(
          "BytecodeGenerator::HandleStmt(StmtNode*,std::vector<Bytecode>&)",
          "Unexpected code.");
      break;
  }
}

void BytecodeGenerator::HandleClassStmt(StmtNode* stmt,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleClassStmt(StmtNode*,std::vector<Bytecode>&)",
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

    case StmtNode::StmtType::kDowhile:
      HandleDowhileStmt(dynamic_cast<DowhileNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kFor:
      HandleForStmt(dynamic_cast<ForNode*>(stmt), code);
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

    case StmtNode::StmtType::kClassDecl:
      HandleClassDecl(dynamic_cast<ClassDeclNode*>(stmt));
      break;

    case StmtNode::StmtType::kFunc:
      HandleFuncInvoke(dynamic_cast<FuncNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kReturn:
      HandleReturn(dynamic_cast<ReturnNode*>(stmt), code);
      break;

    case StmtNode::StmtType::kLabel:
      HandleLabel(dynamic_cast<LabelNode*>(stmt), global_code_);
      break;

    case StmtNode::StmtType::kGoto:
      HandleGoto(dynamic_cast<GotoNode*>(stmt), global_code_);
      break;

    case StmtNode::StmtType::kStmt:
      // std::cout << "STMT WARNING." << std::endl;
      break;

    default:
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassStmt(StmtNode*,std::vector<Bytecode>&"
          ")",
          "Unexpected code.");
      break;
  }
}

void BytecodeGenerator::HandleBreakStmt(std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  std::size_t index = 0;
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64tWithoutValue(index)));
  // code.push_back(Bytecode(_AQVM_OPERATOR_GOTO,1,loop_break_index_.back()));

  loop_break_index_.push_back(index);
}

void BytecodeGenerator::HandleSwitchStmt(SwitchNode* stmt,
                                         std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  EXIT_COMPILER(
      "BytecodeGenerator::HandleSwitchStmt(SwitchNode*,std::vector<Bytecode>&)",
      "The switch statement is not yet supported.");
}

void BytecodeGenerator::HandleReturn(ReturnNode* stmt,
                                     std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  if (stmt->GetExpr() == nullptr) {
    exit_index_.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  } else {
    std::size_t return_value = HandleExpr(stmt->GetExpr(), code);

    bool is_find = false;
    auto return_iterator = var_decl_map_.find("#!return");
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      return_iterator = var_decl_map_.find(current_scope_[i] + "#" +
                                           static_cast<std::string>("!return"));
      if (return_iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
          "Not found identifier define.");

    is_find = false;
    auto return_reference_iterator = var_decl_map_.find("#!return_reference");
    for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
      return_reference_iterator =
          var_decl_map_.find(current_scope_[i] + "#" +
                             static_cast<std::string>("!return_reference"));
      if (return_reference_iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleReturn(ReturnNode*,std::vector<Bytecode>&)",
          "Not found identifier define.");

    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2,
                            return_iterator->second.second, return_value));
    code.push_back(Bytecode(_AQVM_OPERATOR_EQUAL, 2,
                            return_reference_iterator->second.second,
                            return_iterator->second.second));
    exit_index_.push_back(code.size());
    code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
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

  std::size_t if_location = code.size();

  // Need true branch and false branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t true_location = code.size();
  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();

  std::size_t goto_location = code.size();
  // Need exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
  std::size_t false_location = code.size();
  if (stmt->GetElseBody() != nullptr) {
    current_scope_.push_back(current_scope_.back() + "@@" +
                             std::to_string(++undefined_count_));
    HandleStmt(stmt->GetElseBody(), code);
    current_scope_.pop_back();
  }
  std::size_t exit_branch = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  std::vector<std::size_t> goto_args;
  if_args.push_back(condition_index);
  if_args.push_back(true_location);
  if_args.push_back(false_location);
  code[if_location].SetArgs(if_args);

  goto_args.push_back(global_memory_.AddUint64t(exit_branch));
  code[goto_location].SetArgs(goto_args);

  /*  // true branch name
    std::string true_name("$" + std::to_string(++undefined_name_count_));
    std::vector<Bytecode> true_code;
    HandleStmt(stmt->GetBody(), true_code);
    func_list_.push_back(Function(true_name, true_code));

    // true branch ptr
    std::size_t true_name_index =
        global_memory_.Add(0x01, true_name.size() + 1, true_name.c_str());
    std::size_t true_name_ptr_index = global_memory_.Add(0x06, 8);
    code.push_back(
        Bytecode(_AQVM_OPERATOR_PTR, true_name_index, true_name_ptr_index));

    if (stmt->GetElseBody() != nullptr) {
      // have else branch
      std::string false_name("$" + std::to_string(++undefined_name_count_));
      std::vector<Bytecode> false_code;
      HandleStmt(stmt->GetElseBody(), false_code);
      func_list_.push_back(Function(false_name, false_code));

      std::size_t false_name_index =
          global_memory_.Add(0x01, false_name.size() + 1, false_name.c_str());
      std::size_t false_name_ptr_index = global_memory_.Add(0x06, 8);
      code.push_back(
          Bytecode(_AQVM_OPERATOR_PTR, false_name_index, false_name_ptr_index));

      code.push_back(Bytecode(_AQVM_OPERATOR_IF, condition_index,
                              true_name_ptr_index, false_name_ptr_index));
    } else {
      // no else branch (void branch)
      std::size_t false_name_index = global_memory_.Add(0x01, 3, "$0");
      std::size_t false_name_ptr_index = global_memory_.Add(0x06, 8);
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

  loop_break_index_.push_back(-1);
  // std::size_t code_index = 0;
  // loop_break_index_.push_back(global_memory_.AddUint64tWithoutValue(code_index));

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  // Need body branch and exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t body_location = code.size();

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64t(start_location)));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    // std::vector<std::size_t> goto_arg;
    // goto_arg.push_back(global_memory_.AddUint64t(exit_location));
    // code[loop_break_index_.back()].SetArgs(goto_arg);
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();

  /*// condition branch
  std::string condition_name("$condition_" +
                             std::to_string(++undefined_name_count_));
  std::size_t condition_name_index = global_memory_.Add(
      0x01, condition_name.size() + 1, condition_name.c_str());
  std::size_t condition_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(Bytecode(_AQVM_OPERATOR_PTR, condition_name_index,
                          condition_name_ptr_index));

  // body branch
  std::string body_name("$body_" + std::to_string(++undefined_name_count_));
  std::size_t body_name_index =
      global_memory_.Add(0x01, body_name.size() + 1, body_name.c_str());
  std::size_t body_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_PTR, body_name_index, body_name_ptr_index));

  // exit branch
  std::string exit_name("$exit");
  std::size_t exit_name_index =
      global_memory_.Add(0x01, exit_name.size() + 1, exit_name.c_str());
  std::size_t exit_name_ptr_index = global_memory_.Add(0x06, 8);
  code.push_back(
      Bytecode(_AQVM_OPERATOR_PTR, exit_name_index, exit_name_ptr_index));

  // Generate body code
  std::vector<Bytecode> body_code;
  HandleStmt(stmt->GetBody(), body_code);
  body_code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, condition_name_ptr_index));

  std::vector<Bytecode> condition_code;
  condition_code.push_back(Bytecode(_AQVM_OPERATOR_IF, condition_index,
                                    body_name_ptr_index,
  exit_name_ptr_index));

  func_list_.push_back(Function(condition_name, condition_code));

  func_list_.push_back(Function(body_name, body_code));

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE, condition_name_ptr_index,
  0));
  */
}

void BytecodeGenerator::HandleDowhileStmt(DowhileNode* stmt,
                                          std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleDowhileStmt(DowhileNode*,std::vector<"
        "Bytecode>&)",
        "stmt is nullptr.");

  loop_break_index_.push_back(-1);

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  // std::size_t start_location = code.size();

  std::size_t body_location = code.size();

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));
  HandleStmt(stmt->GetBody(), code);
  current_scope_.pop_back();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));

  // code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1, start_location));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    // std::vector<std::size_t> goto_arg;
    // goto_arg.push_back(global_memory_.AddUint64t(exit_location));
    // code[loop_break_index_.back()].SetArgs(goto_arg);
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();
}
void BytecodeGenerator::HandleForStmt(ForNode* stmt,
                                      std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (stmt == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleWhileStmt(WhileNode*,std::vector<Bytecode>&)",
        "stmt is nullptr.");

  loop_break_index_.push_back(-1);

  current_scope_.push_back(current_scope_.back() + "@@" +
                           std::to_string(++undefined_count_));

  HandleStmt(stmt->GetStart(), code);

  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
  std::size_t start_location = code.size();

  std::size_t condition_index = HandleExpr(stmt->GetCondition(), code);

  std::size_t if_location = code.size();

  // Need body branch and exit branch
  code.push_back(Bytecode(_AQVM_OPERATOR_IF, 0));
  std::size_t body_location = code.size();

  HandleStmt(stmt->GetBody(), code);
  HandleExpr(stmt->GetEnd(), code);
  current_scope_.pop_back();
  code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                          global_memory_.AddUint64t(start_location)));

  std::size_t exit_location = code.size();
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));

  std::vector<std::size_t> if_args;
  if_args.push_back(condition_index);
  if_args.push_back(body_location);
  if_args.push_back(exit_location);
  code[if_location].SetArgs(if_args);

  while (loop_break_index_.back() != -1) {
    // std::vector<std::size_t> goto_arg;
    // goto_arg.push_back(global_memory_.AddUint64t(exit_location));
    // code[loop_break_index_.back()].SetArgs(goto_arg);
    global_memory_.SetUint64tValue(loop_break_index_.back(), exit_location);
    loop_break_index_.pop_back();
  }

  loop_break_index_.pop_back();
}

std::size_t BytecodeGenerator::HandleFuncInvoke(FuncNode* func,
                                                std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (func == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func is nullptr.");

  // std::cout << "A point" << std::endl;
  ExprNode* func_name_node = func->GetName();
  if (func_name_node == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
        "func_name_node is nullptr.");
  std::string func_name = static_cast<std::string>(*func_name_node);
  std::vector<ExprNode*> args = func->GetArgs();
  /*for (std::size_t i = 0; i < args.size(); i++) {
    if (i == 0) {
      func_name += "@";
    } else {
      func_name += ",";
    }

    func_name += GetExprTypeString(args[i]);
  }*/

  // FuncDeclNode func_decl;
  for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
    /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
              << std::endl;*/
    auto iterator = func_decl_map_.find(func_name);
    if (i != -1)
      iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
    if (iterator != func_decl_map_.end()) {
      func_name = func_name;
      if (i != -1) func_name = current_scope_[i] + "." + func_name;
      break;
    }
    // if(i!=-1)
    // std::cout<<"FN: "<<current_scope_[i] + "." + func_name<<std::endl;

    if (i == -1)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&"
          ")",
          "Function not found.");
  }

  /*Type* func_type = func_decl.GetReturnType();
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
          "BytecodeGenerator::HandleFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
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
      case Type::BaseType::kClass:
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
  }*/

  std::vector<std::size_t> vm_args;

  std::size_t func_name_index = global_memory_.AddString(func_name);

  vm_args.push_back(2);
  vm_args.push_back(func_name_index);
  vm_args.push_back(args.size() + 1);

  std::size_t return_value_index = global_memory_.Add(1);
  // std::size_t return_value_ptr_index = global_memory_.Add(1);
  std::size_t return_value_reference_index = global_memory_.Add(1);
  /*code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, return_value_index,
                          return_value_ptr_index));*/
  code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, return_value_reference_index,
                          return_value_index));
  vm_args.push_back(return_value_reference_index);

  for (std::size_t i = 0; i < args.size(); i++) {
    vm_args.push_back(HandleExpr(args[i], code));
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, vm_args));

  return return_value_index;
}

std::size_t BytecodeGenerator::HandleClassFuncInvoke(
    FuncNode* func, std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (func == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<"
        "Bytecode>&)",
        "func is nullptr.");

  // std::cout << "A point" << std::endl;
  ExprNode* func_name_node = func->GetName();
  if (func_name_node == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<"
        "Bytecode>&)",
        "func_name_node is nullptr.");
  std::string func_name = static_cast<std::string>(*func_name_node);
  std::vector<ExprNode*> args = func->GetArgs();
  /*for (std::size_t i = 0; i < args.size(); i++) {
  if (i == 0) {
  func_name += "@";
  } else {
  func_name += ",";
  }

  func_name += GetExprTypeString(args[i]);
  }*/

  // FuncDeclNode func_decl;
  for (int64_t i = current_scope_.size() - 1; i >= -1; i--) {
    /*std::cout << "func_name: " << current_scope_[i] + "::" + func_name
    << std::endl;*/
    auto iterator = func_decl_map_.find(func_name);
    if (i != -1)
      iterator = func_decl_map_.find(current_scope_[i] + "." + func_name);
    if (iterator != func_decl_map_.end()) {
      func_name = func_name;
      if (i != -1) func_name = current_scope_[i] + "." + func_name;
      break;
    }
    if (i == -1)
      EXIT_COMPILER(
          "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<"
          "Bytecode>&)",
          "Function not found.");
  }

  /*Type* func_type = func_decl.GetReturnType();
  if (func_type == nullptr)
  EXIT_COMPILER(
  "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<Bytecode>&)",
  "func_type is nullptr.");

  while (func_type->GetType() != Type::TypeType::kBase &&
  func_type->GetType() != Type::TypeType::kPointer &&
  func_type->GetType() != Type::TypeType::kArray &&
  func_type->GetType() != Type::TypeType::kReference) {
  if (func_type->GetType() == Type::TypeType::NONE)
  EXIT_COMPILER(
  "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<"
  "Bytecode>&)",
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
  case Type::BaseType::kClass:
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
  "BytecodeGenerator::HandleClassFuncInvoke(FuncNode*,std::vector<"
  "Bytecode>&)",
  "Unexpected code.");
  break;
  }
  } else if (func_type->GetType() == Type::TypeType::kPointer ||
  func_type->GetType() == Type::TypeType::kArray ||
  func_type->GetType() == Type::TypeType::kReference) {
  vm_type = 0x06;
  }*/

  std::vector<std::size_t> vm_args;

  std::size_t func_name_index = global_memory_.AddString(func_name);

  vm_args.push_back(2);
  vm_args.push_back(func_name_index);
  vm_args.push_back(args.size() + 1);

  std::size_t return_value_index = global_memory_.Add(1);
  std::size_t return_value_ptr_index = global_memory_.Add(1);
  std::size_t return_value_reference_index = global_memory_.Add(1);
  /*code.push_back(Bytecode(_AQVM_OPERATOR_PTR, 2, return_value_index,
                          return_value_ptr_index));*/
  code.push_back(Bytecode(_AQVM_OPERATOR_REFER, 2, return_value_reference_index,
                          return_value_index));
  vm_args.push_back(return_value_reference_index);

  for (std::size_t i = 0; i < args.size(); i++) {
    vm_args.push_back(HandleExpr(args[i], code));
  }

  code.push_back(Bytecode(_AQVM_OPERATOR_INVOKE_METHOD, vm_args));

  return return_value_index;
}

void BytecodeGenerator::HandleLabel(LabelNode* label,
                                    std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (label == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleLabel(LabelNode*,std::vector<Bytecode>&)",
        "label is nullptr.");

  std::string label_name =
      current_scope_.back() + "$" + std::string(label->GetLabel());

  if (label_map_.find(label_name) != label_map_.end())
    EXIT_COMPILER(
        "BytecodeGenerator::HandleLabel(LabelNode*,std::vector<Bytecode>&)",
        "Has found same name label.");
  label_map_.emplace(label_name, code.size());
  code.push_back(Bytecode(_AQVM_OPERATOR_NOP, 0));
}

void BytecodeGenerator::HandleGoto(GotoNode* label,
                                   std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (label == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleGoto(GotoNode*,std::vector<Bytecode>&)",
        "label is nullptr.");

  std::string label_name = std::string(label->GetLabel());

  for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
    auto iterator = label_map_.find(current_scope_[i] + "$" + label_name);
    if (iterator != label_map_.end()) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                              global_memory_.AddUint64t(iterator->second)));
      return;
    }
    if (i == current_func_index_) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
      goto_map_.push_back(
          std::pair<std::string, std::size_t>(label_name, code.size() - 1));
      return;
    }
  }
}

void BytecodeGenerator::HandleStartGoto(GotoNode* label,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (label == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::HandleStartGoto(GotoNode*,std::vector<Bytecode>&)",
        "label is nullptr.");

  std::string label_name = std::string(label->GetLabel());

  for (int64_t i = current_scope_.size() - 1; i >= current_func_index_; i--) {
    auto iterator = label_map_.find(current_scope_[i] + "$" + label_name);
    if (iterator != label_map_.end()) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 1,
                              global_memory_.AddUint64t(iterator->second)));
      return;
    }
    if (i == current_func_index_) {
      code.push_back(Bytecode(_AQVM_OPERATOR_GOTO, 0));
      start_goto_map_.push_back(
          std::pair<std::string, std::size_t>(label_name, code.size() - 1));
      return;
    }
  }
}

std::size_t BytecodeGenerator::GetClassIndex(ExprNode* expr,
                                             std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::GetClassIndex(ExprNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");

  switch (expr->GetType()) {
    case StmtNode::StmtType::kIdentifier: {
      std::size_t index = 0;
      if (current_class_ != nullptr &&
          current_class_->GetVar(
              (std::string)(*dynamic_cast<IdentifierNode*>(expr)), index)) {
        std::size_t return_index = global_memory_.Add(1);
        code.push_back(Bytecode(_AQVM_OPERATOR_LOAD_MEMBER, 3, return_index, 0,
                                global_memory_.AddString((std::string)(
                                    *dynamic_cast<IdentifierNode*>(expr)))));
        return return_index;
      }

      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(
            current_scope_[i] + "#" +
            static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
        if (iterator != var_decl_map_.end()) {
          /*std::cout << "Identifier: "
                    << (std::string) * dynamic_cast<IdentifierNode*>(expr)
                    << std::endl;*/

          return iterator->second.second;
        }
      }
      EXIT_COMPILER("BytecodeGenerator::GetClassIndex(ExprNode*)",
                    "Not found.");
      break;
    }

    case StmtNode::StmtType::kValue: {
      std::size_t vm_type = dynamic_cast<ValueNode*>(expr)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = dynamic_cast<ValueNode*>(expr)->GetByteValue();
          return global_memory_.AddByte(value);
          break;
        }

          /*case 0x02: {
            int value = dynamic_cast<ValueNode*>(expr)->GetIntValue();
            // std::cout << "value: " << value << std::endl;
            // value = is_big_endian_ ? value : SwapInt(value);
            return global_memory_.AddLong(value);
          }*/

        case 0x02: {
          int64_t value = dynamic_cast<ValueNode*>(expr)->GetLongValue();
          // value = is_big_endian_ ? value : SwapLong(value);
          return global_memory_.AddLong(value);
        }

          /*case 0x04: {
            float value = dynamic_cast<ValueNode*>(expr)->GetFloatValue();
            // value = is_big_endian_ ? value : SwapFloat(value);
            return global_memory_.AddDouble(value);
          }*/

        case 0x03: {
          double value = dynamic_cast<ValueNode*>(expr)->GetDoubleValue();
          // value = is_big_endian_ ? value : SwapDouble(value);
          return global_memory_.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = dynamic_cast<ValueNode*>(expr)->GetUInt64Value();
          // value = is_big_endian_ ? value : SwapUint64t(value);
          return global_memory_.AddUint64t(value);
        }

        case 0x05: {
          std::string value = dynamic_cast<ValueNode*>(expr)->GetStringValue();
          std::size_t str_index = global_memory_.AddString(value);
          return str_index;
        }

        default:
          EXIT_COMPILER(
              "BytecodeGenerator::GetClassIndex(ExprNode*,std::vector<Bytecode>"
              "&)",
              "Unexpected code.");
          break;
      }
    }

    case StmtNode::StmtType::kFunc:
      return HandleClassFuncInvoke(dynamic_cast<FuncNode*>(expr), code);

    default:
      return 0;
  }

  return 0;
}

std::size_t BytecodeGenerator::GetIndex(ExprNode* expr,
                                        std::vector<Bytecode>& code) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER(
        "BytecodeGenerator::GetIndex(ExprNode*,std::vector<Bytecode>&)",
        "expr is nullptr.");
  if (current_class_ != nullptr) {
    return GetClassIndex(expr, code);
  }

  switch (expr->GetType()) {
    case StmtNode::StmtType::kIdentifier: {
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        auto iterator = var_decl_map_.find(
            current_scope_[i] + "#" +
            static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
        std::cout << current_scope_[i] + "#" +
                         static_cast<std::string>(
                             *dynamic_cast<IdentifierNode*>(expr))
                  << std::endl;
        if (iterator != var_decl_map_.end()) {
          /*std::cout << "Identifier: "
          << (std::string) * dynamic_cast<IdentifierNode*>(expr)
          << std::endl;*/

          return iterator->second.second;
        }
      }
      EXIT_COMPILER("BytecodeGenerator::GetIndex(ExprNode*)", "Not found.");
      break;
    }

    case StmtNode::StmtType::kValue: {
      std::size_t vm_type = dynamic_cast<ValueNode*>(expr)->GetVmType();
      switch (vm_type) {
        case 0x01: {
          int8_t value = dynamic_cast<ValueNode*>(expr)->GetByteValue();
          return global_memory_.AddByte(value);
          break;
        }

          /*case 0x02: {
          int value = dynamic_cast<ValueNode*>(expr)->GetIntValue();
          // std::cout << "value: " << value << std::endl;
          // value = is_big_endian_ ? value : SwapInt(value);
          return global_memory_.AddLong(value);
          }*/

        case 0x02: {
          int64_t value = dynamic_cast<ValueNode*>(expr)->GetLongValue();
          // value = is_big_endian_ ? value : SwapLong(value);
          return global_memory_.AddLong(value);
        }

          /*case 0x04: {
          float value = dynamic_cast<ValueNode*>(expr)->GetFloatValue();
          // value = is_big_endian_ ? value : SwapFloat(value);
          return global_memory_.AddDouble(value);
          }*/

        case 0x03: {
          double value = dynamic_cast<ValueNode*>(expr)->GetDoubleValue();
          // value = is_big_endian_ ? value : SwapDouble(value);
          return global_memory_.AddDouble(value);
        }

        case 0x04: {
          uint64_t value = dynamic_cast<ValueNode*>(expr)->GetUInt64Value();
          // value = is_big_endian_ ? value : SwapUint64t(value);
          return global_memory_.AddUint64t(value);
        }

        case 0x05: {
          std::string value = dynamic_cast<ValueNode*>(expr)->GetStringValue();
          std::size_t str_index = global_memory_.AddString(value);
          return str_index;
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

  return 0;
}

std::size_t BytecodeGenerator::AddConstInt8t(int8_t value) {
  TRACE_FUNCTION;
  return global_memory_.AddByte(value);
}

/*[[deprecated]] uint8_t BytecodeGenerator::GetExprVmType(ExprNode* expr) {
  TRACE_FUNCTION;
  if (expr == nullptr)
    EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                  "expr is nullptr.");

  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    // std::cout << "Unary" << std::endl;
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kAddrOf) {
      return 0x06;
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::CONVERT) {
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
    // std::cout << "Binary" << std::endl;
    uint8_t left =
        GetExprVmType(dynamic_cast<BinaryNode*>(expr)->GetLeftExpr());
    uint8_t right =
        GetExprVmType(dynamic_cast<BinaryNode*>(expr)->GetRightExpr());

    return left > right ? left : right;
  }

  if (expr->GetType() == StmtNode::StmtType::kValue) {
    // std::cout << "Value" << std::endl;
    return dynamic_cast<ValueNode*>(expr)->GetVmType();
  }

  if (expr->GetType() == StmtNode::StmtType::kConditional) {
    // std::cout << "Conditional" << std::endl;
    uint8_t true_value =
        GetExprVmType(dynamic_cast<ConditionalNode*>(expr)->GetTrueExpr());
    uint8_t false_value =
        GetExprVmType(dynamic_cast<ConditionalNode*>(expr)->GetFalseExpr());

    return true_value > false_value ? true_value : false_value;
  }

  if (expr->GetType() == StmtNode::StmtType::kFunc) {
    // std::cout << "Func" << std::endl;
    auto iterator =
        func_decl_map_.find(*dynamic_cast<FuncNode*>(expr)->GetName());
    if (iterator == func_decl_map_.end())
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
    bool is_find = false;
    auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
    for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
      iterator = var_decl_map_.find(
          current_scope_[i] + "#" +
          static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
      if (iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
    // std::cout << "VarDecl" << std::endl;
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
  if (expr->GetType() == StmtNode::StmtType::kArrayDecl) {
    return 0x06;
  }
  if (expr->GetType() == StmtNode::StmtType::kCast) {
    // std::cout << "Cast" << std::endl;
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
  if (expr->GetType() == StmtNode::StmtType::kArray) {
    return GetExprPtrValueVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
  }
  // std::cout << "Unexpected" << std::endl;
  EXIT_COMPILER("BytecodeGenerator::GetExprVmType(ExprNode*)",
                "Unexpected code.");
  return 0;
}*/

/*[[deprecated]] uint8_t BytecodeGenerator::GetExprPtrValueVmType(ExprNode*
expr) { TRACE_FUNCTION; if (expr->GetType() == StmtNode::StmtType::kUnary) { if
(dynamic_cast<UnaryNode*>(expr)->GetOperator() == UnaryNode::Operator::kAddrOf)
{ return GetExprVmType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
    }
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::CONVERT) {
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
        func_decl_map_.find(*dynamic_cast<FuncNode*>(expr)->GetName());
    if (iterator == func_decl_map_.end())
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
              case Type::BaseType::kString:
              case Type::BaseType::kClass:
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
              case Type::BaseType::kString:
              case Type::BaseType::kClass:
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
              case Type::BaseType::kString:
              case Type::BaseType::kClass:
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
      bool is_find = false;
      auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
      for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
        iterator = var_decl_map_.find(
            current_scope_[i] + "#" +
            static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
        if (iterator != var_decl_map_.end()) {
          is_find = true;
          break;
        }
      }
      if (!is_find)
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
          bool is_find = false;
          auto iterator =
              var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
          for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
            iterator = var_decl_map_.find(
                current_scope_[i] + "#" +
                static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
            if (iterator != var_decl_map_.end()) {
              is_find = true;
              break;
            }
          }
          if (!is_find)
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
          bool is_find = false;
          auto iterator =
              var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
          for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
            iterator = var_decl_map_.find(
                current_scope_[i] + "#" +
                static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
            if (iterator != var_decl_map_.end()) {
              is_find = true;
              break;
            }
          }
          if (!is_find)
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
          bool is_find = false;
          auto iterator =
              var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
          for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
            iterator = var_decl_map_.find(
                current_scope_[i] + "#" +
                static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
            if (iterator != var_decl_map_.end()) {
              is_find = true;
              break;
            }
          }
          if (!is_find)
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
                case Type::BaseType::kString:
                case Type::BaseType::kClass:
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
          case Type::BaseType::kString:
          case Type::BaseType::kClass:
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
}*/

/*[[deprecated]] uint8_t BytecodeGenerator::ConvertTypeToVmType(Type* type) {
  TRACE_FUNCTION;
  switch (type->GetType()) {
    case Type::TypeType::kBase:
    case Type::TypeType::kConst:
      switch (type->GetBaseType()) {
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
        case Type::BaseType::kString:
        case Type::BaseType::kClass:
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
      switch (type->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (dynamic_cast<ArrayType*>(type)->GetSubType()->GetBaseType()) {
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
      switch (type->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (
              dynamic_cast<PointerType*>(type)->GetSubType()->GetBaseType()) {
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
      switch (type->GetType()) {
        case Type::TypeType::kBase:
        case Type::TypeType::kConst:
          switch (
              dynamic_cast<ReferenceType*>(type)->GetSubType()->GetBaseType()) {
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
            case Type::BaseType::kString:
            case Type::BaseType::kClass:
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
}*/

/*[[deprecated]] std::size_t BytecodeGenerator::GetExprVmSize(uint8_t type) {
  TRACE_FUNCTION;
  switch (type) {
    case 0x00:
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
}*/

/*[[deprecated]] int BytecodeGenerator::SwapInt(int x) {
  TRACE_FUNCTION;
  uint32_t ux = (uint32_t)x;
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  return (int)ux;
}*/

int64_t BytecodeGenerator::SwapLong(int64_t x) {
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
  return (int64_t)ux;
}

/*[[deprecated]] float BytecodeGenerator::SwapFloat(float x) {
  TRACE_FUNCTION;
  uint32_t ux;
  memcpy(&ux, &x, sizeof(uint32_t));
  ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
       ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
  float result;
  memcpy(&result, &ux, sizeof(float));
  return result;
}*/

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

std::size_t BytecodeGenerator::EncodeUleb128(std::size_t value,
                                             std::vector<uint8_t>& output) {
  TRACE_FUNCTION;
  std::size_t count = 0;
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

Type* BytecodeGenerator::GetExprType(ExprNode* expr) {
  if (expr->GetType() == StmtNode::StmtType::kArray) {
    bool is_find = false;
    auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
    for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
      iterator = var_decl_map_.find(
          current_scope_[i] + "#" +
          static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
      if (iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Not found array.");

    ArrayDeclNode* array_decl = (ArrayDeclNode*)iterator->second.second;
    if (array_decl->GetVarType()->GetType() == Type::TypeType::kArray) {
      return dynamic_cast<ArrayType*>(array_decl->GetVarType())->GetSubType();
    } else if (array_decl->GetVarType()->GetType() ==
               Type::TypeType::kPointer) {
      return dynamic_cast<PointerType*>(array_decl->GetVarType())->GetSubType();
    } else {
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Unknown type.");
    }
  } else if (expr->GetType() == StmtNode::StmtType::kArrayDecl) {
    return dynamic_cast<ArrayDeclNode*>(expr)->GetVarType();
  } else if (expr->GetType() == StmtNode::StmtType::kIdentifier) {
    bool is_find = false;
    auto iterator = var_decl_map_.find(*dynamic_cast<IdentifierNode*>(expr));
    for (int64_t i = current_scope_.size() - 1; i >= 0; i--) {
      iterator = var_decl_map_.find(
          current_scope_[i] + "#" +
          static_cast<std::string>(*dynamic_cast<IdentifierNode*>(expr)));
      if (iterator != var_decl_map_.end()) {
        is_find = true;
        break;
      }
    }
    if (!is_find)
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Not found variable.");

    return iterator->second.first->GetVarType();
  } else if (expr->GetType() == StmtNode::StmtType::kUnary) {
    switch (dynamic_cast<UnaryNode*>(expr)->GetOperator()) {
      case UnaryNode::Operator::kPostInc:
      case UnaryNode::Operator::kPostDec:
      case UnaryNode::Operator::kPreInc:
      case UnaryNode::Operator::kPreDec:
      case UnaryNode::Operator::kPlus:
      case UnaryNode::Operator::kMinus:
      case UnaryNode::Operator::kNot:
      case UnaryNode::Operator::kBitwiseNot:
        return GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr());
      case UnaryNode::Operator::kAddrOf: {
        PointerType* ptr = new PointerType();
        ptr->SetSubType(GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()));
        return ptr;
      }
      case UnaryNode::Operator::kDeref:
        if (GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr())->GetType() ==
            Type::TypeType::kPointer) {
          return dynamic_cast<PointerType*>(
                     GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()))
              ->GetSubType();
        } else if (GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr())
                       ->GetType() == Type::TypeType::kArray) {
          return dynamic_cast<ArrayType*>(
                     GetExprType(dynamic_cast<UnaryNode*>(expr)->GetExpr()))
              ->GetSubType();
        } else {
          EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                        "Unknown type.");
        }
      case UnaryNode::Operator::ARRAY:
        EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                      "Unexpected code.");
      /*case UnaryNode::Operator::CONVERT:
        return dynamic_cast<CastNode*>(expr)->GetCastType();*/
      default:
        EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                      "Unexpected code.");
    }
  } else if (expr->GetType() == StmtNode::StmtType::kBinary) {
    Type* left = GetExprType(dynamic_cast<BinaryNode*>(expr)->GetLeftExpr());
    Type* right = GetExprType(dynamic_cast<BinaryNode*>(expr)->GetRightExpr());

    if (dynamic_cast<BinaryNode*>(expr)->GetOperator() ==
        BinaryNode::Operator::kDiv) {
      return Type::CreateDoubleType();
    }

    if (left->GetType() == Type::TypeType::kConst)
      left = dynamic_cast<ConstType*>(left)->GetSubType();
    if (right->GetType() == Type::TypeType::kConst)
      right = dynamic_cast<ConstType*>(right)->GetSubType();
    if (left->GetType() == Type::TypeType::kReference)
      left = dynamic_cast<ReferenceType*>(left)->GetSubType();
    if (right->GetType() == Type::TypeType::kReference)
      right = dynamic_cast<ReferenceType*>(right)->GetSubType();
    if (left->GetType() == Type::TypeType::NONE ||
        right->GetType() == Type::TypeType::NONE)
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Unknown Type.");

    if (left->GetType() == Type::TypeType::kPointer ||
        right->GetType() == Type::TypeType::kPointer ||
        left->GetType() == Type::TypeType::kArray ||
        right->GetType() == Type::TypeType::kArray) {
      if (left->GetType() == Type::TypeType::kPointer ||
          left->GetType() == Type::TypeType::kArray)
        return left;
      if (right->GetType() == Type::TypeType::kPointer ||
          right->GetType() == Type::TypeType::kArray)
        return right;
    }

    if (left->GetType() == Type::TypeType::kClass) return left;
    if (right->GetType() == Type::TypeType::kClass) return right;

    if (left->GetBaseType() == right->GetBaseType()) return left;

    if (left->GetSize() > right->GetSize()) {
      return left;
    } else if (left->GetSize() < right->GetSize()) {
      return right;
    } else {
      int left_priority = 0;
      int right_priority = 0;
      switch (left->GetBaseType()) {
        case Type::BaseType::kVoid:
          left_priority = 0;
          break;
        case Type::BaseType::kBool:
          left_priority = 1;
          break;
        case Type::BaseType::kChar:
          left_priority = 2;
          break;
        case Type::BaseType::kShort:
          left_priority = 3;
          break;
        case Type::BaseType::kInt:
          left_priority = 4;
          break;
        case Type::BaseType::kFloat:
          left_priority = 5;
          break;
        case Type::BaseType::kLong:
          left_priority = 6;
          break;
        case Type::BaseType::kDouble:
          left_priority = 7;
          break;
        case Type::BaseType::kString:
          left_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          left_priority = 9;
          break;
        case Type::BaseType::kPointer:
          left_priority = 10;
          break;
        case Type::BaseType::kArray:
          left_priority = 11;
          break;
        case Type::BaseType::kEnum:
          left_priority = 12;
          break;
        case Type::BaseType::kUnion:
          left_priority = 13;
          break;
        case Type::BaseType::kStruct:
          left_priority = 14;
          break;
        case Type::BaseType::kClass:
          left_priority = 15;
          break;
        case Type::BaseType::kFunction:
          left_priority = 16;
          break;
        case Type::BaseType::kAuto:
          left_priority = 17;
          break;
        default:
          EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                        "Unknown type.");
      }
      switch (right->GetBaseType()) {
        case Type::BaseType::kVoid:
          right_priority = 0;
          break;
        case Type::BaseType::kBool:
          right_priority = 1;
          break;
        case Type::BaseType::kChar:
          right_priority = 2;
          break;
        case Type::BaseType::kShort:
          right_priority = 3;
          break;
        case Type::BaseType::kInt:
          right_priority = 4;
          break;
        case Type::BaseType::kFloat:
          right_priority = 5;
          break;
        case Type::BaseType::kLong:
          right_priority = 6;
          break;
        case Type::BaseType::kDouble:
          right_priority = 7;
          break;
        case Type::BaseType::kString:
          right_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          right_priority = 9;
          break;
        case Type::BaseType::kPointer:
          right_priority = 10;
          break;
        case Type::BaseType::kArray:
          right_priority = 11;
          break;
        case Type::BaseType::kEnum:
          right_priority = 12;
          break;
        case Type::BaseType::kUnion:
          right_priority = 13;
          break;
        case Type::BaseType::kStruct:
          right_priority = 14;
          break;
        case Type::BaseType::kClass:
          right_priority = 15;
          break;
        case Type::BaseType::kFunction:
          right_priority = 16;
          break;
        case Type::BaseType::kAuto:
          right_priority = 17;
          break;
        default:
          EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                        "Unknown type.");
      }
      if (left_priority > right_priority) return left;
      if (left_priority < right_priority) return right;
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Unexpected code.");
    }

  } else if (expr->GetType() == StmtNode::StmtType::kFunc) {
    return dynamic_cast<FuncDeclNode*>(expr)->GetReturnType();
  } else if (expr->GetType() == StmtNode::StmtType::kVarDecl) {
    return dynamic_cast<VarDeclNode*>(expr)->GetVarType();
  } else if (expr->GetType() == StmtNode::StmtType::kValue) {
    return dynamic_cast<ValueNode*>(expr)->GetValueType();
  } else if (expr->GetType() == StmtNode::StmtType::kConditional) {
    Type* true_expr =
        GetExprType(dynamic_cast<ConditionalNode*>(expr)->GetTrueExpr());
    Type* false_expr =
        GetExprType(dynamic_cast<ConditionalNode*>(expr)->GetFalseExpr());

    if (true_expr->GetType() == Type::TypeType::kConst)
      true_expr = dynamic_cast<ConstType*>(true_expr)->GetSubType();
    if (false_expr->GetType() == Type::TypeType::kConst)
      false_expr = dynamic_cast<ConstType*>(false_expr)->GetSubType();
    if (true_expr->GetType() == Type::TypeType::kReference)
      true_expr = dynamic_cast<ReferenceType*>(true_expr)->GetSubType();
    if (false_expr->GetType() == Type::TypeType::kReference)
      false_expr = dynamic_cast<ReferenceType*>(false_expr)->GetSubType();
    if (true_expr->GetType() == Type::TypeType::NONE ||
        false_expr->GetType() == Type::TypeType::NONE)
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Unknown Type.");

    if (true_expr->GetType() == Type::TypeType::kPointer ||
        false_expr->GetType() == Type::TypeType::kPointer ||
        true_expr->GetType() == Type::TypeType::kArray ||
        false_expr->GetType() == Type::TypeType::kArray) {
      if (true_expr->GetType() == Type::TypeType::kPointer ||
          true_expr->GetType() == Type::TypeType::kArray)
        return true_expr;
      if (false_expr->GetType() == Type::TypeType::kPointer ||
          false_expr->GetType() == Type::TypeType::kArray)
        return false_expr;
    }

    if (true_expr->GetType() == Type::TypeType::kClass) return true_expr;
    if (false_expr->GetType() == Type::TypeType::kClass) return false_expr;

    if (true_expr->GetBaseType() == false_expr->GetBaseType()) return true_expr;

    if (true_expr->GetSize() > false_expr->GetSize()) {
      return true_expr;
    } else if (true_expr->GetSize() < false_expr->GetSize()) {
      return false_expr;
    } else {
      int true_expr_priority = 0;
      int false_expr_priority = 0;
      switch (true_expr->GetBaseType()) {
        case Type::BaseType::kVoid:
          true_expr_priority = 0;
          break;
        case Type::BaseType::kBool:
          true_expr_priority = 1;
          break;
        case Type::BaseType::kChar:
          true_expr_priority = 2;
          break;
        case Type::BaseType::kShort:
          true_expr_priority = 3;
          break;
        case Type::BaseType::kInt:
          true_expr_priority = 4;
          break;
        case Type::BaseType::kFloat:
          true_expr_priority = 5;
          break;
        case Type::BaseType::kLong:
          true_expr_priority = 6;
          break;
        case Type::BaseType::kDouble:
          true_expr_priority = 7;
          break;
        case Type::BaseType::kString:
          true_expr_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          true_expr_priority = 9;
          break;
        case Type::BaseType::kPointer:
          true_expr_priority = 10;
          break;
        case Type::BaseType::kArray:
          true_expr_priority = 11;
          break;
        case Type::BaseType::kEnum:
          true_expr_priority = 12;
          break;
        case Type::BaseType::kUnion:
          true_expr_priority = 13;
          break;
        case Type::BaseType::kStruct:
          true_expr_priority = 14;
          break;
        case Type::BaseType::kClass:
          true_expr_priority = 15;
          break;
        case Type::BaseType::kFunction:
          true_expr_priority = 16;
          break;
        case Type::BaseType::kAuto:
          true_expr_priority = 17;
          break;
        default:
          EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                        "Unknown type.");
      }
      switch (false_expr->GetBaseType()) {
        case Type::BaseType::kVoid:
          false_expr_priority = 0;
          break;
        case Type::BaseType::kBool:
          false_expr_priority = 1;
          break;
        case Type::BaseType::kChar:
          false_expr_priority = 2;
          break;
        case Type::BaseType::kShort:
          false_expr_priority = 3;
          break;
        case Type::BaseType::kInt:
          false_expr_priority = 4;
          break;
        case Type::BaseType::kFloat:
          false_expr_priority = 5;
          break;
        case Type::BaseType::kLong:
          false_expr_priority = 6;
          break;
        case Type::BaseType::kDouble:
          false_expr_priority = 7;
          break;
        case Type::BaseType::kString:
          false_expr_priority = 8;
          break;
        case Type::BaseType::kTypedef:
          false_expr_priority = 7;
          break;
        case Type::BaseType::kPointer:
          false_expr_priority = 10;
          break;
        case Type::BaseType::kArray:
          false_expr_priority = 11;
          break;
        case Type::BaseType::kEnum:
          false_expr_priority = 12;
          break;
        case Type::BaseType::kUnion:
          false_expr_priority = 13;
          break;
        case Type::BaseType::kStruct:
          false_expr_priority = 14;
          break;
        case Type::BaseType::kClass:
          false_expr_priority = 15;
          break;
        case Type::BaseType::kFunction:
          false_expr_priority = 16;
          break;
        case Type::BaseType::kAuto:
          false_expr_priority = 17;
          break;
        default:
          EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                        "Unknown type.");
      }
      if (true_expr_priority > false_expr_priority) return true_expr;
      if (true_expr_priority < false_expr_priority) return false_expr;
      EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                    "Unexpected code.");
    }
  } /*else if (expr->GetType() == StmtNode::StmtType::kCast) {
    return dynamic_cast<CastNode*>(expr)->GetCastType();
  } */
  else {
    EXIT_COMPILER("BytecodeGenerator::GetExprType(ExprNode*)",
                  "Unsupport type.");
  }
  return nullptr;
}
std::string BytecodeGenerator::GetExprTypeString(ExprNode* expr) {
  Type* type = GetExprType(expr);
  if (type == nullptr)
    EXIT_COMPILER("BytecodeGenerator::GetExprTypeString(ExprNode*)",
                  "type is nullptr.");
  if (type->GetType() == Type::TypeType::NONE)
    EXIT_COMPILER("BytecodeGenerator::GetExprTypeString(ExprNode*)",
                  "Unknown type.");
  if (type->GetType() == Type::TypeType::kConst)
    return *dynamic_cast<ConstType*>(type);
  if (type->GetType() == Type::TypeType::kPointer)
    return *dynamic_cast<PointerType*>(type);
  if (type->GetType() == Type::TypeType::kArray)
    return *dynamic_cast<ArrayType*>(type);
  if (type->GetType() == Type::TypeType::kReference)
    return *dynamic_cast<ReferenceType*>(type);
  if (type->GetType() == Type::TypeType::kClass)
    return *dynamic_cast<ClassType*>(type);
  if (type->GetType() == Type::TypeType::kBase) return *type;

  EXIT_COMPILER("BytecodeGenerator::GetExprTypeString(ExprNode*)",
                "Unexpected code.");

  return std::string();
}

bool BytecodeGenerator::IsDereferenced(ExprNode* expr) {
  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    if (dynamic_cast<UnaryNode*>(expr)->GetOperator() ==
        UnaryNode::Operator::kDeref) {
      return true;
    }
  } else if (expr->GetType() == StmtNode::StmtType::kArray) {
    return true;
  }
  return false;
}

}  // namespace Compiler
}  // namespace Aq

int main(int argc, char* argv[]) {
  auto start_time = std::chrono::high_resolution_clock::now();

  if (argc < 3) {
    printf("Usage: %s <filename> <output>\n", argv[0]);
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
  lexer.LexToken(first_token_buffer, first_token_buffer);
  token.push_back(first_token_buffer);
  while (true) {
    Aq::Compiler::Token token_buffer;
    lexer.LexToken(token.back(), token_buffer);
    token.push_back(token_buffer);
    if (lexer.IsReadEnd()) {
      break;
    }
  }

  // std::cout << "Lex End." << std::endl;

  Aq::Compiler::CompoundNode* ast = Aq::Compiler::Parser::Parse(token);

  if (ast == nullptr) EXIT_COMPILER("main(int,char**)", "ast is nullptr\n");

  // std::cout << "Parse End." << std::endl;

  Aq::Compiler::BytecodeGenerator bytecode_generator;

  bytecode_generator.GenerateBytecode(ast, argv[2]);

  std::cout << "[INFO] " << "Generate Bytecode SUCCESS!" << std::endl;

  auto end_time = std::chrono::high_resolution_clock::now();
  auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  std::cout << "[INFO] " << "Execution time: " << time_diff.count() << " ms"
            << std::endl;

  return 0;
}