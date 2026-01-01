#ifndef EXPRCALL_HPP
#define EXPRCALL_HPP
#include "ExprNode.hpp"

class ExprCall : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;
  std::vector<std::shared_ptr<ExprNode>> params;

  ExprCall(std::shared_ptr<ExprNode> expr, std::vector<std::shared_ptr<ExprNode>> &&params):
    expr(std::move(expr)), params(std::move(params)), ExprWithoutBlockNode(K_ExprCall){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif