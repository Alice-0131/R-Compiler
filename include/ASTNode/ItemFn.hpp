#ifndef ITEMFN_HPP
#define ITEMFN_HPP
#include "ItemNode.hpp"
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

class ItemFn : public ItemAssociatedNode
{
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
};

#endif