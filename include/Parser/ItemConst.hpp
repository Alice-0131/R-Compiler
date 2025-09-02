#ifndef ITEMCONST_HPP
#define ITEMCONST_HPP
#include "ItemNode.hpp"
class TypeNode;
class ExprNode;

class ItemConst : public ItemAssociatedNode
{
private:
  std::string identifier;
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<ExprNode> expr;
public:
  ItemConst(std::string identifier, std::unique_ptr<TypeNode> type, 
    std::unique_ptr<ExprNode> expr): identifier(identifier), 
    type(std::move(type)), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) {visitor.visit(*this);}
};

#endif