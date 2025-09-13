#ifndef PATTERNNODE_HPP
#define PATTERNNODE_HPP
#include "ASTNode.hpp"

class PatternNode : public ASTNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif