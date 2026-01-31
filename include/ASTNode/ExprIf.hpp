#ifndef EXPRIF_HPP
#define EXPRIF_HPP
#include "ExprNode.hpp"

class ExprIf : public ExprWithBlockNode
{
public:
  std::shared_ptr<ExprNode> condition;
  std::shared_ptr<ExprBlock> if_block;
  std::shared_ptr<ExprNode> else_block;

  ExprIf(std::shared_ptr<ExprNode> condition, std::shared_ptr<ExprBlock> if_block,
    std::shared_ptr<ExprNode> else_block): condition(std::move(condition)),
    if_block(std::move(if_block)), else_block(std::move(else_block)),
    ExprWithBlockNode(K_ExprIf){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif