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
    l_square,
    r_square,
    l_paren,
    r_paren,
    l_brace,
    r_brace,
    period,
    ellipsis,
    amp,
    ampamp,
    ampequal,
    star,
    starequal,
    plus,
    plusplus,
    plusequal,
    minus,
    arrow,
    minusminus,
    minusequal,
    tilde,
    exclaim,
    exclaimequal,
    slash,
    slashequal,
    percent,
    percentequal,
    less,
    lessless,
    lessequal,
    lesslessequal,
    spaceship,
    greater,
    greatergreater,
    greaterequal,
    greatergreaterequal,
    caret,
    caretequal,
    pipe,
    pipepipe,
    pipeequal,
    question,
    colon,
    semi,
    equal,
    equalequal,
    comma,
    hash,
    hashhash,
    hashat,
    periodstar,
    arrowstar,
    coloncolon,
    at,
    lesslessless,
    greatergreatergreater,
    caretcaret,
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

    // DEBUG/TEST CODE
    std::cout << std::string(location, length) << std::endl;

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
  enum class TypeType { kBase, kPointer, kArray };

  enum class BaseType {
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

  Type() { type_ = TypeType::kBase; }
  virtual void SetType(BaseType type) { base_data_ = type; }
  virtual ~Type() = default;

  Type(const Type&) = default;
  Type& operator=(const Type&) = default;

  TypeType GetType() { return type_; }

 protected:
  TypeType type_;
  Type* type_data_;

 private:
  BaseType base_data_;
};

class PointerType : public Type {
 public:
  PointerType() { type_ = TypeType::kPointer; }
  virtual void SetType(Type* type) {
    type_ = TypeType::kPointer;
    type_data_ = type;
  }
  virtual ~PointerType() = default;

  PointerType(const PointerType&) = default;
  PointerType& operator=(const PointerType&) = default;
};

class ArrayType : public Type {
 public:
  ArrayType() { type_ = TypeType::kArray; }
  virtual void SetType(Type* type, int size) {
    type_ = TypeType::kArray;
    type_data_ = type;
    size_ = size;
  }
  virtual ~ArrayType() = default;

  int GetSize() { return size_; }

  ArrayType(const ArrayType&) = default;
  ArrayType& operator=(const ArrayType&) = default;

 private:
  int size_;
};

class StmtNode {
 public:
  StmtNode() { type_ = StmtType::kStmt; }
  virtual ~StmtNode() = default;

  enum class StmtType {
    kStmt,
    kDecl,
    kExpr,
    kFuncDecl,
    kVarDecl,
    kFuncInvoke,
    kAssign,
    kIf,
    kNumber,
    kVar,
    kUnary,
    kBinary,
    kConditional,
    kFunc,
    kCast,
    kArrayDecl,
    kArray
  };

  StmtType GetType() { return type_; }

  StmtNode(const StmtNode&) = default;
  StmtNode& operator=(const StmtNode&) = default;

 protected:
  StmtType type_;
  std::vector<StmtNode*> stmts_;
};

class ExprNode : public StmtNode {
 public:
  ExprNode() { type_ = StmtType::kExpr; }
  virtual ~ExprNode() = default;

  ExprNode(const ExprNode&) = default;
  ExprNode& operator=(const ExprNode&) = default;
};

class NumberNode : public ExprNode {
 public:
  NumberNode() { type_ = StmtType::kNumber; }
  void SetNumberNode(Token value) { value_ = value; }
  virtual ~NumberNode() = default;

  NumberNode(const NumberNode&) = default;
  NumberNode& operator=(const NumberNode&) = default;

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
    kLNot,
    ARRAY
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

class BinaryNode : public ExprNode {
 public:
  enum class Operator {
    kAdd,
    kSub,
    kMul,
    kDiv,
    kRem,
    kAnd,
    kOr,
    kXor,
    kShl,
    kShr,
    kLT,
    kGT,
    kLE,
    kGE,
    kEQ,
    kNE,
    kLAnd,
    kLOr,
    kAssign,
    kAddAssign,
    kSubAssign,
    kMulAssign,
    kDivAssign,
    kRemAssign,
    kAndAssign,
    kOrAssign,
    kXorAssign,
    kShlAssign,
    kShrAssign,
    kComma
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
  void SetFuncNode(Token name, std::vector<ExprNode*> params) {
    name_ = name;
    params_ = params;
  }
  virtual ~FuncNode() = default;

  FuncNode(const FuncNode&) = default;
  FuncNode& operator=(const FuncNode&) = default;

 private:
  Token name_;
  std::vector<ExprNode*> params_;
};

class VarNode : public ExprNode {
 public:
  VarNode() { type_ = StmtType::kVar; }
  void SetVarNode(Token name) { name_ = name; }
  virtual ~VarNode() = default;

  VarNode(const VarNode&) = default;
  VarNode& operator=(const VarNode&) = default;

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
  void SetVarDeclNode(Type* type, Token name) {
    var_type_ = type;
    name_ = name;
    value_.push_back(NULL);
  }
  void SetVarDeclNode(Type* type, Token name, std::vector<ExprNode*> value) {
    var_type_ = type;
    name_ = name;
    value_ = value;
  }

  virtual ~VarDeclNode() = default;

  VarDeclNode(const VarDeclNode&) = default;
  VarDeclNode& operator=(const VarDeclNode&) = default;

 protected:
  Type* var_type_;
  Token name_;
  std::vector<ExprNode*> value_;
};

class ArrayDeclNode : public VarDeclNode {
 public:
  ArrayDeclNode() { DeclNode::type_ = StmtType::kArrayDecl; }
  void SetArrayDeclNode(Type* type, Token name, ExprNode* size) {
    var_type_ = type;
    name_ = name;
    size_ = size;
    value_.push_back(new ExprNode());
  }
  void SetArrayDeclNode(Type* type, Token name, ExprNode* size,
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
  void SetFuncDeclNode(Type* type, Token name, std::vector<ExprNode*> args,
                       std::vector<ExprNode*> stmts) {
    return_type_ = type;
    name_ = name;
    args_ = args;
    stmts_ = stmts;
  }
  virtual ~FuncDeclNode() = default;

  FuncDeclNode(const FuncDeclNode&) = default;
  FuncDeclNode& operator=(const FuncDeclNode&) = default;

 private:
  Type* return_type_;
  Token name_;
  std::vector<ExprNode*> args_;
  std::vector<ExprNode*> stmts_;
};

class FuncInvokeNode : public ExprNode {
 public:
  FuncInvokeNode() { type_ = StmtType::kFuncInvoke; }
  void SetFuncInvokeNode(Token name, std::vector<ExprNode*> args) {
    name_ = name;
    args_ = args;
  }
  virtual ~FuncInvokeNode() = default;

  FuncInvokeNode(const FuncInvokeNode&) = default;
  FuncInvokeNode& operator=(const FuncInvokeNode&) = default;

 private:
  Token name_;
  std::vector<ExprNode*> args_;
};

class IfNode : public StmtNode {
 public:
  IfNode() { type_ = StmtType::kIf; }
  void SetIfNode(ExprNode* condition, std::vector<StmtNode*> body) {
    condition_ = condition;
    body_ = body;
  }

  IfNode(const IfNode&) = default;
  IfNode& operator=(const IfNode&) = default;

 private:
  ExprNode* condition_;
  std::vector<StmtNode*> body_;
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

class Parser {
 public:
  Parser();
  ~Parser();
  std::vector<StmtNode> Parse(std::vector<Token> token);

 private:
  bool IsDecl(Token* token, size_t length);
  bool IsFuncDecl(Token* token, size_t length);
  size_t ParseFuncDecl(Token* token, size_t length, FuncDeclNode& result);
  ExprNode* ParseExpr(Token* token, size_t length, size_t& index);
  ExprNode* ParsePrimaryExpr(Token* token, size_t length, size_t& index);
  ExprNode* ParseFullExpr(Token* token, size_t length, size_t& index);
};

Parser::Parser() = default;
Parser::~Parser() = default;

// TODO(Parser): NOT COMPLETE.
std::vector<StmtNode> Parser::Parse(std::vector<Token> token) {
  for (size_t i = 0; i < token.size();) {
    if (IsDecl(token.data() + i, token.size() - i)) {
      if (IsFuncDecl(token.data() + i, token.size() - i)) {
        FuncDeclNode result;
        ParseFuncDecl(token.data() + i, token.size() - i, result);
      } else {
      }
    } else {
    }
  }
}

bool Parser::IsDecl(Token* token, size_t length) {
  if (token[0].type == Token::Type::KEYWORD) {
    if (token[0].value.keyword == Token::KeywordType::Auto ||
        token[0].value.keyword == Token::KeywordType::Bool ||
        token[0].value.keyword == Token::KeywordType::Char ||
        token[0].value.keyword == Token::KeywordType::Double ||
        token[0].value.keyword == Token::KeywordType::Float ||
        token[0].value.keyword == Token::KeywordType::Int ||
        token[0].value.keyword == Token::KeywordType::Long ||
        token[0].value.keyword == Token::KeywordType::Void ||
        token[0].value.keyword == Token::KeywordType::String ||
        token[0].value.keyword == Token::KeywordType::Struct ||
        token[0].value.keyword == Token::KeywordType::Union ||
        token[0].value.keyword == Token::KeywordType::Enum ||
        token[0].value.keyword == Token::KeywordType::Namespace ||
        token[0].value.keyword == Token::KeywordType::Template ||
        token[0].value.keyword == Token::KeywordType::Typedef ||
        token[0].value.keyword == Token::KeywordType::Extern ||
        token[0].value.keyword == Token::KeywordType::Class ||
        token[0].value.keyword == Token::KeywordType::Const ||
        token[0].value.keyword == Token::KeywordType::Friend ||
        token[0].value.keyword == Token::KeywordType::Inline ||
        token[0].value.keyword == Token::KeywordType::Number ||
        token[0].value.keyword == Token::KeywordType::Short ||
        token[0].value.keyword == Token::KeywordType::Signed ||
        token[0].value.keyword == Token::KeywordType::Unsigned ||
        token[0].value.keyword == Token::KeywordType::Virtual ||
        token[0].value.keyword == Token::KeywordType::Wchar_t) {
      return true;
    } else {
      return false;
    }
  } else if ((token[0].type == Token::Type::IDENTIFIER &&
              token[1].type == Token::Type::IDENTIFIER) ||
             (token[0].type == Token::Type::IDENTIFIER &&
              token[1].type == Token::Type::OPERATOR &&
              (token[1].value._operator == Token::OperatorType::star ||
               token[1].value._operator == Token::OperatorType::amp ||
               token[1].value._operator == Token::OperatorType::ampamp) &&
              token[2].type == Token::Type::IDENTIFIER)) {
    return true;
  }
  return false;
}

bool Parser::IsFuncDecl(Token* token, size_t length) {
  for (size_t i = 0; i < length; i++) {
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

size_t Parser::ParseFuncDecl(Token* token, size_t length,
                             FuncDeclNode& result) {
  std::vector<Token> type;
  size_t index = 0;
  while ((token[index].type == Token::Type::KEYWORD &&
          (token[index].value.keyword == Token::KeywordType::Auto ||
           token[index].value.keyword == Token::KeywordType::Bool ||
           token[index].value.keyword == Token::KeywordType::Char ||
           token[index].value.keyword == Token::KeywordType::Double ||
           token[index].value.keyword == Token::KeywordType::Float ||
           token[index].value.keyword == Token::KeywordType::Int ||
           token[index].value.keyword == Token::KeywordType::Long ||
           token[index].value.keyword == Token::KeywordType::Void ||
           token[index].value.keyword == Token::KeywordType::String ||
           token[index].value.keyword == Token::KeywordType::Const ||
           token[index].value.keyword == Token::KeywordType::Friend ||
           token[index].value.keyword == Token::KeywordType::Inline ||
           token[index].value.keyword == Token::KeywordType::Number ||
           token[index].value.keyword == Token::KeywordType::Short ||
           token[index].value.keyword == Token::KeywordType::Signed ||
           token[index].value.keyword == Token::KeywordType::Unsigned ||
           token[index].value.keyword == Token::KeywordType::Virtual ||
           token[index].value.keyword == Token::KeywordType::Wchar_t)) ||
         ((token[index].type == Token::Type::IDENTIFIER &&
           token[index + 1].type == Token::Type::IDENTIFIER)) ||
         (token[index].type == Token::Type::IDENTIFIER &&
          token[index + 1].type == Token::Type::OPERATOR &&
          (token[index + 1].value._operator == Token::OperatorType::star ||
           token[index + 1].value._operator == Token::OperatorType::amp ||
           token[index + 1].value._operator == Token::OperatorType::ampamp) &&
          token[index + 2].type == Token::Type::IDENTIFIER) ||
         (token[index].type == Token::Type::OPERATOR &&
          (token[index].value._operator == Token::OperatorType::star ||
           token[index].value._operator == Token::OperatorType::amp ||
           token[index].value._operator == Token::OperatorType::ampamp) &&
          token[index + 1].type == Token::Type::IDENTIFIER)) {
    type.push_back(token[index]);
    index++;
  }

  /*result.SetFuncDeclNode(type, token[index],
     std::vector<std::unique_ptr<ExprNode>>(),
                         std::vector<std::unique_ptr<StmtNode>>());*/
}

ExprNode* Parser::ParseExpr(Token* token, size_t length, size_t& index) {
  while (index <= length) {
    if (token[index].type == Token::Type::OPERATOR) {
      switch (token[index].value._operator) {
        case Token::OperatorType::NONE:
          break;
        case Token::OperatorType::l_square:  // [
          break;
        case Token::OperatorType::r_square:  // ]
          break;
        case Token::OperatorType::l_paren:  // (
          break;
        case Token::OperatorType::r_paren:  // )
          break;
        case Token::OperatorType::l_brace:  // {
          break;
        case Token::OperatorType::r_brace:  // }
          break;
        case Token::OperatorType::period:  // .
          break;
        case Token::OperatorType::ellipsis:  // ...
          break;
        case Token::OperatorType::amp:  // &
          break;
        case Token::OperatorType::ampamp:  // &&
          break;
        case Token::OperatorType::ampequal:  // &=
          break;
        case Token::OperatorType::star:  // *
          break;
        case Token::OperatorType::starequal:  // *=
          break;
        case Token::OperatorType::plus:  // +
          break;
        case Token::OperatorType::plusplus:  // ++
          break;
        case Token::OperatorType::plusequal:  // +=
          break;
        case Token::OperatorType::minus:  // -
          break;
        case Token::OperatorType::arrow:  // ->
          break;
        case Token::OperatorType::minusminus:  // --
          break;
        case Token::OperatorType::minusequal:  // -=
          break;
        case Token::OperatorType::tilde:  // ~
          break;
        case Token::OperatorType::exclaim:  // !
          break;
        case Token::OperatorType::exclaimequal:  // !=
          break;
        case Token::OperatorType::slash:  // /
          break;
        case Token::OperatorType::slashequal:  // /=
          break;
        case Token::OperatorType::percent:  // %
          break;
        case Token::OperatorType::percentequal:  // %=
          break;
        case Token::OperatorType::less:  // <
          break;
        case Token::OperatorType::lessless:  // <<
          break;
        case Token::OperatorType::lessequal:  // <=
          break;
        case Token::OperatorType::lesslessequal:  // <<=
          break;
        case Token::OperatorType::spaceship:  // <=>
          break;
        case Token::OperatorType::greater:  // >
          break;
        case Token::OperatorType::greatergreater:  // >>
          break;
        case Token::OperatorType::greaterequal:  // >=
          break;
        case Token::OperatorType::greatergreaterequal:  // >>=
          break;
        case Token::OperatorType::caret:  // ^
          break;
        case Token::OperatorType::caretequal:  // ^=
          break;
        case Token::OperatorType::pipe:  // |
          break;
        case Token::OperatorType::pipepipe:  // ||
          break;
        case Token::OperatorType::pipeequal:  // |=
          break;
        case Token::OperatorType::question:  // ?
          break;
        case Token::OperatorType::colon:  // :
          break;
        case Token::OperatorType::semi:  // ;
          break;
        case Token::OperatorType::equal:  // =
          break;
        case Token::OperatorType::equalequal:  // ==
          break;
        case Token::OperatorType::comma:  // ,
          break;
        case Token::OperatorType::hash:  // #
          break;
        case Token::OperatorType::hashhash:  // ##
          break;
        case Token::OperatorType::hashat:  // #@
          break;
        case Token::OperatorType::periodstar:  // .*
          break;
        case Token::OperatorType::arrowstar:  // ->*
          break;
        case Token::OperatorType::coloncolon:  // ::
          break;
        case Token::OperatorType::at:  // @
          break;
        case Token::OperatorType::lesslessless:  // <<<
          break;
        case Token::OperatorType::greatergreatergreater:  // >>>
          break;
        case Token::OperatorType::caretcaret:  // ^^
          break;
        default:
          break;
      }
    } else if (token[index].type == Token::Type::IDENTIFIER) {
    }
  }
}

ExprNode* Parser::ParsePrimaryExpr(Token* token, size_t length, size_t& index) {
  size_t index = 0;
  enum class State { kPreOper, kPostOper, kEnd };
  State state = State::kPreOper;
  ExprNode* expr = NULL;
  ExprNode* primary = NULL;
  while (state != State::kEnd && index <= length) {
    if (token[index].type == Token::Type::OPERATOR) {
      switch (token[index].value._operator) {
        case Token::OperatorType::amp:  // &
          if (state == State::kPreOper) {
            UnaryNode* amp_node = new UnaryNode();
            amp_node->SetUnaryNode(UnaryNode::Operator::kAddrOf, NULL);
            if (expr == NULL || primary == NULL) {
              expr = amp_node;
              primary = amp_node;
            } else {
              dynamic_cast<UnaryNode*>(primary)->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(primary)->GetOperator(), amp_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::star:  // *
          if (state == State::kPreOper) {
            UnaryNode* star_node = new UnaryNode();
            star_node->SetUnaryNode(UnaryNode::Operator::kDeref, NULL);
            if (expr == NULL || primary == NULL) {
              expr = star_node;
              primary = star_node;
            } else {
              dynamic_cast<UnaryNode*>(primary)->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(primary)->GetOperator(), star_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::plus:  // +
          if (state == State::kPreOper) {
            UnaryNode* plus_node = new UnaryNode();
            plus_node->SetUnaryNode(UnaryNode::Operator::kPlus, NULL);
            if (expr == NULL || primary == NULL) {
              expr = plus_node;
              primary = plus_node;
            } else {
              dynamic_cast<UnaryNode*>(primary)->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(primary)->GetOperator(), plus_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;
        case Token::OperatorType::minus:  // -
          if (state == State::kPreOper) {
            UnaryNode* minus_node = new UnaryNode();
            minus_node->SetUnaryNode(UnaryNode::Operator::kMinus, NULL);
            if (expr == NULL || primary == NULL) {
              expr = minus_node;
              primary = minus_node;
            } else {
              dynamic_cast<UnaryNode*>(primary)->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(primary)->GetOperator(), minus_node);
            }
            index++;
            break;
          }
          state = State::kEnd;
          break;

        case Token::OperatorType::l_square:  // [
          if (state == State::kPostOper) {
            ArrayNode* array = new ArrayNode();
            array->SetArrayNode(expr, ParseExpr(token, length, index));
            expr = array;
            index++;
            break;
          }
          break;
        case Token::OperatorType::r_square:  // ]
          if (dynamic_cast<UnaryNode*>(expr)->GetOperator() !=
              UnaryNode::Operator::ARRAY) {
            state = State::kEnd;
            break;
          }
          index++;
          break;
        case Token::OperatorType::l_paren:  // (
          if (state == State::kPreOper) {
            if (expr == NULL || primary == NULL) {
              expr = ParseExpr(token, length, index);
              primary = expr;
            } else {
              dynamic_cast<UnaryNode*>(primary)->SetUnaryNode(
                  dynamic_cast<UnaryNode*>(primary)->GetOperator(),
                  ParseExpr(token, length, index));
            }
          } else if (state == State::kPostOper) {
          }
          break;
        case Token::OperatorType::r_paren:  // )
          break;
        case Token::OperatorType::l_brace:  // {
          break;
        case Token::OperatorType::r_brace:  // }
          break;
        case Token::OperatorType::period:  // .
          break;
        case Token::OperatorType::arrow:  // ->
          break;
        case Token::OperatorType::minusminus:  // --
          break;
        case Token::OperatorType::tilde:  // ~
          break;
        case Token::OperatorType::exclaim:  // !
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
          break;
        default:
          break;
      }
    } else if (token[index].type == Token::Type::IDENTIFIER) {
    }
  }
}

ExprNode* Parser::ParseFullExpr(Token* token, size_t length, size_t& index) {}

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
  Aq::Compiler::Parser parser;
  parser.Parse(token);
}