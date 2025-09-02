#ifndef EXPROPERATOR_HPP
#define EXPROPERATOR_HPP
#include "ExprNode.hpp"
class TypeNode;

enum ExprOpUnaryType
{
  BORROW,      // & | &&
  MUT_BORROW,  // & | && mut
  DEREFERENCE, // *
  NEGATE,      // -
  NOT          // !
};

enum ExprOpBinaryType
{
  // Arithmetic and Logical Binary Operators
  PLUS,  // +
  MINUS, // -
  MUL,   // *
  DIV,   // /
  MOD,   // %
  AND,   // &
  OR,    // |
  XOR,   // ^
  SHL,   // <<
  SHR,   // >>
  // Comparison Operators
  EQUAL,         // ==
  NOT_EQUAL,     // !=
  GREATER,       // >
  LESS,          // <
  GREATER_EQUAL, // >=
  LESS_EQUAL,    // <=
  // Lazy Boolean Opearators
  OR_OR,   // ||
  AND_AND, // &&
};

class ExprOperatorNode : public ExprNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprOpUnary : public ExprOperatorNode
{
private:
  ExprOpUnaryType type;
  std::unique_ptr<ExprNode> expr;
public:
  ExprOpUnary(ExprOpUnaryType type, std::unique_ptr<ExprNode> expr): 
    type(type), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprOpBinary : public ExprOperatorNode
{
private:
  ExprOpBinaryType type;
  std::unique_ptr<ExprNode> left;
  std::unique_ptr<ExprNode> right;
public:
  ExprOpBinary(ExprOpBinaryType type, std::unique_ptr<ExprNode> left, 
    std::unique_ptr<ExprNode> right): type(type), 
    left(std::move(left)), right(std::move(right)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprOpCast : public ExprOperatorNode
{
private:
  std::unique_ptr<ExprNode> expr;
  std::unique_ptr<TypeNode> type;
public:
  ExprOpCast(std::unique_ptr<ExprNode> expr, std::unique_ptr<TypeNode> type):
    expr(std::move(expr)), type(std::move(type)){}
  void accept(ASTVisitor &visitor) override{visitor.visit(*this);}
};

#endif