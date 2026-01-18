#ifndef PATTERNLITERAL_HPP
#define PATTERNLITERAL_HPP
#include "PatternNode.hpp"
#include "ExprLiteral.hpp" 
// Assuming pattern literal holds a literal expr or value
class PatternLiteral : public PatternNode {
public:
  std::shared_ptr<ExprLiteralNode> literal;
  PatternLiteral(std::shared_ptr<ExprLiteralNode> lit) : literal(std::move(lit)), PatternNode(K_PatternLiteral) {}
  void accept(ASTVisitor &visitor) override { /**/ }
};
#endif
