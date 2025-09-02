#ifndef EXPRBLOCK_HPP
#define EXPRBLOCK_HPP
#include "ExprNode.hpp"
class StmtNode;

class ExprBlock : public ExprWithBlockNode
{
private:
  std::vector<std::unique_ptr<StmtNode>> stmts;
  std::unique_ptr<ExprWithoutBlockNode> expr;
public:
  ExprBlock(std::vector<std::unique_ptr<StmtNode>> &&stmts, std::unique_ptr<ExprWithoutBlockNode> expr):
    stmts(std::move(stmts)), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif