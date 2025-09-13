#ifndef TYPEREFERENCE_HPP
#define TYPEREFERENCE_HPP
#include "TypeNode.hpp"

class TypeReference : public TypeNode
{
private:
  bool is_mut;
  std::unique_ptr<TypeNode> type;
public:
  TypeReference(bool is_mut, std::unique_ptr<TypeNode> type):
    is_mut(is_mut), type(std::move(type)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif