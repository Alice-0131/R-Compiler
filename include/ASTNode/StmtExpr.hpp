#ifndef STMTEXPR_HPP
#define STMTEXPR_HPP
#include "StmtNode.hpp"
class ExprNode;

class StmtExpr : public StmtNode
{
public:
  std::shared_ptr<ExprNode> expr;

  StmtExpr(std::shared_ptr<ExprNode> expr): StmtNode(K_StmtExpr), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif