#ifndef TYPEUNIT_HPP
#define TYPEUNIT_HPP
#include "TypeNode.hpp"

class TypeUnit : public TypeNode
{
public:
  TypeUnit() : TypeNode(K_TypeUnit) {}
  void accept(ASTVisitor &visitor) override{visitor.visit(*this);}
};

#endif