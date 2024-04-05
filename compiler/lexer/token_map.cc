// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license in
// the root directory.

#include "compiler/lexer/token_map.h"

#include "compiler/compiler.h"
#include "compiler/token/keyword.h"
#include "compiler/token/operator.h"
#include "compiler/token/token.h"
#include "compiler/token/token_kind.h"
#include "debugger/debugger.h"

namespace Aq {
Compiler::TokenMap::TokenMap() {
  /// TODO: Should be improved
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
Compiler::TokenMap::~TokenMap() = default;

Compiler::Token::Kind Compiler::TokenMap::GetKind(std::string key) {
  /// TODO: Should be improved
  /// return token_map_.Find(_operator);
}
}  // namespace Aq