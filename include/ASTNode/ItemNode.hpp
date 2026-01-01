#ifndef ITEMNODE_HPP
#define ITEMNODE_HPP
#include "ASTNode.hpp"

class ItemNode : public ASTNode
{
public:
  ItemNode(TypeID Tid) : ASTNode(Tid) {}
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ItemAssociatedNode : public ItemNode
{
public:
  ItemAssociatedNode(TypeID Tid) : ItemNode(Tid) {}
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif