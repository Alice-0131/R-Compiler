#ifndef ITEMFN_HPP
#define ITEMFN_HPP
#include "ItemNode.hpp"
class TypeNode;
class PatternNode;
class ExprNode;
class ExprBlock;

struct ShorthandSelf
{
  bool is_and;
  bool is_mut;
};

struct TypedSelf
{
  bool is_mut;
  std::unique_ptr<TypeNode> type;
};

struct SelfParam
{
  bool flag; // true: ShorthandSelf; false: TypedSelf
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
  std::unique_ptr<ExprNode> block_expr;
public:
  ItemFn(bool is_const, std::string identifier, FnParameters &&function_parameters, 
    std::unique_ptr<TypeNode> function_return_type, std::unique_ptr<ExprNode> block_expr): 
    is_const(is_const), identifier(identifier), function_parameters(std::move(function_parameters)), 
    function_return_type(std::move(function_return_type)), block_expr(std::move(block_expr)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif