#ifndef PATTERNREFERENCE_HPP
#define PATTERNREFERENCE_HPP
#include "PatternNode.hpp"

class PatternReference : public PatternNode
{
public:
  bool is_and; // true: &; false: &&
  bool is_mut;
  std::shared_ptr<PatternNode> pattern;
public:
  PatternReference(bool is_and, bool is_mut, std::shared_ptr<PatternNode> pattern):
    is_and(is_and), is_mut(is_mut), pattern(std::move(pattern)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif