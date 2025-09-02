#ifndef ITEMTRAIT_HPP
#define ITEMTRAIT_HPP
#include "ItemNode.hpp"

class ItemTrait : public ItemNode
{
private:
  std::string identifier;
  std::vector<ItemAssociatedNode> associated_item;
public:
  ItemTrait(std::string identifier, std::vector<ItemAssociatedNode> &&associated_item):
    identifier(identifier), associated_item(std::move(associated_item)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif