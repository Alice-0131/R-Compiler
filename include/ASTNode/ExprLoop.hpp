#ifndef EXPRLOOP_HPP
#define EXPRLOOP_HPP
#include "ExprNode.hpp"
class ExprBlock;

class ExprLoopNode : public ExprWithBlockNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprLoopInfinite : public ExprLoopNode
{
public:
  std::shared_ptr<ExprBlock> block;

  ExprLoopInfinite(std::shared_ptr<ExprBlock> block): block(std::move(block)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLoopPredicate : public ExprLoopNode
{
public:
  std::shared_ptr<ExprNode> condition;
  std::shared_ptr<ExprBlock> block;

  ExprLoopPredicate(std::shared_ptr<ExprNode> condition, std::shared_ptr<ExprBlock> block):
    condition(std::move(condition)), block(std::move(block)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprBreak : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;

  ExprBreak(std::shared_ptr<ExprNode> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprContinue : public ExprWithoutBlockNode
{
public:
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif