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
private:
  char literal;
public:
  ExprLiteralChar(char literal): literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralString : public ExprLiteralNode
{
private:
  std::string literal;
public:
  ExprLiteralString(std::string literal) : literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralInt : public ExprLiteralNode
{
private:
  int literal;
public:
  ExprLiteralInt(int literal): literal(literal){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralBool : public ExprLiteralNode
{
private:
  bool literal;
public:
  ExprLiteralBool(bool literal) : literal(literal) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif