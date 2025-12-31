#ifndef ITEMFN_HPP
#define ITEMFN_HPP
#include "ItemNode.hpp"
#include "../Semantic/Type.hpp"
class TypeNode;
class PatternNode;
class ExprNode;
class ExprBlock;

struct SelfParam
{
  bool is_and = false;
  bool is_mut = false;
};

struct FnParam
{
  std::shared_ptr<PatternNode> pattern;
  std::shared_ptr<TypeNode> type;
};

struct FnParameters
{
  SelfParam self_param;
  std::vector<FnParam> fn_params;
};

struct VarDecl {
  std::string Name;
  const QualType *Ty;
  bool mut;
};

class ItemFn : public ItemAssociatedNode
{
private:
  const FuncQualType *Ty = nullptr;
  std::unordered_map<std::string, VarDecl> VarDecls;
public:
  bool is_const;
  std::string identifier;
  FnParameters function_parameters;
  std::shared_ptr<TypeNode> function_return_type;
  std::shared_ptr<ExprBlock> block_expr;

  ItemFn(bool is_const, std::string identifier, FnParameters &&function_parameters, 
    std::shared_ptr<TypeNode> function_return_type, std::shared_ptr<ExprBlock> block_expr): 
    is_const(is_const), identifier(identifier), function_parameters(std::move(function_parameters)), 
    function_return_type(std::move(function_return_type)), block_expr(std::move(block_expr)){}

  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}

  const FuncQualType *getQualType() const { return Ty; }

  void setFunctionType(const FuncQualType *Ty) { this->Ty = Ty; }

  void createVarDecl(std::string Name, const QualType *Ty, bool mut) {
    VarDecls[Name] = VarDecl{Name, Ty, mut};
  }

  bool containVarDecl(const std::string &Name) const {
    return VarDecls.find(Name) != VarDecls.end();
  }

  const VarDecl &getVarDecl(const std::string &Name) const {
    auto it = VarDecls.find(Name);
    if (it == VarDecls.end()) {
      throw std::runtime_error("Variable declaration not found: " + Name);
    }
    return it->second;
  }

  bool getVarMut(const std::string &Name) const {
    auto it = getVarDecl(Name);
    return it.mut;
  }

  const std::unordered_map<std::string, VarDecl> &getVarDecls() const {
    return VarDecls;
  }
};

#endif