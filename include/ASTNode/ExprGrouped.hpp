#ifndef EXPRGROUPED_HPP
#define EXPRGROUPED_HPP
#include "ExprNode.hpp"

class ExprGrouped : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;

  ExprGrouped(std::shared_ptr<ExprNode> expr): 
    expr(std::move(expr)), ExprWithoutBlockNode(K_ExprGrouped){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif