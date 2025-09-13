#ifndef EXPRNODE_HPP
#define EXPRNODE_HPP
#include "ASTNode.hpp"

class ExprNode : public ASTNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ExprWithoutBlockNode : public ExprNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ExprWithBlockNode : public ExprNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif