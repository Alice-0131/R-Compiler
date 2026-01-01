#ifndef TYPEPATH_HPP
#define TYPEPATH_HPP
#include "TypeNode.hpp"
#include "Path.hpp"

class TypePath : public TypeNode
{
public:
  std::shared_ptr<Path> path;
public:
  TypePath(std::shared_ptr<Path> path): path(std::move(path)), TypeNode(K_TypePath){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}

  std::string getTypeName() { return path->identifier; }
};

#endif