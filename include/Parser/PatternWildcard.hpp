#ifndef PATTERNWILDCARD_HPP
#define PATTERNWILDCARD_HPP
#include "PatternNode.hpp"

class PatternWildcard : public PatternNode
{
public:
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif