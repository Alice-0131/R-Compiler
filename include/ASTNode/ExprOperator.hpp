#ifndef EXPROPERATOR_HPP
#define EXPROPERATOR_HPP
#include "ExprNode.hpp"
class TypeNode;

enum ExprOpUnaryType
{
  BORROW_,      // & | &&
  MUT_BORROW_,  // & | && mut
  DEREFERENCE_, // *
  NEGATE_,      // -
  NOT_          // !
};

enum ExprOpBinaryType
{
  // Arithmetic and Logical Binary Operators
  PLUS_,  // +
  MINUS_, // -
  MUL_,   // *
  DIV_,   // /
  MOD_,   // %
  AND_,   // &
  OR_,    // |
  XOR_,   // ^
  SHL_,   // <<
  SHR_,   // >>
  // Comparison Operators
  EQUAL_,         // ==
  NOT_EQUAL_,     // !=
  GREATER_,       // >
  LESS_,          // <
  GREATER_EQUAL_, // >=
  LESS_EQUAL_,    // <=
  // Lazy Boolean Opearators
  OR_OR_,   // ||
  AND_AND_, // &&
  // Assignment expressions
  ASSIGN_, // =
  // Compound assignment expressions
  PLUS_EQ_,  // +=
  MINUS_EQ_, // -=
  MUL_EQ_,   // *=
  DIV_EQ_,   // /=
  MOD_EQ_,   // %=
  AND_EQ_,   // &=
  OR_EQ_,    // |=
  XOR_EQ_,   // ^=
  SHL_EQ_,   // <<=
  SHR_EQ_,   // >>=
};

class ExprOperatorNode : public ExprNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprOpUnary : public ExprOperatorNode
{
public:
  ExprOpUnaryType type;
  std::shared_ptr<ExprNode> expr;

  ExprOpUnary(ExprOpUnaryType type, std::shared_ptr<ExprNode> expr): 
    type(type), expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprOpBinary : public ExprOperatorNode
{
public:
  ExprOpBinaryType type;
  std::shared_ptr<ExprNode> left;
  std::shared_ptr<ExprNode> right;

  ExprOpBinary(ExprOpBinaryType type, std::shared_ptr<ExprNode> left, 
    std::shared_ptr<ExprNode> right): type(type), 
    left(std::move(left)), right(std::move(right)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprOpCast : public ExprOperatorNode
{
public:
  std::shared_ptr<ExprNode> expr;
  std::shared_ptr<TypeNode> type;

  ExprOpCast(std::shared_ptr<ExprNode> expr, std::shared_ptr<TypeNode> type):
    expr(std::move(expr)), type(std::move(type)){}
  void accept(ASTVisitor &visitor) override{visitor.visit(*this);}
};

#endif