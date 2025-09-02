#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include "token.hpp"

class Lexer
{
private:
  std::string src;
  int pos;
  void removeComments();

public:
  Lexer(std::string &src): src(src), pos(0){};
  std::vector<Token> tokenize();
};

#endif