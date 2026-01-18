#ifndef PATTERNPATH_HPP
#define PATTERNPATH_HPP
#include "PatternNode.hpp"
#include "Path.hpp"

class PatternPath : public PatternNode {
public:
  std::shared_ptr<Path> path;
  PatternPath(std::shared_ptr<Path> p) : path(std::move(p)), PatternNode(K_PatternPath) {}
  void accept(ASTVisitor &visitor) override { /**/ }
};
#endif
