#ifndef EXPRIF_HPP
#define EXPRIF_HPP
#include "ExprNode.hpp"

class ExprIf : public ExprNode
{
private:
  std::unique_ptr<ExprNode> condition;
  std::unique_ptr<ExprBlock> if_block;
  std::unique_ptr<ExprNode> else_block;
public:
  ExprIf(std::unique_ptr<ExprNode> condition, std::unique_ptr<ExprBlock> if_block,
    std::unique_ptr<ExprNode> else_block): condition(std::move(condition)),
    if_block(std::move(if_block)), else_block(std::move(else_block)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif