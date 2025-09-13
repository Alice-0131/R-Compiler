#ifndef ITEMENUM_HPP
#define ITEMENUM_HPP
#include "ItemNode.hpp"

class ItemEnum :public ItemNode
{
private:
  std::string identifier;
  std::vector<std::string> enum_variants;
public:
  ItemEnum(std::string identifier, std::vector<std::string> &enum_variants):
    identifier(identifier), enum_variants(enum_variants){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif