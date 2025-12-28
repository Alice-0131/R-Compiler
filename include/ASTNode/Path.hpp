#ifndef PATH_HPP
#define PATH_HPP
#include "ASTNode.hpp"
#include "../Semantic/Type.hpp"
#include <string>

enum PathType { Identifier, Self, self };

class Path : ASTNode {
public:
  const QualType *Ty;

  PathType type;
  std::string identifier;

public:
  Path(PathType type, std::string identifier, TypeID Tid = K_Path)
      : type(type), identifier(identifier), ASTNode(Tid) {}
  void accept(ASTVisitor &visitor) override { visitor.visit(*this); }

  const QualType *setQualType(const QualType *Ty) {
    return this->Ty = Ty;
  }

  const QualType *getQualType() const {
    if (!Ty) {
      throw std::runtime_error("Path::getQualType: QualType is not set.");
    }
    return Ty;
  }
};

#endif