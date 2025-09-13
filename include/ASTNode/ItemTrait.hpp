#ifndef ITEMTRAIT_HPP
#define ITEMTRAIT_HPP
#include "ItemNode.hpp"

class ItemTrait : public ItemNode
{
private:
  std::string identifier;
  std::vector<std::unique_ptr<ItemAssociatedNode>> associated_items;
public:
  ItemTrait(std::string identifier, std::vector<std::unique_ptr<ItemAssociatedNode>> &&associated_items):
    identifier(identifier), associated_items(std::move(associated_items)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif