#ifndef PATTERNPATH_HPP
#define PATTERNPATH_HPP
#include "PatternNode.hpp"

class PatternPath : public PatternNode
{
private:
  std::unique_ptr<ExprPath> expr;
public:
  PatternPath(std::unique_ptr<ExprPath> expr): expr(std::move(expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif