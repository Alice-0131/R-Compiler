#ifndef EXPRMETHODCALL_HPP
#define EXPRMETHODCALL_HPP
#include "ExprNode.hpp"

class ExprMethodCall : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> expr;
  std::unique_ptr<Path> path;
  std::vector<std::unique_ptr<ExprNode>> params;
public:
  ExprMethodCall(std::unique_ptr<ExprNode> expr, std::unique_ptr<Path> path,
    std::vector<std::unique_ptr<ExprNode>> &&params): expr(std::move(expr)),
    path(std::move(path)), params(std::move(params)) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif