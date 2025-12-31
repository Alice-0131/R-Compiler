#ifndef STMTLET_HPP
#define STMTLET_HPP
#include "StmtNode.hpp"
class PatternNode;
class TypeNode;
class ExprNode;

class StmtLet : public StmtNode
{
public:
  std::shared_ptr<PatternNode> pattern;
  std::shared_ptr<TypeNode> type;
  std::shared_ptr<ExprNode> expr;

  StmtLet(std::shared_ptr<PatternNode> pattern, std::shared_ptr<TypeNode> type,
    std::shared_ptr<ExprNode> expr): StmtNode(K_StmtLet), pattern(std::move(pattern)), 
    type(std::move(type)), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif