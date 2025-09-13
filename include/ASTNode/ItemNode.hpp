#ifndef ITEMNODE_HPP
#define ITEMNODE_HPP
#include "ASTNode.hpp"

class ItemNode : public ASTNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ItemAssociatedNode : public ItemNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif