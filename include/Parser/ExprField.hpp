#ifndef EXPRFIELD_HPP
#define EXPRFIELD_HPP
#include "ExprNode.hpp"

class ExprField : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> expr;
  std::string identifier;
public:
  ExprField(std::unique_ptr<ExprNode> expr, std::string identifier): 
    expr(std::move(expr)), identifier(identifier){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif