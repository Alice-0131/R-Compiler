#ifndef STMTLET_HPP
#define STMTLET_HPP
#include "StmtNode.hpp"
class PatternNode;
class TypeNode;
class ExprNode;

class StmtLet : public StmtNode
{
private:
  std::unique_ptr<PatternNode> pattern;
  std::unique_ptr<TypeNode> type;
  std::unique_ptr<ExprNode> expr;
public:
  StmtLet(std::unique_ptr<PatternNode> pattern, std::unique_ptr<TypeNode> type,
    std::unique_ptr<ExprNode> expr): pattern(std::move(pattern)), 
    type(std::move(type)), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif