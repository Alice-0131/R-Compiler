#ifndef EXPRFIELD_HPP
#define EXPRFIELD_HPP
#include "ExprNode.hpp"

class ExprField : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> expr;
  std::string identifier;

  ExprField(std::shared_ptr<ExprNode> expr, std::string identifier): 
    expr(std::move(expr)), identifier(identifier){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif