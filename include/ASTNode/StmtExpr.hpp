#ifndef STMTEXPR_HPP
#define STMTEXPR_HPP
#include "StmtNode.hpp"
class ExprNode;

class StmtExpr : public StmtNode
{
private:
  std::unique_ptr<ExprNode> expr;
public:
  StmtExpr(std::unique_ptr<ExprNode> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif