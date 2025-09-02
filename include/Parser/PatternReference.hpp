#ifndef PATTERNREFERENCE_HPP
#define PATTERNREFERENCE_HPP
#include "PatternNode.hpp"

class PatternReference : public PatternNode
{
private:
  bool is_mut;
  std::unique_ptr<PatternNode> pattern;
public:
  PatternReference(bool is_mut, std::unique_ptr<PatternNode> pattern):
    is_mut(is_mut), pattern(std::move(pattern)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif