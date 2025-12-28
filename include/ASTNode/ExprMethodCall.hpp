#ifndef EXPRMETHODCALL_HPP
#define EXPRMETHODCALL_HPP
#include "ExprNode.hpp"

class ExprMethodCall : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;
  std::shared_ptr<Path> path;
  std::vector<std::shared_ptr<ExprNode>> params;

  ExprMethodCall(std::shared_ptr<ExprNode> expr, std::shared_ptr<Path> path,
    std::vector<std::shared_ptr<ExprNode>> &&params): expr(std::move(expr)),
    path(std::move(path)), params(std::move(params)) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif