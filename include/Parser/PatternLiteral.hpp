#ifndef PATTERNLITERAL_HPP
#define PATTERNLITERAL_HPP
#include "PatternNode.hpp"

class PatternLiteral : public PatternNode
{
private:
  bool is_minus;
  std::unique_ptr<ExprLiteral> pattern;
public:
  PatternLiteral(bool is_minus, std::unique_ptr<ExprLiteral> pattern):
    is_minus(is_minus), pattern(std::move(pattern)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};


#endif