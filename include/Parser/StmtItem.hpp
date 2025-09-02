#ifndef STMTITEM_HPP
#define STMTITEM_HPP
#include "StmtNode.hpp"
class ItemNode;

class StmtItem : public StmtNode
{
private:
  std::unique_ptr<ItemNode> item;
public:
  StmtItem(std::unique_ptr<ItemNode> item): item(std::move(item)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif