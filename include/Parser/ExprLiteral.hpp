#ifndef EXPRLITERAL_HPP
#define EXPRLITERAL_HPP
#include "ExprNode.hpp"

class ExprLiteral : public ExprWithoutBlockNode
{
private:
  std::string literal;
public:
  ExprLiteral(std::string literal): literal(literal){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif