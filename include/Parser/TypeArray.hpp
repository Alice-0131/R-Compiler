#ifndef TYPEARRAY_HPP
#define TYPEARRAY_HPP
#include "TypeNode.hpp"
class ExprNode;

class TypeArray : public TypeNode
{
private:
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<ExprNode> expr;
public:
  TypeArray(std::unique_ptr<TypeNode> type, std::unique_ptr<ExprNode> expr):
    type(std::move(type)), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif