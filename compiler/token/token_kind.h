// Copyright 2024 AQ authors, All Rights Reserved.
// This program is licensed under the AQ License. You can find the AQ license
/// in the root directory.

#ifndef AQ_COMPILER_TOKEN_TOKEN_KIND_H_
#define AQ_COMPILER_TOKEN_TOKEN_KIND_H_

#include "compiler/compiler.h"
#include "compiler/token/token.h"

namespace Aq {
enum class Compiler::Token::Kind {
  NONE,
  START,
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
  kLsquare,
  kRsquare,
  kLparen,
  kRparen,
  kLbrace,
  kRbrace,
  kPeriod,
  kEllipsis,
  kAmp,
  kAmpamp,
  kAmpequal,
  kStar,
  kStarequal,
  kPlus,
  kPlusplus,
  kPlusequal,
  kMinus,
  kArrow,
  kMinusminus,
  kMinusequal,
  kTilde,
  kExclaim,
  kExclaimequal,
  kSlash,
  kSlashequal,
  kPercent,
  kPercentequal,
  kLess,
  kLessless,
  kLessequal,
  kLesslessequal,
  kSpaceship,
  kGreater,
  kGreatergreater,
  kGreaterequal,
  kGreatergreaterequal,
  kCaret,
  kCaretequal,
  kPipe,
  kPipepipe,
  kPipeequal,
  kQuestion,
  kColon,
  kSemi,
  kEqual,
  kEqualequal,
  kComma,
  kHash,
  kHashhash,
  kHashat,
  kPeriodstar,
  kArrowstar,
  kColoncolon,
  kAt,
  kLesslessless,
  kGreatergreatergreater,
  kCaretcaret,
  NUMBER,
  CHARACTER,
  STRING,
  COMMENT
};
}  // namespace Aq
#endif