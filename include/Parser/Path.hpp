#ifndef PATH_HPP
#define PATH_HPP
#include <string>
#include "ASTNode.hpp"

class Path : ASTNode
{
private:
  int flag; // 0: identifier; 1: Self; 2: self
  std::string identifier;
public:
  Path(int flag, std::string identifier): flag(flag), identifier(identifier){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif