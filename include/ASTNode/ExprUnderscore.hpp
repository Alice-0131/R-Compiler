#ifndef EXPRUNDERSCORE_HPP
#define EXPRUNDERSCORE_HPP
#include "ExprNode.hpp"

class ExprUnderscore : public ExprWithoutBlockNode {
public:
  ExprUnderscore() : ExprWithoutBlockNode(K_ExprUnderscore) {}
  void accept(ASTVisitor &visitor) override { /**/ }
};
#endif
