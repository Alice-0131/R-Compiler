#ifndef ITEMIMPL_HPP
#define ITEMIMPL_HPP
#include "ItemNode.hpp"
class TypeNode;

class ItemImpl : public ItemNode
{
private:
  std::string identifier;
  std::unique_ptr<TypeNode> type;
  std::vector<std::unique_ptr<ItemAssociatedNode>> associated_items;
public:
  ItemImpl(std::string identifier, std::unique_ptr<TypeNode> type,
    std::vector<std::unique_ptr<ItemAssociatedNode>> &&associated_items): identifier(identifier), 
    type(std::move(type)), associated_items(std::move(associated_items)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif