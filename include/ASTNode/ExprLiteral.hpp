#ifndef EXPRLITERAL_HPP
#define EXPRLITERAL_HPP
#include "ExprNode.hpp"

class ExprLiteralNode : public ExprWithoutBlockNode
{
public:
  ExprLiteralNode(TypeID Tid) : ExprWithoutBlockNode(Tid) {}
  void accept(ASTVisitor &visitor) = 0;
};

class ExprLiteralChar : public ExprLiteralNode
{
public:
  char literal;
public:
  ExprLiteralChar(char literal): literal(literal), ExprLiteralNode(K_ExprLiteralChar) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralString : public ExprLiteralNode
{
public:
  std::string literal;
public:
  ExprLiteralString(std::string literal) : literal(literal) , ExprLiteralNode(K_ExprLiteralString){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralInt : public ExprLiteralNode
{
public:
  long literal;
  std::string type;
public:
  ExprLiteralInt(int literal): literal(literal), ExprLiteralNode(K_ExprLiteralInt){}
  ExprLiteralInt(long literal, std::string type)
    : literal(literal), type(type), ExprLiteralNode(K_ExprLiteralInt) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprLiteralBool : public ExprLiteralNode
{
public:
  bool literal;
public:
  ExprLiteralBool(bool literal) : literal(literal), ExprLiteralNode(K_ExprLiteralBool) {}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif