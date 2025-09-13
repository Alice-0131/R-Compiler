#ifndef PATH_HPP
#define PATH_HPP
#include <string>
#include "ASTNode.hpp"

enum PathType
{
  Identifier,
  Self,
  self
};

class Path : ASTNode
{
private:
  PathType type;
  std::string identifier;
public:
  Path(PathType type, std::string identifier): type(type), identifier(identifier){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif