#ifndef STMTITEM_HPP
#define STMTITEM_HPP
#include "StmtNode.hpp"
class ItemNode;

class StmtItem : public StmtNode
{
public:
  std::shared_ptr<ItemNode> item;

  StmtItem(std::shared_ptr<ItemNode> item): item(std::move(item)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif