#ifndef PATTERNIDENTIFIER_HPP
#define PATTERNIDENTIFIER_HPP
#include "PatternNode.hpp"

class PatternIdentifier : public PatternNode
{
private:
  bool is_ref;
  bool is_mut;
  std::string identifier;
public:
  PatternIdentifier(bool is_ref, bool is_mut, std::string identifier): 
    is_ref(is_ref), is_mut(is_mut), identifier(identifier){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif