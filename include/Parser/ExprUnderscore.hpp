#ifndef EXPRUNDERSCORE_HPP
#define EXPRUNDERSCORE_HPP
#include "ExprNode.hpp"

class ExprUnderscore : public ExprNode
{
public:
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif