#ifndef TYPENODE_HPP
#define TYPENODE_HPP
#include "ASTNode.hpp"

class TypeNode : public ASTNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif