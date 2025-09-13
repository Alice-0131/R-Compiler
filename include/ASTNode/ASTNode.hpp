#ifndef ASTNODE_HPP
#define ASTNODE_HPP
#include <vector>
#include <string>
#include <memory>
#include "../ASTVisitor/ASTVisitor.hpp"

class ASTNode
{
public:
  //virtual void print() const = 0; // debug
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif