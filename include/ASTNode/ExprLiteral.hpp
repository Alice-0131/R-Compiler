#ifndef EXPRLITERAL_HPP
#define EXPRLITERAL_HPP
#include "ExprNode.hpp"

class ExprLiteralNode : public ExprWithoutBlockNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprLiteralChar : public ExprLiteralNode
{
public:
  char literal;
public:
  ExprLiteralChar(char literal): literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralString : public ExprLiteralNode
{
public:
  std::string literal;
public:
  ExprLiteralString(std::string literal) : literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralInt : public ExprLiteralNode
{
public:
  long literal;
  std::string type;
public:
  ExprLiteralInt(int literal): literal(literal){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralBool : public ExprLiteralNode
{
public:
  bool literal;
public:
  ExprLiteralBool(bool literal) : literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif