#ifndef ITEMFN_HPP
#define ITEMFN_HPP
#include "ItemNode.hpp"
class TypeNode;
class PatternNode;
class ExprNode;
class ExprBlock;

struct ShorthandSelf
{
  bool is_and = false;
  bool is_mut = false;
};

struct TypedSelf
{
  bool is_mut = false;
  std::unique_ptr<TypeNode> type;
};

struct SelfParam
{
  int flag = 0; // 0: none; 1: ShorthandSelf; 2: TypedSelf
  ShorthandSelf shorthand_self;
  TypedSelf typed_self;
};

struct FnParam
{
  std::unique_ptr<PatternNode> pattern;
  std::unique_ptr<TypeNode> type;
};

struct FnParameters
{
  SelfParam self_param;
  std::vector<FnParam> fn_params;
};

class ItemFn : public ItemAssociatedNode
{
private:
  bool is_const;
  std::string identifier;
  FnParameters function_parameters;
  std::unique_ptr<TypeNode> function_return_type;
  std::unique_ptr<ExprBlock> block_expr;
public:
  ItemFn(bool is_const, std::string identifier, FnParameters &&function_parameters, 
    std::unique_ptr<TypeNode> function_return_type, std::unique_ptr<ExprBlock> block_expr): 
    is_const(is_const), identifier(identifier), function_parameters(std::move(function_parameters)), 
    function_return_type(std::move(function_return_type)), block_expr(std::move(block_expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif