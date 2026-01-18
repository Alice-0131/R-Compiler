#ifndef PATTERNWILDCARD_HPP
#define PATTERNWILDCARD_HPP
#include "PatternNode.hpp"

class PatternWildcard : public PatternNode {
public:
  PatternWildcard() : PatternNode(K_PatternWildcard) {}
  void accept(ASTVisitor &visitor) override { /**/ }
};
#endif
