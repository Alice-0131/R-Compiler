#ifndef PATTERNIDENTIFIER_HPP
#define PATTERNIDENTIFIER_HPP
#include "PatternNode.hpp"

class PatternIdentifier : public PatternNode
{
private:
  bool is_ref;
  bool is_mut;
  std::string identifier;
  std::unique_ptr<PatternNode> pattern;
public:
  PatternIdentifier(bool is_ref, bool is_mut, std::string identifier,
    std::unique_ptr<PatternNode> pattern): is_ref(is_ref), is_mut(is_mut),
    identifier(identifier), pattern(std::move(pattern)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif