#include "../../include/Semantic/SymbolCollector.hpp"
#include <memory>
#include <stdexcept>

Symbol::~Symbol() {}

bool Scope::add_type_symbol(std::shared_ptr<Symbol> symbol) {
  if (type_symbols.count(symbol->name)) {
    return false;
  }
  type_symbols[symbol->name] = symbol;
  return true;
}

bool Scope::add_value_symbol(std::shared_ptr<Symbol> symbol) {
  if (value_symbols.count(symbol->name)) {
    return false;
  }
  value_symbols[symbol->name] = symbol;
  return true;
}

std::shared_ptr<Symbol> Scope::find_type_symbol(std::string &name) {
  if (type_symbols.find(name) != type_symbols.end()) {
    return type_symbols[name];
  }
  // if (type == Function) {
  //   return nullptr;
  // }
  if (parent) {
    return parent->find_type_symbol(name);
  }
  return nullptr;
}

std::shared_ptr<Symbol> Scope::find_value_symbol(std::string &name) {
  if (value_symbols.find(name) != value_symbols.end()) {
    return value_symbols[name];
  }
  // if (type == Function) {
  //   return nullptr;
  // }
  if (parent) {
    return parent->find_value_symbol(name);
  }
  return nullptr;
}

void SymbolCollector::add_scope(ScopeType type) {
  auto child = std::make_shared<Scope>(cur_scope, type);
  cur_scope->children.push_back(child);
  cur_scope = child;
}

void SymbolCollector::exit_scope() {
  if (cur_scope) {
    cur_scope = cur_scope->parent;
  } else {
    throw std::runtime_error("SymbolCollector: no scope left.");
  }
}

void SymbolCollector::visit(Crate &node) {
  for (auto &item : node.children) {
    item->accept(*this);
  }
}

void SymbolCollector::visit(ItemFn &node) {
  auto symbol_function = std::make_shared<SymbolFunction>(node.identifier,
     node.function_parameters, node.function_return_type ? node.function_return_type : std::make_shared<TypeUnit>());
  if (!cur_scope->add_value_symbol(symbol_function)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
  add_scope(Function);
  for (auto stmt : node.block_expr->stmts) {
    stmt->accept(*this);
  }
  node.block_expr->expr->accept(*this);
  exit_scope();
}

void SymbolCollector::visit(ItemStruct &node) {
  auto symbol_struct = std::make_shared<SymbolStruct>(node.identifier, node.struct_fields);
  if (!cur_scope->add_type_symbol(symbol_struct)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
}

void SymbolCollector::visit(ItemEnum &node) {
  auto symbol_enum = std::make_shared<SymbolEnum>(node.identifier, node.enum_variants);
  if (!cur_scope->add_type_symbol(symbol_enum)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
}

void SymbolCollector::visit(ItemConst &node) {
  auto symbol_const = std::make_shared<SymbolConst>(node.identifier, node.type, node.expr);
  if (!cur_scope->add_value_symbol(symbol_const)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
}

void SymbolCollector::visit(ItemTrait &node) {
  auto symbol_trait = std::make_shared<SymbolTrait>(node.identifier, node.associated_items);
  if (!cur_scope->add_type_symbol(symbol_trait)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
  add_scope(Trait);
  for (auto item : node.associated_items) {
    if (dynamic_cast<ItemFn*>(item.get())) {
      item->accept(*this); 
    }
  }
  exit_scope();
}

void SymbolCollector::visit(ItemImpl &node) {
  add_scope(Impl);
  for (auto item : node.associated_items) {
    if (dynamic_cast<ItemFn*>(item.get())) {
      item->accept(*this); 
    }
  }
  exit_scope();
}

void SymbolCollector::visit(StmtItem &node) {
  node.item->accept(*this);
}

void SymbolCollector::visit(StmtLet &node) {
  std::string identifier;
  bool is_mut;
  if (auto id = dynamic_cast<PatternIdentifier*>(node.pattern.get())) {
    identifier = id->identifier;
    is_mut = id->is_mut;
  } else {//TODO(?)
    throw std::runtime_error("SymbolCollector: I don't know how to deal with it.");
  }
  auto symbol_variable = std::make_shared<SymbolVariable>(identifier, node.type, is_mut);
  if (!cur_scope->add_value_symbol(symbol_variable)) {
    throw std::runtime_error("SymbolCollector: already exist.");
  }
  node.expr->accept(*this);
}

void SymbolCollector::visit(StmtExpr &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(ExprBlock &node) {
  add_scope(Block);
  for (auto stmt : node.stmts) {
    stmt->accept(*this);
  }
  node.expr->accept(*this);
  exit_scope();
}

void SymbolCollector::visit(ExprOpUnary &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(ExprOpBinary &node) {
  node.left->accept(*this);
  node.right->accept(*this);
}

void SymbolCollector::visit(ExprOpCast &node) {
  node.expr->accept(*this);
  node.type->accept(*this);
}

void SymbolCollector::visit(ExprGrouped &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(ExprArrayExpand &node) {
  for (auto expr : node.elements) {
    expr->accept(*this);
  }
}

void SymbolCollector::visit(ExprArrayAbbreviate &node) {
  node.size->accept(*this);
  node.value->accept(*this);
}

void SymbolCollector::visit(ExprIndex &node) {
  node.index->accept(*this);
  node.array->accept(*this);
}

void SymbolCollector::visit(ExprStruct &node) {
  for (auto field : node.fields) {
    field.expr->accept(*this);
  }
}

void SymbolCollector::visit(ExprCall &node) {
  node.expr->accept(*this);
  for (auto param : node.params) {
    param->accept(*this);
  }
}

void SymbolCollector::visit(ExprMethodCall &node) {
  node.expr->accept(*this);
  for (auto param : node.params) {
    param->accept(*this);
  }
}

void SymbolCollector::visit(ExprField &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(ExprLoopInfinite &node) {
  add_scope(Loop);
  for (auto stmt : node.block->stmts) {
    stmt->accept(*this);
  }
  node.block->expr->accept(*this);
  exit_scope();
}

void SymbolCollector::visit(ExprLoopPredicate &node) {
  node.condition->accept(*this);
  add_scope(Loop);
  for (auto stmt : node.block->stmts) {
    stmt->accept(*this);
  }
  node.block->expr->accept(*this);
  exit_scope();
}

void SymbolCollector::visit(ExprBreak &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(ExprIf &node) {
  node.condition->accept(*this);
  node.if_block->accept(*this);
  if (node.else_block) {
    node.else_block->accept(*this);
  }
}

void SymbolCollector::visit(ExprReturn &node) {
  node.expr->accept(*this);
}

void SymbolCollector::visit(TypeReference &node) {
  node.type->accept(*this);
}

void SymbolCollector::visit(TypeArray &node) {
  node.type->accept(*this);
  node.expr->accept(*this);
}