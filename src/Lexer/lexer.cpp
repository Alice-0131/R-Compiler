#include <string>
#include <regex>
#include <vector>
#include <stack>
#include "lexer.hpp"
#include "token.hpp"

struct lexerRule
{
  tokenType type;
  std::regex pattern;
};

// mostly by AI : start
std::vector<lexerRule> rules = {
  // others
  {WHITESPACE, std::regex(R"(^\s+)")},
  // Keywords
  {AS, std::regex(R"(^as\b)")},
  {BREAK, std::regex(R"(^break\b)")},
  {CONST, std::regex(R"(^const\b)")},
  {CONTINUE, std::regex(R"(^continue\b)")},
  {CRATE, std::regex(R"(^crate\b)")},
  {ELSE, std::regex(R"(^else\b)")},
  {ENUM, std::regex(R"(^enum\b)")},
  {FALSE, std::regex(R"(^false\b)")},
  {FN, std::regex(R"(^fn\b)")},
  {FOR, std::regex(R"(^for\b)")},
  {IF, std::regex(R"(^if\b)")},
  {IMPL, std::regex(R"(^impl\b)")},
  {IN, std::regex(R"(^in\b)")},
  {LET, std::regex(R"(^let\b)")},
  {LOOP, std::regex(R"(^loop\b)")},
  {MATCH, std::regex(R"(^match\b)")},
  {MOD, std::regex(R"(^mod\b)")},
  {MOVE, std::regex(R"(^move\b)")},
  {MUT, std::regex(R"(^mut\b)")},
  {REF, std::regex(R"(^ref\b)")},
  {RETURN, std::regex(R"(^return\b)")},
  {SELF, std::regex(R"(^self\b)")},
  {SELF_, std::regex(R"(^Self\b)")},
  {STATIC, std::regex(R"(^static\b)")},
  {STRUCT, std::regex(R"(^struct\b)")},
  {SUPER, std::regex(R"(^super\b)")},
  {TRAIT, std::regex(R"(^trait\b)")},
  {TRUE, std::regex(R"(^true\b)")},
  {TYPE, std::regex(R"(^type\b)")},
  {UNSAFE, std::regex(R"(^unsafe\b)")},
  {USE, std::regex(R"(^use\b)")},
  {WHERE, std::regex(R"(^where\b)")},
  {WHILE, std::regex(R"(^while\b)")},
  {DYN, std::regex(R"(^dyn\b)")},
  {ABSTRACT, std::regex(R"(^abstract\b)")},
  {BECOME, std::regex(R"(^become\b)")},
  {BOX, std::regex(R"(^box\b)")},
  {DO, std::regex(R"(^do\b)")},
  {FINAL, std::regex(R"(^final\b)")},
  {MACRO, std::regex(R"(^macro\b)")},
  {OVERRIDE, std::regex(R"(^override\b)")},
  {PRIV, std::regex(R"(^priv\b)")},
  {TYPEOF, std::regex(R"(^typeof\b)")},
  {UNSIZED, std::regex(R"(^unsized\b)")},
  {VIRTUAL, std::regex(R"(^virtual\b)")},
  {YIELD, std::regex(R"(^yield\b)")},
  {TRY, std::regex(R"(^try\b)")},
  {GEN, std::regex(R"(^gen\b)")},
  // Punctuation
  {SHL_EQ, std::regex(R"(^<<=)")},
  {SHR_EQ, std::regex(R"(^>>=)")},
  {DOT_DOT_DOT, std::regex(R"(^\.\.\.)")},
  {DOT_DOT_EQ, std::regex(R"(^\.\.=)")},
  {AND_AND, std::regex(R"(^&&)")},
  {OR_OR, std::regex(R"(^\|\|)")},
  {SHL, std::regex(R"(^<<)")},
  {SHR, std::regex(R"(^>>)")},
  {PLUS_EQ, std::regex(R"(^\+=)")},
  {MINUS_EQ, std::regex(R"(^\-=)")},
  {STAR_EQ, std::regex(R"(^\*=)")},
  {SLASH_EQ, std::regex(R"(^\/=)")},
  {PERCENT_EQ, std::regex(R"(^%=)")},
  {CARET_EQ, std::regex(R"(^\^=)")},
  {AND_EQ, std::regex(R"(^&=)")},
  {OR_EQ, std::regex(R"(^\|=)")},
  {EQ_EQ, std::regex(R"(^==)")},
  {NE, std::regex(R"(^!=)")},
  {GE, std::regex(R"(^>=)")},
  {LE, std::regex(R"(^<=)")},
  {DOT_DOT, std::regex(R"(^\.\.)")},
  {PATH_SEP, std::regex(R"(^::)")},
  {R_ARROW, std::regex(R"(^->)")},
  {FAT_ARROW, std::regex(R"(^=>)")},
  {L_ARROW, std::regex(R"(^<-)")},
  {PLUS, std::regex(R"(^\+)")},
  {MINUS, std::regex(R"(^\-)")},
  {STAR, std::regex(R"(^\*)")},
  {SLASH, std::regex(R"(^\/)")},
  {PERCENT, std::regex(R"(^%)")},
  {CARET, std::regex(R"(^\^)")},
  {NOT, std::regex(R"(^!)")},
  {AND, std::regex(R"(^&)")},
  {OR, std::regex(R"(^\|)")},
  {EQ, std::regex(R"(^=)")},
  {GT, std::regex(R"(^>)")},
  {LT, std::regex(R"(^<)")},
  {AT, std::regex(R"(^@)")},
  {UNDERSCORE, std::regex(R"(^_)")},
  {DOT, std::regex(R"(^\.)")},
  {COMMA, std::regex(R"(^,)")},
  {SEMI, std::regex(R"(^;)")},
  {COLON, std::regex(R"(^:)")},
  {POUND, std::regex(R"(^#)")},
  {DOLLAR, std::regex(R"(^\$)")},
  {QUESTION, std::regex(R"(^\?)")},
  {TILDE, std::regex(R"(^~)")},
  // Literals
  {CHAR_LITERAL, std::regex(R"(^'([^'\\\n\r\t]|\\['"]|\\[nrt\\0]|\\x[0-7][0-9a-fA-F])')")},
  {STRING_LITERAL, std::regex(R"(^"([^"\\\r]|\\['"]|\\[nrt\\0]|\\x[0-7][0-9a-fA-F]|\\\n)*")")},
  {RAW_STRING_LITERAL, std::regex(R"(^r(#*)\".*?\"\1)")},
  {CSTRING_LITERAL, std::regex(R"(^c"([^"\\\r\x00]|\\([nrt\\"]|x[0-7][0-9a-fA-F]|\n))*")")},
  {RAW_CSTRING_LITERAL, std::regex(R"(^cr(#*)\"[^\r\x00]*?\"\1)")},
  {INTEGER_LITERAL, std::regex(R"(^([0-9][0-9_]*|0b[0-1_]*[0-1][0-1_]*|0o[0-7_]*[0-7][0-7_]*|0x[0-9a-fA-F_]*[0-9a-fA-F][0-9a-fA-F_]*)([ui]32)?)")},
  // Identifiers
  {IDENTIFIER, std::regex(R"(^[a-zA-Z][a-zA-Z0-9_]*\b)")},
  // Delimiters
  {L_BRACE, std::regex(R"(^\{)")},
  {R_BRACE, std::regex(R"(^\})")},
  {L_BRACKET, std::regex(R"(^\[)")},
  {R_BRACKET, std::regex(R"(^\])")},
  {L_PAREN, std::regex(R"(^\()")},
  {R_PAREN, std::regex(R"(^\))")},
};
// mostly by AI end

void Lexer::removeComments() {
  while (src[pos] == '/'){
    if (src.length() == pos + 1 || src[pos + 1] != '/' && src[pos + 1] != '*') { // error
      throw std::runtime_error("lexer: not matched.");
    } else if (src[pos + 1] == '/') {
      for (pos = pos + 2; pos < src.length(); ++pos) {
        if (src[pos] == '\n') {
          break;
        }
      }
    } else if (src[pos + 1] == '*') {
      std::stack<int> stack;
      stack.push(pos);
      for (pos = pos + 2; pos < src.length() && stack.size(); ++pos) {
        if (src[pos] == '/') {
          if (src[pos + 1] == '*') {
            stack.push(pos);
            ++pos;
          }
        } else if (src[pos] == '*') {
          if (src[pos + 1] == '/') {
            stack.pop();
            ++pos;
          }
        }
      }
      if (stack.size() != 0) {
        throw std::runtime_error("lexer: not matched.");
      }
    }
  }
}

std::vector<Token> Lexer::tokenize(){
  std::vector<Token> tokens;
  while (pos < src.length()) {
    removeComments();
    bool is_matched = false;
    for (const auto &rule : rules) {
      std::smatch match;
      std::string str = src.substr(pos);
      if (std::regex_search(str, match, rule.pattern)){
        is_matched = true;
        if (rule.type != WHITESPACE){
          tokens.push_back({rule.type, match.str()});
        }
        pos += match.str().length();
        break;
      }
    }
    if (!is_matched) {
      throw std::runtime_error("lexer: not matched.");
    }
  }
  tokens.push_back({E_O_F, ""});
}
