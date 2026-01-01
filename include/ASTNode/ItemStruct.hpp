#ifndef ITEMSTRUCT_HPP
#define ITEMSTRUCT_HPP
#include "ItemNode.hpp"
#include "../Semantic/Type.hpp"
class TypeNode;

struct StructField
{
  std::string identifier;
  std::shared_ptr<TypeNode> type;
};


class ItemStruct : public ItemNode
{
private:
  QualType *Ty = nullptr;
public:
  std::string identifier;
  std::vector<StructField> struct_fields;

  ItemStruct(std::string identifier, std::vector<StructField> &&struct_fields): 
    identifier(identifier), struct_fields(std::move(struct_fields)), ItemNode(K_ItemStruct){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}

  void setQualType(const QualType *Ty) {
    this->Ty = const_cast<QualType *>(Ty);
  }

  const QualType *getQualType() const {
    if (Ty == nullptr) {
      throw std::runtime_error("QualType of ItemStruct is not set.");
    }
    return Ty;
  }
};


#endif