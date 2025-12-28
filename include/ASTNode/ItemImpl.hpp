#ifndef ITEMIMPL_HPP
#define ITEMIMPL_HPP
#include "ItemNode.hpp"
class TypeNode;

class ItemImpl : public ItemNode
{
public:
  std::string identifier;
  std::shared_ptr<TypeNode> type;
  std::vector<std::shared_ptr<ItemAssociatedNode>> associated_items;

  ItemImpl(std::string identifier, std::shared_ptr<TypeNode> type,
    std::vector<std::shared_ptr<ItemAssociatedNode>> &&associated_items): identifier(identifier), 
    type(std::move(type)), associated_items(std::move(associated_items)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif