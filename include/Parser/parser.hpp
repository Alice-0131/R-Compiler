#ifndef PARSER_HPP
#define PARSER_HPP
#include <vector>
#include <memory>
#include "token.hpp"
#include "ASTNode.hpp"

class Parser
{
private:
  const std::vector<Token>& tokens;
  int pos;



public:
  Parser(const std::vector<Token>& tokens): tokens(tokens), pos(0){}
  std::unique_ptr<Crate> parse();
};


#endif