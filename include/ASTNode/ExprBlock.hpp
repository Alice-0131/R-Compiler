#ifndef EXPRBLOCK_HPP
#define EXPRBLOCK_HPP
#include "ExprNode.hpp"
class StmtNode;

class ExprBlock : public ExprWithBlockNode
{
public:
  std::vector<std::shared_ptr<StmtNode>> stmts;
  std::shared_ptr<ExprNode> expr;

  ExprBlock(std::vector<std::shared_ptr<StmtNode>> &&stmts, std::shared_ptr<ExprNode> expr):
    stmts(std::move(stmts)), expr(std::move(expr)), ExprWithBlockNode(K_ExprBlock){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif