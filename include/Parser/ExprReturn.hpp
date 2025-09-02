#ifndef EXPRRETURN_HPP
#define EXPRRETURN_HPP
#include "ExprNode.hpp"

class ExprReturn : public ExprNode
{
private:
  std::unique_ptr<ExprNode> expr;
public:
  ExprReturn(std::unique_ptr<ExprNode> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};


#endif