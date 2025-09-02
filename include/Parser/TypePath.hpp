#ifndef TYPEPATH_HPP
#define TYPEPATH_HPP
#include "TypeNode.hpp"

class TypePath : public TypeNode
{
private:
  std::unique_ptr<Path> path;
public:
  TypePath(std::unique_ptr<Path> path): path(std::move(path)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif