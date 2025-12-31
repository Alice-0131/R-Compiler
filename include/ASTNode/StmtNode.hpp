#ifndef STMTNODE_HPP
#define STMTNODE_HPP
#include "ASTNode.hpp"

class StmtNode : public ASTNode
{
private:
  bool ret = false;

public:
  StmtNode(TypeID Tid) : ASTNode(Tid) {}
  virtual void accept(ASTVisitor &visitor) = 0;
  bool hasRet(void) const { return ret; }
  void setRet(bool r) { ret = r; }
};

#endif