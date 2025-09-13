#ifndef PATTERNLITERAL_HPP
#define PATTERNLITERAL_HPP
#include "ExprLiteral.hpp"
#include "PatternNode.hpp"
class ExprLiteralNode;

class PatternLiteral : public PatternNode
{
private:
  bool is_minus;
  std::unique_ptr<ExprLiteralNode> pattern;
public:
  PatternLiteral(bool is_minus, std::unique_ptr<ExprLiteralNode> pattern):
    is_minus(is_minus), pattern(std::move(pattern)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};


#endif