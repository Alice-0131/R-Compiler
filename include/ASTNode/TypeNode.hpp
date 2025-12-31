#ifndef TYPENODE_HPP
#define TYPENODE_HPP
#include "ASTNode.hpp"
#include "../Semantic/Type.hpp"

class TypeNode : public ASTNode
{
public:
  const QualType *Ty;

  TypeNode(TypeID Tid) : ASTNode(Tid) {}
  virtual void accept(ASTVisitor &visitor) = 0;

  const QualType * setQualType(const QualType *Ty) {
    return this->Ty = Ty;
  }

  const QualType *getQualType() const {
    return Ty;
  }
};

#endif