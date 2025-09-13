#ifndef CRATE_HPP
#define CRATE_HPP
#include <vector>
#include <memory>
#include "ASTNode.hpp"

class ItemNode;
class Crate : public ASTNode
{
public:
  std::vector<std::unique_ptr<ItemNode>> children;

  Crate(std::vector<std::unique_ptr<ItemNode>> &&children) : children(std::move(children)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif