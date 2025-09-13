#ifndef EXPRGROUPED_HPP
#define EXPRGROUPED_HPP
#include "ExprNode.hpp"

class ExprGrouped : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> expr;
public:
  ExprGrouped(std::unique_ptr<ExprNode> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif