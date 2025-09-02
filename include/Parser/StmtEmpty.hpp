#ifndef STMTEMPTY_HPP
#define STMTEMPTY_HPP
#include "StmtNode.hpp"

class StmtEmpty : public StmtNode
{
public:
  StmtEmpty(){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif