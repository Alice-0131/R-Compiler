#ifndef STMTEMPTY_HPP
#define STMTEMPTY_HPP
#include "StmtNode.hpp"

class StmtEmpty : public StmtNode
{
public:
  StmtEmpty():StmtNode(K_StmtEmpty){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif