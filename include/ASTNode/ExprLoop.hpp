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
private:
  std::unique_ptr<ExprBlock> block;
public:
  ExprLoopInfinite(std::unique_ptr<ExprBlock> block): block(std::move(block)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLoopPredicate : public ExprLoopNode
{
private:
  std::unique_ptr<ExprNode> condition;
  std::unique_ptr<ExprBlock> block;
public:
  ExprLoopPredicate(std::unique_ptr<ExprNode> condition, std::unique_ptr<ExprBlock> block):
    condition(std::move(condition)), block(std::move(block)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprBreak : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> expr;
public:
  ExprBreak(std::unique_ptr<ExprNode> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprContinue : public ExprWithoutBlockNode
{
public:
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif