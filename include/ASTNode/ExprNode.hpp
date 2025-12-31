#ifndef EXPRNODE_HPP
#define EXPRNODE_HPP
#include "ASTNode.hpp"
#include "../Semantic/Type.hpp"

class ExprNode : public ASTNode
{
private:
  QualType *Ty = nullptr;
  bool mut = false;
  bool ret = false;

public:
  ExprNode(TypeID Tid = K_ExprNode) : ASTNode(Tid) {}

  virtual void accept(ASTVisitor &visitor) = 0;

  bool isMut(void) const { return mut; }

  void setMut(bool m) { mut = m; }

  bool hasRet(void) const { return ret; }

  void setRet(bool r) { ret = r; }

  const QualType *setQualType(const QualType *Ty) {
    this->Ty = const_cast<QualType *>(Ty);
    return this->Ty;
  }

  const QualType *getQualType() const {
    if (Ty == nullptr) {
      throw std::runtime_error("Type not set for ExprNode");
    }
    return Ty;
  }
};

class ExprWithoutBlockNode : public ExprNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

class ExprWithBlockNode : public ExprNode
{
public:
  virtual void accept(ASTVisitor &visitor) = 0;
};

#endif