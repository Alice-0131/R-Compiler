#ifndef STMTNODE_HPP
#define STMTNODE_HPP
#include "ASTNode.hpp"

class StmtNode : public ASTNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif