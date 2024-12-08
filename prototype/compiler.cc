// Copyright 2024 AQ author, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
    std::size_t init_capacity = 1024;
    pair_list_ = new PairList[init_capacity];
    if (!pair_list_) {
      pair_list_ = new PairList[1];
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
    if (size_ / capacity_ > 0.8) {
      Resize();
    }

    // Create key-value pairs and insert them into the linked list.
    Pair pair;
    pair.key = key;
    pair.value = value;
    pair_list_[hash].Prepend(pair);
  };

  // Find the value of a key.
  T Find(std::string key) {
    unsigned int hash = Hash(key);
    return pair_list_[hash].Find(key);
  };

 private:
  struct Pair {
    std::string key;
    T value;
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
    void CopyDataToNewList(PairList* new_list, size_t new_capacity) {
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
      return static_cast<T>(0);
    };

   private:
    // A node type of the linked list.
    struct Node {
      Pair data;
      Node* next = nullptr;
      Node(Pair pair) : data(pair) {};
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
    size_t length;
  };
  union Value {
    ValueStr number;
    KeywordType keyword;
    ValueStr identifier;
    OperatorType _operator;
    ValueStr character;
    ValueStr string;
  };

  Type type = Type::START;
  Value value;

  Token();
  ~Token();

  Token(const Token&) = default;
  Token(Token&&) noexcept = default;
  Token& operator=(const Token&) = default;
  Token& operator=(Token&&) noexcept = default;

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
        os << std::string(token.value.character.location,
                          token.value.character.length);
        break;
      case Token::Type::STRING:
        os << std::string(token.value.string.location,
                          token.value.string.length);
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

Token::Token() = default;
Token::~Token() = default;

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
  Lexer(char* source_code, size_t length)
      : buffer_ptr_(source_code), buffer_end_(source_code + length - 1) {};
  ~Lexer() = default;

  Lexer(const Lexer&) = default;
  Lexer(Lexer&&) noexcept = default;
  Lexer& operator=(const Lexer&) = default;
  Lexer& operator=(Lexer&&) noexcept = default;

  // Lexical analysis |buffer_ptr_|, and store the analyzed token into
  // |return_token|.
  int LexToken(Token& return_token);

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

int Lexer::LexToken(Token& return_token) {
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
        if (*(read_ptr + 1) == '0' || *(read_ptr + 1) == '1' ||
            *(read_ptr + 1) == '2' || *(read_ptr + 1) == '3' ||
            *(read_ptr + 1) == '4' || *(read_ptr + 1) == '5' ||
            *(read_ptr + 1) == '6' || *(read_ptr + 1) == '7' ||
            *(read_ptr + 1) == '8' || *(read_ptr + 1) == '9') {
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
    size_t length = read_ptr - buffer_ptr_;
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
        return_token.value.character = value;
        break;

      case Token::Type::STRING:
        return_token.value.string = value;
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
  }
  virtual void SetType(BaseType type) { base_data_ = type; }
  virtual ~Type() = default;

  Type(const Type&) = default;
  Type& operator=(const Type&) = default;

  TypeType GetType() { return type_; }

  BaseType GetBaseType() { return base_data_; }

  static Type* CreateType(Token* token, size_t length, size_t& index);

  virtual size_t GetSize() {
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

  ValueNode(const ValueNode&) = default;
  ValueNode& operator=(const ValueNode&) = default;

 private:
  Token value_;
};

class UnaryNode : public ExprNode {
 public:
  enum class Operator {
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

  UnaryNode() { type_ = StmtType::kUnary; }
  void SetUnaryNode(Operator op, ExprNode* expr) {
    op_ = op;
    expr_ = expr;
  }

  Operator GetOperator() { return op_; }

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
  }
  void SetArrayNode(ExprNode* expr, ExprNode* index) {
    expr_ = expr;
    index_ = index;
  }
  virtual ~ArrayNode() = default;

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
  }
  void SetConvertNode(Type* type, ExprNode* expr) {
    converted_type_ = type;
    expr_ = expr;
  }
  virtual ~ConvertNode() = default;

  ConvertNode(const ConvertNode&) = default;
  ConvertNode& operator=(const ConvertNode&) = default;

 private:
  Type* converted_type_;
};

class BinaryNode : public ExprNode {
 public:
  enum class Operator {
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

  BinaryNode() { type_ = StmtType::kBinary; }
  void SetBinaryNode(Operator op, ExprNode* left, ExprNode* right) {
    op_ = op;
    left_ = left;
    right_ = right;
  }
  virtual ~BinaryNode() = default;

  BinaryNode(const BinaryNode&) = default;
  BinaryNode& operator=(const BinaryNode&) = default;

 private:
  Operator op_;
  ExprNode* left_;
  ExprNode* right_;
};

class ConditionalNode : public ExprNode {
 public:
  ConditionalNode() { type_ = StmtType::kConditional; }
  void SetConditionalNode(ExprNode* condition, ExprNode* true_expr,
                          ExprNode* false_expr) {
    condition_ = condition;
    true_expr_ = true_expr;
    false_expr_ = false_expr;
  }
  virtual ~ConditionalNode() = default;

  ConditionalNode(const ConditionalNode&) = default;
  ConditionalNode& operator=(const ConditionalNode&) = default;

 private:
  ExprNode* condition_;
  ExprNode* true_expr_;
  ExprNode* false_expr_;
};

class FuncNode : public ExprNode {
 public:
  FuncNode() { type_ = StmtType::kFunc; }
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
  VarDeclNode() { DeclNode::type_ = StmtType::kVarDecl; }
  void SetVarDeclNode(Type* type, ExprNode* name) {
    var_type_ = type;
    name_ = name;
    value_.push_back(nullptr);
  }
  void SetVarDeclNode(Type* type, ExprNode* name, ExprNode* value) {
    var_type_ = type;
    name_ = name;
    value_.push_back(value);
  }

  virtual ~VarDeclNode() = default;

  VarDeclNode(const VarDeclNode&) = default;
  VarDeclNode& operator=(const VarDeclNode&) = default;

  Type* GetType() { return var_type_; }
  ExprNode* GetName() { return name_; }
  std::vector<ExprNode*> GetValue() { return value_; }

 protected:
  Type* var_type_;
  ExprNode* name_;
  std::vector<ExprNode*> value_;
};

class ArrayDeclNode : public VarDeclNode {
 public:
  ArrayDeclNode() { DeclNode::type_ = StmtType::kArrayDecl; }
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
  FuncDeclNode() { type_ = StmtType::kFuncDecl; }
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
  IfNode() { type_ = StmtType::kIf; }
  void SetIfNode(ExprNode* condition, StmtNode* body) {
    condition_ = condition;
    body_ = body;
  }

  void SetIfNode(ExprNode* condition, StmtNode* body, StmtNode* else_body) {
    condition_ = condition;
    body_ = body;
    else_body_ = else_body;
  }

  IfNode(const IfNode&) = default;
  IfNode& operator=(const IfNode&) = default;

 private:
  ExprNode* condition_;
  StmtNode* body_;
  StmtNode* else_body_;
};

class WhileNode : public StmtNode {
 public:
  WhileNode() { type_ = StmtType::kWhile; }
  virtual ~WhileNode() = default;

  void SetWhileNode(ExprNode* condition, StmtNode* body) {
    condition_ = condition;
    body_ = body;
  }

  WhileNode(const WhileNode&) = default;
  WhileNode& operator=(const WhileNode&) = default;

 private:
  ExprNode* condition_;
  StmtNode* body_;
};

class CastNode : public ExprNode {
 public:
  CastNode() { type_ = StmtType::kCast; }
  void SetCastNode(Type* type, ExprNode* expr) {
    cast_type_ = type;
    expr_ = expr;
  }
  virtual ~CastNode() = default;

  CastNode(const CastNode&) = default;
  CastNode& operator=(const CastNode&) = default;

 private:
  Type* cast_type_;
  ExprNode* expr_;
};

ExprNode::operator std::string() {
  if (type_ == StmtType::kIdentifier)
    return *dynamic_cast<IdentifierNode*>(this);
  return std::string();
}

class Parser {
 public:
  Parser();
  ~Parser();
  static CompoundNode* Parse(std::vector<Token> token);

  static ExprNode* ParseExpr(Token* token, size_t length, size_t& index);

  static ExprNode* ParsePrimaryExpr(Token* token, size_t length, size_t& index);

 private:
  static bool IsDecl(Token* token, size_t length, size_t index);
  static bool IsFuncDecl(Token* token, size_t length, size_t index);
  static StmtNode* ParseStmt(Token* token, size_t length, size_t& index);
  static VarDeclNode* ParseVarDecl(Token* token, size_t length, size_t& index);
  static FuncDeclNode* ParseFuncDecl(Token* token, size_t length,
                                     size_t& index);
  static ExprNode* ParseBinaryExpr(Token* token, size_t length, size_t& index,
                                   ExprNode* left, unsigned int priority);
  static unsigned int GetPriority(Token token);
};

class ConstType : public Type {
 public:
  ConstType() { type_ = TypeType::kConst; }
  virtual void SetType(Type* type) {
    type_ = TypeType::kConst;
    type_data_ = type;
  }
  virtual ~ConstType() = default;

  Type* GetSubType() { return type_data_; }

  size_t GetSize() override { return type_data_->GetSize(); }

  ConstType(const ConstType&) = default;
  ConstType& operator=(const ConstType&) = default;
};

class PointerType : public Type {
 public:
  PointerType() { type_ = TypeType::kPointer; }
  virtual void SetType(Type* type) {
    type_ = TypeType::kPointer;
    type_data_ = type;
  }
  virtual ~PointerType() = default;

  Type* GetSubType() { return type_data_; }

  size_t GetSize() override { return 8; }

  PointerType(const PointerType&) = default;
  PointerType& operator=(const PointerType&) = default;
};

class ArrayType : public Type {
 public:
  ArrayType() { type_ = TypeType::kArray; }
  virtual void SetType(Type* type, ExprNode* size) {
    type_ = TypeType::kArray;
    type_data_ = type;
    size_ = size;
  }
  virtual ~ArrayType() = default;

  Type* GetSubType() { return type_data_; }

  ExprNode* GetArraySize() { return size_; }

  size_t GetSize() override { return 8; }

  ArrayType(const ArrayType&) = default;
  ArrayType& operator=(const ArrayType&) = default;

 private:
  ExprNode* size_;
};

class ReferenceType : public Type {
 public:
  ReferenceType() { type_ = TypeType::kReference; }
  virtual void SetType(Type* type) {
    type_ = TypeType::kReference;
    type_data_ = type;
  }
  virtual ~ReferenceType() = default;

  ReferenceType(const ReferenceType&) = default;
  ReferenceType& operator=(const ReferenceType&) = default;
};

Type* Type::CreateType(Token* token, size_t length, size_t& index) {
  std::cout << "CreateType" << " \n" << token[index] << std::endl << std::endl;
  Type* type = new Type();
  while (index < length) {
    if (token[index].type == Token::Type::KEYWORD) {
      switch (token[index].value.keyword) {
        case Token::KeywordType::Const: {
          ConstType* const_type = new ConstType();
          if (type->GetType() != Type::TypeType::NONE) {
            const_type->SetType(type);
            type = const_type;
            break;
          }
          index++;
          if (index < length && token[index].type == Token::Type::KEYWORD) {
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
                std::cout << "ReturnType" << " \n"
                          << token[index] << std::endl
                          << std::endl;
                return type;
            }
            const_type->SetType(type);
            type = const_type;
          } else {
            type->SetType(Type::BaseType::NONE);
            std::cout << "ReturnType" << " \n"
                      << token[index] << std::endl
                      << std::endl;
            return type;
          }
          break;
        }
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
        case Token::KeywordType::Struct:
          type->SetType(Type::BaseType::kStruct);
          std::cout << "ReturnType" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return type;
        case Token::KeywordType::Union:
          type->SetType(Type::BaseType::kUnion);
          std::cout << "ReturnType" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return type;
        case Token::KeywordType::Enum:
          type->SetType(Type::BaseType::kEnum);
          std::cout << "ReturnType" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return type;
        case Token::KeywordType::Auto:
          type->SetType(Type::BaseType::kAuto);
          break;
        default:
          std::cout << "ReturnType" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return type;
      }
    } else if (token[index].type == Token::Type::OPERATOR) {
      switch (token[index].value._operator) {
        case Token::OperatorType::star: {
          PointerType* pointer_type = new PointerType();
          pointer_type->SetType(type);
          type = pointer_type;
          break;
        }
        case Token::OperatorType::amp: {
          ReferenceType* reference_type = new ReferenceType();
          reference_type->SetType(type);
          type = reference_type;
          break;
        }
        default:
          std::cout << "ReturnType" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return type;
      }
    } else if (token[index].type == Token::Type::IDENTIFIER) {
      size_t index_temp = index;
      Parser::ParsePrimaryExpr(token, length, index_temp);
      if (token[index_temp].type == Token::Type::OPERATOR &&
          token[index_temp].value._operator == Token::OperatorType::l_square) {
        ArrayType* array_type = new ArrayType();
        array_type->SetType(type, Parser::ParseExpr(token, length, index_temp));
        type = array_type;
      }
      std::cout << "ReturnType" << " \n"
                << token[index] << std::endl
                << std::endl;
      return type;
    }
    index++;
  }
  std::cout << "ReturnType" << " \n" << token[index] << std::endl << std::endl;
  return type;
}

Parser::Parser() = default;
Parser::~Parser() = default;

CompoundNode* Parser::Parse(std::vector<Token> token) {
  Token* token_ptr = token.data();
  size_t index = 0;
  size_t length = token.size();
  CompoundNode* ast = nullptr;
  std::vector<StmtNode*> stmts;
  while (index < token.size()) {
    std::cout << "index: " << index << ", size: " << token.size() << " \n"
              << token[index] << std::endl
              << std::endl;
    if (IsDecl(token_ptr, length, index)) {
      if (IsFuncDecl(token_ptr, length, index)) {
        stmts.push_back(ParseFuncDecl(token_ptr, length, index));
      } else {
        std::cout << "VarDecl" << " \n"
                  << token[index] << std::endl
                  << std::endl;
        stmts.push_back(
            dynamic_cast<DeclNode*>(ParseVarDecl(token_ptr, length, index)));
        if (token_ptr[index].type != Token::Type::OPERATOR ||
            token_ptr[index].value._operator != Token::OperatorType::semi) {
          std::cout << "Error" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return nullptr;
        }
        index++;
      }
    } else {
      // ERROR
      index++;
    }
  }
  ast = new CompoundNode();
  ast->SetCompoundNode(stmts);
  return ast;
}

bool Parser::IsDecl(Token* token, size_t length, size_t index) {
  std::cout << "RUN Parser::IsDecl" << " \n"
            << token[index] << std::endl
            << std::endl;
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
      std::cout << "RETURN TRUE Parser::IsDecl 1" << " \n"
                << token[index] << std::endl
                << std::endl;
      return true;
    } else {
      std::cout << "RETURN FALSE Parser::IsDecl" << " \n"
                << token[index] << std::endl
                << std::endl;
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
    std::cout << "RETURN TRUE Parser::IsDecl 2" << " \n"
              << token[index] << std::endl
              << std::endl;
    return true;
  }
  std::cout << "RETURN FALSE Parser::IsDecl" << " \n"
            << token[index] << std::endl
            << std::endl;
  return false;
}

bool Parser::IsFuncDecl(Token* token, size_t length, size_t index) {
  for (size_t i = index; i < length; i++) {
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

StmtNode* Parser::ParseStmt(Token* token, size_t length, size_t& index) {
  std::cout << "RUN Parser::ParseStmt" << " \n"
            << token[index] << std::endl
            << std::endl;
  if (IsDecl(token, length, index)) {
    if (IsFuncDecl(token, length, index)) {
      return nullptr;
    } else {
      VarDeclNode* result = ParseVarDecl(token, length, index);
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::semi)
        return nullptr;
      index++;
      return dynamic_cast<DeclNode*>(result);
    }
  }
  std::cout << "RUN Parser::ParseStmt ISNT DECL" << " \n"
            << token[index] << std::endl
            << std::endl;
  switch (token[index].type) {
    case Token::Type::OPERATOR:
      switch (token[index].value._operator) {
        case Token::OperatorType::semi:
          index++;
          return nullptr;
        case Token::OperatorType::l_brace: {
          std::cout << "COMPOUNDNODE" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          CompoundNode* result = new CompoundNode();
          std::vector<StmtNode*> stmts;
          index++;
          while (token[index].value._operator != Token::OperatorType::r_brace &&
                 index < length) {
            StmtNode* stmt = ParseStmt(token, length, index);
            if (stmt == nullptr) break;
            stmts.push_back(stmt);
          }
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_brace)
            return nullptr;
          result->SetCompoundNode(stmts);
          index++;
          return result;
        }
        case Token::OperatorType::r_square:
        case Token::OperatorType::r_paren:
        case Token::OperatorType::r_brace:
          return nullptr;
        default:
          StmtNode* stmt_node = ParseExpr(token, length, index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::semi)
            return nullptr;
          index++;
          return stmt_node;
      }
    case Token::Type::KEYWORD:
      switch (token[index].value.keyword) {
        case Token::KeywordType::If: {
          std::cout << "IFNODE" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          IfNode* result = new IfNode();
          index++;
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            return nullptr;
          ExprNode* condition = ParseExpr(token, length, ++index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            return nullptr;
          index++;
          StmtNode* body = ParseStmt(token, length, index);
          result->SetIfNode(condition, body);
          if (token[index].type == Token::Type::KEYWORD &&
              token[index].value.keyword == Token::KeywordType::Else) {
            result->SetIfNode(condition, body, ParseStmt(token, length, index));
          }
          index++;
          std::cout << "IFNODE END" << " \n"
                    << token[index] << std::endl
                    << std::endl;
          return result;
        }
        case Token::KeywordType::While: {
          WhileNode* result = new WhileNode();
          index++;
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::l_paren)
            return nullptr;
          ExprNode* condition = ParseExpr(token, length, ++index);
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::r_paren)
            return nullptr;
          index++;
          result->SetWhileNode(condition, ParseStmt(token, length, index));
          return result;
        }
        default:
          return nullptr;
      }
    default:
      StmtNode* stmt_node = ParseExpr(token, length, index);
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::semi)
        return nullptr;
      index++;
      return stmt_node;
  }
}

FuncDeclNode* Parser::ParseFuncDecl(Token* token, size_t length,
                                    size_t& index) {
  std::cout << "ParseFuncDecl" << " \n"
            << token[index] << std::endl
            << std::endl;
  FuncDeclNode* func_decl = nullptr;
  Type* type = Type::CreateType(token, length, index);
  std::cout << "C POINT" << " \n" << token[index] << std::endl << std::endl;
  // if (token[index].type != Token::Type::IDENTIFIER) return nullptr;
  ExprNode* stat = Parser::ParsePrimaryExpr(token, length, index);
  std::cout << "A POINT" << " \n" << token[index] << std::endl << std::endl;
  if (stat->GetType() != StmtNode::StmtType::kFunc)
    std::cout << "AP ERROR" << " \n" << token[index] << std::endl << std::endl;
  if (stat == nullptr || stat->GetType() != StmtNode::StmtType::kFunc)
    return nullptr;
  std::cout << "B POINT" << " \n" << token[index] << std::endl << std::endl;

  if (token[index].type != Token::Type::OPERATOR ||
      token[index].value._operator != Token::OperatorType::l_brace)
    return func_decl;

  /*index++;

  std::vector<StmtNode*> stmts_vector;
  while (true) {
    if ((token[index].type == Token::Type::OPERATOR &&
         token[index].value._operator == Token::OperatorType::r_brace) ||
        index >= length)
      break;
    std::cout << index << "E POINT" << " \n"
              << token[index] << std::endl
              << std::endl;
    StmtNode* stmt = ParseStmt(token, length, ++index);
    stmts_vector.push_back(stmt);
  }
  index++;
  CompoundNode* stmts = new CompoundNode();
  stmts->SetCompoundNode(stmts_vector);*/

  CompoundNode* stmts =
      dynamic_cast<CompoundNode*>(ParseStmt(token, length, index));
  func_decl = new FuncDeclNode();
  func_decl->SetFuncDeclNode(type, dynamic_cast<FuncNode*>(stat), stmts);

  std::cout << "D POINT" << " \n" << token[index] << std::endl << std::endl;

  return func_decl;
}

VarDeclNode* Parser::ParseVarDecl(Token* token, size_t length, size_t& index) {
  VarDeclNode* var_decl = new VarDeclNode();
  Type* type = Type::CreateType(token, length, index);
  ExprNode* name = ParsePrimaryExpr(token, length, index);
  var_decl->SetVarDeclNode(type, name);
  if (token[index].type != Token::Type::OPERATOR) {
    return var_decl;
    std::cout << "END ParseVarDecl FUNC E5" << " \n"
              << token[index] << std::endl
              << std::endl;
  }
  if (token[index].value._operator == Token::OperatorType::equal)
    std::cout << "IS  =  " << " \n" << token[index] << std::endl << std::endl;
  switch (token[index].value._operator) {
    case Token::OperatorType::l_square: {
      ExprNode* size = ParseExpr(token, length, ++index);
      if (token[index].type != Token::Type::OPERATOR ||
          token[index].value._operator != Token::OperatorType::r_square) {
        return var_decl;
        std::cout << "END ParseVarDecl FUNC E4" << " \n"
                  << token[index] << std::endl
                  << std::endl;
      }
      if (token[index].type == Token::Type::OPERATOR &&
          token[index].value._operator == Token::OperatorType::equal) {
        if (token[index].type != Token::Type::OPERATOR ||
            token[index].value._operator != Token::OperatorType::l_brace) {
          return var_decl;
          std::cout << "END ParseVarDecl FUNC E3" << " \n"
                    << token[index] << std::endl
                    << std::endl;
        }
        std::vector<ExprNode*> values;
        while (true) {
          values.push_back(ParseExpr(token, length, ++index));
          if (token[index].type == Token::Type::OPERATOR &&
              token[index].value._operator == Token::OperatorType::r_brace)
            break;
          if (token[index].type != Token::Type::OPERATOR ||
              token[index].value._operator != Token::OperatorType::comma) {
            std::cout << "END ParseVarDecl FUNC E2" << " \n"
                      << token[index] << std::endl
                      << std::endl;
            return var_decl;
          }
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
      std::cout << "EQUAL RUN IN ParseVarDecl FUNC" << " \n"
                << token[index] << std::endl
                << std::endl;
      ExprNode* value = ParseExpr(token, length, ++index);
      var_decl->SetVarDeclNode(type, name, value);
      break;
    }
    default:
      std::cout << index;
      std::cout << "END ParseVarDecl FUNC E1" << " \n"
                << token[index] << std::endl
                << std::endl;
      return var_decl;
  }
  std::cout << "END ParseVarDecl FUNC" << " \n"
            << token[index] << std::endl
            << std::endl;
  return var_decl;
}

ExprNode* Parser::ParsePrimaryExpr(Token* token, size_t length, size_t& index) {
  enum class State { kPreOper, kPostOper, kEnd };
  State state = State::kPreOper;
  ExprNode* full_expr = nullptr;
  ExprNode* main_expr = nullptr;
  ExprNode* preoper_expr = nullptr;

  std::cout << "START ParsePrimaryExpr FUNC" << " \n"
            << token[index] << std::endl
            << std::endl;

  while (state != State::kEnd && index < length) {
    std::cout << "WHILE ParsePrimaryExpr FUNC" << " \n"
              << token[index] << std::endl
              << std::endl;
    if (token[index].type == Token::Type::OPERATOR) {
      std::cout << "OPER ParsePrimaryExpr FUNC" << " \n"
                << token[index] << std::endl
                << std::endl;
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
          std::cout << "LPAREN ParsePrimaryExpr FUNC" << " \n"
                    << token[index] << std::endl
                    << std::endl;
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
            std::cout << "FN ParsePrimaryExpr FUNC" << " \n"
                      << token[index] << std::endl
                      << std::endl;
            std::vector<ExprNode*> args;
            index++;
            while (index < length && token[index].value._operator !=
                                         Token::OperatorType::r_paren) {
              std::cout << "ARG ParsePrimaryExpr FUNC" << " \n"
                        << token[index] << std::endl
                        << std::endl;
              args.push_back(ParseVarDecl(token, length, index));
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
            std::cout << "NEW FUNC NODE ParsePrimaryExpr FUNC" << " \n"
                      << token[index] << std::endl
                      << std::endl;
            func_node->SetFuncNode(main_expr, args);
            std::cout << "NEW FUNC NODE2 ParsePrimaryExpr FUNC" << " \n"
                      << token[index] << std::endl
                      << std::endl;
            UnaryNode* preoper_unary_node = nullptr;
            /*if (preoper_expr != nullptr)
              preoper_unary_node = dynamic_cast<UnaryNode*>(preoper_expr);
            if (preoper_unary_node != nullptr) {
              preoper_unary_node->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(preoper_expr)->GetOperator(),
                  func_node);
            } else {
              preoper_expr = func_node;
            }
            main_expr = func_node;*/
            if (full_expr == nullptr || preoper_expr == nullptr) {
              full_expr = main_expr = func_node;
            } else {
              if (preoper_expr != nullptr) {
                UnaryNode* unary_node = dynamic_cast<UnaryNode*>(preoper_expr);
                if (unary_node == nullptr) {
                  std::cout << "UNARY ERROR" << " \n"
                            << token[index] << std::endl
                            << std::endl;
                  return nullptr;
                }
                unary_node->SetUnaryNode(unary_node->GetOperator(), func_node);
              }
              main_expr = func_node;
            }
            std::cout << "NEW FUNC NODE END ParsePrimaryExpr FUNC" << " \n"
                      << token[index] << std::endl
                      << std::endl;
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
      std::cout << "IDENT ParsePrimaryExpr FUNC" << " \n"
                << token[index] << std::endl
                << std::endl;
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
        if (preoper_expr != nullptr)
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

  std::cout << "END ParsePrimaryExpr FUNC" << " \n"
            << token[index] << std::endl
            << std::endl;
  return full_expr;
}

ExprNode* Parser::ParseExpr(Token* token, size_t length, size_t& index) {
  std::cout << "START ParseExpr" << " \n"
            << token[index] << std::endl
            << std::endl;
  if (index >= length) return nullptr;
  ExprNode* expr = ParsePrimaryExpr(token, length, index);
  expr = ParseBinaryExpr(token, length, index, expr, 0);
  std::cout << "END ParseExpr" << " \n"
            << token[index] << std::endl
            << std::endl;
  return expr;
}

ExprNode* Parser::ParseBinaryExpr(Token* token, size_t length, size_t& index,
                                  ExprNode* left, unsigned int priority) {
  ExprNode* expr = left;
  while (index < length && GetPriority(token[index]) > priority) {
    if (token[index].type != Token::Type::OPERATOR) return expr;
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

  // TODO(Parser::ParseBinaryExpr): Complete the function.
  return expr;
}

unsigned int Parser::GetPriority(Token token) {
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
  return 0;
}

class BytecodeGenerator {
 public:
  BytecodeGenerator() = default;
  ~BytecodeGenerator() = default;

  void GenerateBytecode(CompoundNode* stmt);

 private:
  class Memory {
   public:
    Memory() { IsBigEndian(); }
    ~Memory() = default;

    size_t Add(uint8_t type, size_t size) {
      size_t index = memory_.size();
      type_.push_back(type);
      memory_.resize(all_size_ + size);
      all_size_ += size;

      if (type > 0x0F) return index;

      for (size_t i = 0; i < size; i++) {
        if (memory_.size() % 2 != 0) {
          memory_.push_back(0);
          all_size_++;
          type_[type_.size() - 1] = (type_[type_.size() - 1] << 4) | type;
        } else {
          memory_.push_back(0);
          all_size_ += 2;
          type_.push_back(type);
        }
      }

      return index;
    }

    size_t Add(uint8_t type, size_t size, void* data) {
      size_t index = memory_.size();
      type_.push_back(type);
      memory_.resize(all_size_ + size);
      all_size_ += size;

      if (type > 0x0F) return index;

      void* memory_data = malloc(size);

      size_t read_index = 0;
      for (void* data_ptr = memory_data; read_index < size;) {
        switch (type) {
          case 0x01:
            *(int8_t*)memory_data = *(int8_t*)data;
            data = (void*)((uintptr_t)data + 1);
            read_index += 1;
            memory_data = (void*)((uintptr_t)memory_data + 1);
            break;
          case 0x02:
            *(int*)(memory_data) =
                is_big_endian ? *(int*)data : SwapInt(*(int*)data);
            data = (void*)((uintptr_t)data + 4);
            read_index += 4;
            memory_data = (void*)((uintptr_t)memory_data + 4);
            break;
          case 0x03:
            *(long*)(memory_data) =
                is_big_endian ? *(long*)data : SwapLong(*(long*)data);
            data = (void*)((uintptr_t)data + 8);
            read_index += 8;
            memory_data = (void*)((uintptr_t)memory_data + 8);
            break;
          case 0x04:
            *(float*)(memory_data) =
                is_big_endian ? *(float*)data : SwapFloat(*(float*)data);
            data = (void*)((uintptr_t)data + 4);
            read_index += 4;
            memory_data = (void*)((uintptr_t)memory_data + 4);
            break;
          case 0x05:
            *(double*)(memory_data) =
                is_big_endian ? *(double*)data : SwapDouble(*(double*)data);
            data = (void*)((uintptr_t)data + 8);
            read_index += 8;
            memory_data = (void*)((uintptr_t)memory_data + 8);
            break;
          case 0x06:
            *(uint64_t*)(memory_data) = is_big_endian
                                            ? *(uint64_t*)data
                                            : SwapUint64t(*(uint64_t*)data);
            data = (void*)((uintptr_t)data + 8);
            read_index += 8;
            memory_data = (void*)((uintptr_t)memory_data + 8);
          default:
            return index;
        }
      }

      for (size_t i = 0; i < size; i++) {
        if (memory_.size() % 2 != 0) {
          memory_.push_back(*(uint64_t*)memory_data);
          memory_data = (void*)((uintptr_t)memory_data + 1);
          all_size_++;
          type_[type_.size() - 1] = (type_[type_.size() - 1] << 4) | type;
        } else {
          memory_.push_back(*(uint64_t*)memory_data);
          memory_data = (void*)((uintptr_t)memory_data + 1);
          all_size_ += 2;
          type_.push_back(type);
        }
      }
      return index;
    }

   private:
    size_t all_size_ = 0;
    std::vector<uint8_t> memory_;
    std::vector<uint8_t> type_;
    bool is_big_endian = false;

    void IsBigEndian() {
      uint16_t test_data = 0x0011;
      is_big_endian = (*(uint8_t*)&test_data == 0x00);
    }

    int SwapInt(int x) {
      uint32_t ux = (uint32_t)x;
      ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
           ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
      return (int)ux;
    }

    long SwapLong(long x) {
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
      uint32_t ux;
      memcpy(&ux, &x, sizeof(uint32_t));
      ux = ((ux << 24) & 0xFF000000) | ((ux << 8) & 0x00FF0000) |
           ((ux >> 8) & 0x0000FF00) | ((ux >> 24) & 0x000000FF);
      float result;
      memcpy(&result, &ux, sizeof(float));
      return result;
    }

    double SwapDouble(double x) {
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

  void HandleFuncDecl(FuncDeclNode* func_decl, size_t& size);
  void HandleVarDecl(VarDeclNode* var_decl, size_t& size);
  void HandleArrayDecl(ArrayDeclNode* array_decl, size_t& size);
  void HandleStmt(StmtNode* stmt, size_t& size);
  size_t HandleExpr(ExprNode* expr, size_t& size);
  size_t HandleUnaryExpr(UnaryNode* expr, size_t& size);
  size_t HandleBinaryExpr(BinaryNode* expr, size_t& size);
  size_t HandleFuncInvoke(FuncNode* func, size_t& size);

  LexMap<FuncDeclNode*> func_table_;
  LexMap<size_t> var_table_;
  LexMap<ArrayDeclNode*> array_table_;
  Memory memory_;
  std::vector<uint8_t> code_;
  size_t file_size_ = 0;
};

void BytecodeGenerator::GenerateBytecode(CompoundNode* stmt) {
  if (stmt == nullptr) return;
  std::cout << "BytecodeGenerator::GenerateBytecode OK" << std::endl;
  for (size_t i = 0; i < stmt->GetStmts().size(); i++) {
    switch (stmt->GetStmts()[i]->GetType()) {
      case StmtNode::StmtType::kFuncDecl:
        HandleFuncDecl(dynamic_cast<FuncDeclNode*>(stmt->GetStmts()[i]),
                       file_size_);
        break;
      case StmtNode::StmtType::kVarDecl:
        HandleVarDecl(dynamic_cast<VarDeclNode*>(stmt->GetStmts()[i]),
                      file_size_);
        break;
      case StmtNode::StmtType::kArrayDecl:
        HandleVarDecl(dynamic_cast<ArrayDeclNode*>(stmt->GetStmts()[i]),
                      file_size_);
        break;
      default:
        break;
    }
  }
}

void BytecodeGenerator::HandleFuncDecl(FuncDeclNode* func_decl, size_t& size) {
  std::cout << "BytecodeGenerator::HandleFuncDecl OK" << std::endl;
  func_table_.Insert(*func_decl->GetStat()->GetName(), func_decl);
  size_t func_decl_size = 8;
}

void BytecodeGenerator::HandleVarDecl(VarDeclNode* var_decl, size_t& size) {
  std::cout << "BytecodeGenerator::HandleVarDecl OK" << std::endl;
  Type* var_type = var_decl->GetType();
  while (var_type->GetType() == Type::TypeType::kBase ||
         var_type->GetType() == Type::TypeType::kPointer ||
         var_type->GetType() == Type::TypeType::kArray ||
         var_type->GetType() == Type::TypeType::kReference) {
    switch (var_type->GetType()) {
      case Type::TypeType::kConst:
        var_type = dynamic_cast<ConstType*>(var_type)->GetSubType();
        break;
      default:
        return;
    }
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
        vm_type = 0x00;
        break;
    }
  }
  var_table_.Insert(*var_decl->GetName(),
                    memory_.Add(vm_type, var_type->GetSize()));
}

void BytecodeGenerator::HandleArrayDecl(ArrayDeclNode* array_decl,
                                        size_t& size) {
  std::cout << "BytecodeGenerator::HandleArrayDecl OK" << std::endl;
  array_table_.Insert(*array_decl->GetName(), array_decl);
}

size_t BytecodeGenerator::HandleExpr(ExprNode* expr, size_t& size) {
  if (expr->GetType() == StmtNode::StmtType::kUnary) {
    return HandleUnaryExpr(dynamic_cast<UnaryNode*>(expr), size);
  } else if (expr->GetType() == StmtNode::StmtType::kBinary) {
    return HandleBinaryExpr(dynamic_cast<BinaryNode*>(expr), size);
  } else if (expr->GetType() == StmtNode::StmtType::kFunc) {
  }
}
size_t BytecodeGenerator::HandleUnaryExpr(UnaryNode* expr, size_t& size) {
  // TODO(BytecodeGenerator::HandleUnaryExpr): Complete the function.
  switch (expr->GetOperator()) {
    case UnaryNode::Operator::kPostInc:
      break;
    case UnaryNode::Operator::kPostDec:
      break;
    case UnaryNode::Operator::kPreInc:
      break;
    case UnaryNode::Operator::kPreDec:
      break;
    case UnaryNode::Operator::kAddrOf:
      break;
    case UnaryNode::Operator::kDeref:
      break;
    case UnaryNode::Operator::kPlus:
      break;
    case UnaryNode::Operator::kMinus:
      break;
    case UnaryNode::Operator::kNot:
      break;
    case UnaryNode::Operator::kBitwiseNot:
      break;
    default:
      break;
  }
}
size_t BytecodeGenerator::HandleBinaryExpr(BinaryNode* expr, size_t& size) {}

void BytecodeGenerator::HandleStmt(StmtNode* stmt, size_t& size) {}

size_t BytecodeGenerator::HandleFuncInvoke(FuncNode* func, size_t& size) {}

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
  while (true) {
    Aq::Compiler::Token token_buffer;
    lexer.LexToken(token_buffer);
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