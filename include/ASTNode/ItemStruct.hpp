#ifndef ITEMSTRUCT_HPP
#define ITEMSTRUCT_HPP
#include "ItemNode.hpp"
class TypeNode;

struct StructField
{
  std::string identifier;
  std::unique_ptr<TypeNode> type;
};


class ItemStruct : public ItemNode
{
private:
  std::string identifier;
  std::vector<StructField> struct_fields;
public:
  ItemStruct(std::string identifier, std::vector<StructField> &&struct_fields): 
    identifier(identifier), struct_fields(std::move(struct_fields)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};


#endif