#ifndef SYMBOL_HPP
#define SYMBOL_HPP
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "../Parser/parser.hpp"

enum SymbolType
{
  VARIABLE_,
  FUNCTION_,
  STRUCT_,
  ENUM_,
  TRAIT_,
  CONST_
};

class Symbol
{
public:
  std::string name;
  SymbolType type;

  Symbol(std::string name, SymbolType type): name(name), type(type){};
  virtual ~Symbol() = 0;
};

class SymbolVariable: public Symbol
{
public:
  std::shared_ptr<TypeNode> type;
  bool is_mut;

  SymbolVariable(std::string name, std::shared_ptr<TypeNode> type, bool is_mut):
    Symbol(name, VARIABLE_), type(type), is_mut(is_mut) {}
};

class SymbolFunction: public Symbol
{
public:
  FnParameters function_parameters;
  std::shared_ptr<TypeNode> function_return_type;

  SymbolFunction(std::string name, FnParameters &function_parameters,
    std::shared_ptr<TypeNode> function_return_type): Symbol(name, FUNCTION_),
    function_parameters(function_parameters), function_return_type(function_return_type) {}
};

class SymbolStruct: public Symbol
{
public:
  std::vector<StructField> struct_fields;

  SymbolStruct(std::string name, std::vector<StructField> struct_fields):
    Symbol(name, STRUCT_), struct_fields(struct_fields) {}
};

class SymbolEnum: public Symbol
{
public:
  std::vector<std::string> enum_variants;

  SymbolEnum(std::string name, std::vector<std::string> &enum_variants): 
    Symbol(name, ENUM_), enum_variants(enum_variants) {}
};

class SymbolTrait: public Symbol
{
public:
  std::vector<std::shared_ptr<ItemAssociatedNode>> associated_items;
  SymbolTrait(std::string name, std::vector<std::shared_ptr<ItemAssociatedNode>> &associated_items):
    Symbol(name, TRAIT_), associated_items(associated_items){}
};

class SymbolConst: public Symbol
{
public:
  std::shared_ptr<TypeNode> type;
  std::shared_ptr<ExprNode> expr;

  SymbolConst(std::string name, std::shared_ptr<TypeNode> type, std::shared_ptr<ExprNode> expr):
    Symbol(name, CONST_), type(type), expr(expr){}
};

enum ScopeType
{
  Global,
  Block,
  Function,
  Trait,
  Impl,
  Loop
};

class Scope 
{
public:
  std::shared_ptr<Scope> parent;
  std::vector<std::shared_ptr<Scope>> children;
  std::unordered_map<std::string, std::shared_ptr<Symbol>> type_symbols;
  std::unordered_map<std::string, std::shared_ptr<Symbol>> value_symbols;
  ScopeType type;

  Scope(std::shared_ptr<Scope> parent, ScopeType type): parent(parent), type(type) {}
  bool add_type_symbol(std::shared_ptr<Symbol>);
  bool add_value_symbol(std::shared_ptr<Symbol>);
  std::shared_ptr<Symbol> find_type_symbol(std::string &name);
  std::shared_ptr<Symbol> find_value_symbol(std::string &name);
};

class SymbolCollector: public ASTVisitor
{
private:
  std::shared_ptr<Scope> global_scope;
  std::shared_ptr<Scope> cur_scope;

  void add_scope(ScopeType type);
  void exit_scope();

public:
  SymbolCollector() {
    global_scope = std::make_shared<Scope>(nullptr, Global);
    cur_scope = global_scope;
  }
  std::shared_ptr<Scope> get_global_scope() { return global_scope; }
  void collect(std::shared_ptr<Crate> crate){crate->accept(*this);}

  void visit(Crate &node) override;
  void visit(Path &node) override {}

  void visit(ItemFn &node) override;
  void visit(ItemStruct &node) override;
  void visit(ItemEnum &node) override;
  void visit(ItemConst &node) override;
  void visit(ItemTrait &node) override;
  void visit(ItemImpl &node) override;

  void visit(StmtEmpty &node) override {}
  void visit(StmtItem &node) override;
  void visit(StmtLet &node) override;
  void visit(StmtExpr &node) override;

  void visit(ExprLiteralChar &node) override {}
  void visit(ExprLiteralString &node) override {}
  void visit(ExprLiteralInt &node) override {}
  void visit(ExprLiteralBool &node) override {}
  void visit(ExprPath &node) override{}
  void visit(ExprBlock &node) override;
  void visit(ExprOpUnary &node) override;
  void visit(ExprOpBinary &node) override;
  void visit(ExprOpCast &node) override;
  void visit(ExprGrouped &node) override;
  void visit(ExprArrayExpand &node) override;
  void visit(ExprArrayAbbreviate &node) override;
  void visit(ExprIndex &node) override;
  void visit(ExprStruct &node) override;
  void visit(ExprCall &node) override;
  void visit(ExprMethodCall &node) override;
  void visit(ExprField &node) override;
  void visit(ExprLoopInfinite &node) override;
  void visit(ExprLoopPredicate &node) override;
  void visit(ExprBreak &node) override;
  void visit(ExprContinue &node) override {}
  void visit(ExprIf &node) override;
  void visit(ExprReturn &node) override;

  void visit(PatternIdentifier &node) override {}
  void visit(PatternReference &node) override {}

  void visit(TypePath &node) override {}
  void visit(TypeReference &node) override;
  void visit(TypeArray &node) override;
  void visit(TypeUnit &node) override {}
};

#endif