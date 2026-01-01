#ifndef ITEMCONST_HPP
#define ITEMCONST_HPP
#include "ItemNode.hpp"
class TypeNode;
class ExprNode;

class ItemConst : public ItemAssociatedNode
{
public:
  std::string identifier;
  std::shared_ptr<TypeNode> type;
  std::shared_ptr<ExprNode> expr;

  ItemConst(std::string identifier, std::shared_ptr<TypeNode> type, 
    std::shared_ptr<ExprNode> expr): identifier(identifier), 
    type(std::move(type)), expr(std::move(expr)),
    ItemAssociatedNode(K_ItemConst){}
  void accept(ASTVisitor &visitor) {visitor.visit(*this);}
};

#endif