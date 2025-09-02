#ifndef ITEMIMPL_HPP
#define ITEMIMPL_HPP
#include "ItemNode.hpp"
struct ItemAssociated;
class TypeNode;

class ItemImpl : public ItemNode
{
private:
  std::string identifier;
  std::unique_ptr<TypeNode> type;
  std::vector<ItemAssociated> associated_item;
public:
  ItemImpl(std::string identifier, std::unique_ptr<TypeNode> type,
    std::vector<ItemAssociated> &&associated_item): identifier(identifier), 
    type(std::move(type)), associated_item(std::move(associated_item)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif