#ifndef EXPRCALL_HPP
#define EXPRCALL_HPP
#include "ExprNode.hpp"

class ExprCall : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> expr;
  std::vector<std::unique_ptr<ExprNode>> params;
public:
  ExprCall(std::unique_ptr<ExprNode> expr, std::vector<std::unique_ptr<ExprNode>> &&params):
    expr(std::move(expr)), params(std::move(params)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif