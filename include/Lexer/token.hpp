#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum tokenType{
  // Keywords
  AS, BREAK, CONST, CONTINUE, CRATE, ELSE, ENUM, FALSE, FN, FOR, IF, 
  IMPL, IN, LET, LOOP, MATCH, MOD, MOVE, MUT, REF, RETURN, SELF, SELF_, 
  STATIC, STRUCT, SUPER, TRAIT, TRUE, TYPE, UNSAFE, USE, WHERE, WHILE, DYN, 
  ABSTRACT, BECOME, BOX, DO, FINAL, MACRO, OVERRIDE, PRIV, TYPEOF, UNSIZED, 
  VIRTUAL, YIELD, TRY, GEN,
  // Identifiers
  IDENTIFIER,
  // Literals
  CHAR_LITERAL, STRING_LITERAL, RAW_STRING_LITERAL, 
  CSTRING_LITERAL, RAW_CSTRING_LITERAL, 
  INTEGER_LITERAL,
  // Punctuation
  PLUS,        // +
  MINUS,       // -
  STAR,        // *
  SLASH,       // /
  PERCENT,     // %
  CARET,       // ^
  NOT,         // !
  AND,         // &
  OR,          // |
  AND_AND,     // &&
  OR_OR,       // ||
  SHL,         // <<
  SHR,         // >>
  PLUS_EQ,     // +=
  MINUS_EQ,    // -=
  STAR_EQ,     // *=
  SLASH_EQ,    // /=
  PERCENT_EQ,  // %=
  CARET_EQ,    // ^=
  AND_EQ,      // &=
  OR_EQ,       // |=
  SHL_EQ,      // <<=
  SHR_EQ,      // >>=
  EQ,          // =
  EQ_EQ,       // ==
  NE,          // !=
  GT,          // >
  LT,          // <
  GE,          // >=
  LE,          // <=
  AT,          // @
  UNDERSCORE,  // _
  DOT,         // .
  DOT_DOT,     // ..
  DOT_DOT_DOT, // ...
  DOT_DOT_EQ,  // ..=
  COMMA,       // ,
  SEMI,        // ;
  COLON,       // :
  PATH_SEP,    // ::
  R_ARROW,     // ->
  FAT_ARROW,   // =>
  L_ARROW,     // <-
  POUND,       // #
  DOLLAR,      // $
  QUESTION,    // ?
  TILDE,       // ~
  // Delimiters
  L_BRACE,   // {
  R_BRACE,   // }
  L_BRACKET, // [
  R_BRACKET, // ]
  L_PAREN,   // (
  R_PAREN,   // )
  // Reserved tokens
  RESERVED_TOKEN,
  // Others
  WHITESPACE, E_O_F
};

struct Token
{
  tokenType type;
  std::string str;
};


#endif