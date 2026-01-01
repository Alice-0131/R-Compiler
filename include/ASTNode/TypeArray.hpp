#ifndef TYPEARRAY_HPP
#define TYPEARRAY_HPP
#include "TypeNode.hpp"
class ExprNode;

class TypeArray : public TypeNode
{
public:
  std::shared_ptr<TypeNode> type;
  std::shared_ptr<ExprNode> expr;

  TypeArray(std::shared_ptr<TypeNode> type, std::shared_ptr<ExprNode> expr):
    type(std::move(type)), expr(std::move(expr)), TypeNode(K_TypeArray){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif