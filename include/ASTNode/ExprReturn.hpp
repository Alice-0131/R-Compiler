#ifndef EXPRRETURN_HPP
#define EXPRRETURN_HPP
#include "ExprNode.hpp"

class ExprReturn : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;

  ExprReturn(std::shared_ptr<ExprNode> expr): expr(std::move(expr)),
    ExprWithoutBlockNode(K_ExprReturn){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};


#endif