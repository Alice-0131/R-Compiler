#ifndef CONST_SOLVER_H
#define CONST_SOLVER_H

#include "../ASTNode/ExprGrouped.hpp"
#include "../ASTNode/ExprLiteral.hpp"
#include "../ASTNode/ExprNode.hpp"
#include "../ASTNode/ExprOperator.hpp"
#include "../ASTNode/ExprPath.hpp"
#include "../ASTNode/ExprStruct.hpp"
#include "../ASTNode/ItemConst.hpp"
#include "../ASTNode/ItemFn.hpp"
#include "../ASTNode/Path.hpp"
#include "../ASTNode/StmtExpr.hpp"
#include "../ASTNode/StmtLet.hpp"
#include "../ASTNode/TypeArray.hpp"
#include "../ASTNode/TypePath.hpp"
#include "SymTable.hpp"
#include "Type.hpp"
#include <list>
#include <string>
#include <vector>

class ConstSolver;

struct Result { // record <type, isConst, value>
  const QualType *Ty;
  bool flag;
  long value;

  Result(const QualType *Ty = QualType::getVoidType(), bool flag = false,
         int value = 0) : Ty(Ty), flag(flag), value(value) {}
  const QualType *getTy() { return Ty; }
  bool isConst() { return flag; }
  int getValue() { return value; }
  bool empty() { return Ty == QualType::getVoidType() && flag; }
};

class PrioriKnowledge { // record known const
  friend ConstSolver;

private:
  std::vector<ConstTable *> knowledge;

  bool count(std::string &Name) { return getValue(Name).empty(); }

  Result getValue(std::string &Name) {
    for (auto it = knowledge.rbegin(); it != knowledge.rend(); ++it) {
      auto table = *it;
      if (table->count(Name))
        return Result(table->getTy(Name), true, table->getValue(Name));
    }
    return Result();
  }

public:
  void insert(ConstTable &table) { knowledge.push_back(&table); }
};

class Question { // record questions to solve
  friend ConstSolver;

private:
  std::list<ItemConst *> items;
  std::list<ExprNode *> exprs;

  bool empty() { return items.empty() && exprs.empty(); }

  size_t itemLen() { return items.size(); }

  size_t exprLen() { return items.size(); }

public:
  void insert(ItemConst &item) { items.push_back(&item); }
  void insert(ExprNode &expr) { exprs.push_back(&expr); }
};

class Solution {
  friend ConstSolver;

private:
  using ItemSolution = std::pair<ItemConst *, Result>;
  using ExprSolution = std::pair<ExprNode *, Result>;

  std::vector<ItemSolution> itemSolutions;
  std::vector<ExprSolution> exprSolutions;
  std::unordered_map<std::string, Result> valueMap;

  void insert(ItemConst *item, Result result) {
    itemSolutions.push_back(std::make_pair(item, result));
  }

  void insert(ExprNode *expr, Result result) {
    exprSolutions.push_back(std::make_pair(expr, result));
  }

  bool count(std::string &name) { return valueMap.count(name); }

  Result getValue(std::string &name) { return valueMap[name]; }

  void setValue(std::string &name, Result result) { valueMap[name] = result; }
};

class ConstSolver {
public:
  PrioriKnowledge prioriKnowledge;
  Question question;
  Solution solution;

private:
  bool Pass;

  const QualType *getTy(TypeNode &N) {
    if (N.getTypeID() == ASTNode::K_TypePath) {
      auto &typePath = static_cast<TypePath &>(N);
      return getPathType(typePath);
    }
    return QualType::getVoidType();
  }

  const QualType *getPathType(TypePath &N) {
    if (N.path->type != PathType::Identifier) {
      return QualType::getVoidType();
    }
    const std::string &name = N.path->identifier;
    if (name == "bool")  return QualType::getBoolType();
    if (name == "i32")   return QualType::getI32Type();
    if (name == "u32")   return QualType::getU32Type();
    if (name == "usize") return QualType::getUsizeType();
    if (name == "isize") return QualType::getIsizeType();
    return QualType::getVoidType();
  }

  Result getValue(std::string &Name) {
    if (solution.count(Name)) {
      return solution.getValue(Name);
    }
    return prioriKnowledge.getValue(Name);
  }

private:
  void collectItem() {
    for (auto item : question.items) {
      std::string &name = item->identifier;
      const QualType *Ty = getTy(*item->type);
      solution.setValue(name, Result(Ty, false, 0));
    }
  }

  bool checkItem(ItemConst *item) {
    std::string &constName = item->identifier;
    auto &expr = item->expr;
    auto Ty = solution.getValue(constName).getTy();
    auto result = checkExpr(*expr);
    if (!Ty->equals(result.getTy())) {
      Pass = false;
    } else if (result.isConst()) {
      Result r = Result(Ty, true, result.getValue());
      solution.setValue(constName, r);
      solution.insert(item, r);
      return true;
    }
    return false;
  }

  Result checkExpr(ExprNode &N) {
    switch (N.getTypeID()) {
    default:
      Pass = false;
      return Result();
    case ASTNode::K_ExprGrouped:return checkExprGrouped(*dynamic_cast<ExprGrouped *>(&N));
    case ASTNode::K_ExprLiteralBool:return checkExprLiteralBool(*dynamic_cast<ExprLiteralBool *>(&N));
    case ASTNode::K_ExprLiteralChar:return checkExprLiteralChar(*dynamic_cast<ExprLiteralChar *>(&N));
    case ASTNode::K_ExprLiteralInt:return checkExprLiteralInt(*dynamic_cast<ExprLiteralInt *>(&N));
    case ASTNode::K_ExprLiteralString:return checkExprLiteralString(*dynamic_cast<ExprLiteralString *>(&N));
    case ASTNode::K_ExprOpBinary:return checkExprOpBinary(*dynamic_cast<ExprOpBinary *>(&N));
    case ASTNode::K_ExprOpUnary:return checkExprOpUnary(*dynamic_cast<ExprOpUnary *>(&N));
    case ASTNode::K_ExprPath:return checkExprPath(*dynamic_cast<ExprPath *>(&N));
    }
  }

  Result checkExprGrouped(ExprGrouped &E) { return checkExpr(*E.expr); }

  Result checkExprLiteralBool(ExprLiteralBool &E) {
    return Result(QualType::getBoolType(), true, E.literal);
  }

  Result checkExprLiteralChar(ExprLiteralChar &E) {
    return Result(QualType::getCharType(), true, E.literal);
  }

  Result checkExprLiteralInt(ExprLiteralInt &E) {
    return Result(IntLiteralQualType::create(E.literal), true, E.literal);
  }

  Result checkExprLiteralString(ExprLiteralString &E) {
    return Result();
  }

  Result checkExprOpBinary(ExprOpBinary &E) {
    auto result1 = checkExpr(*E.left);
    auto result2 = checkExpr(*E.right);
    if (result1.getTy()->equals(result2.getTy())) {
      return Result(
          result1.getTy(), result1.isConst() && result2.isConst(),
          getBopValue(result1.getValue(), result2.getValue(), E.type));
    }else {
      return Result();
    }
  }

  Result checkExprOpUnary(ExprOpUnary &E) {
    auto result = checkExpr(*E.expr);
    bool flag = isBoolOp(E.type);
    if (result.getTy()->isBool() && flag) {
      return Result(result.getTy(), result.isConst(), !result.getValue());
    }
    if (result.getTy()->equals(IntLiteralQualType::create(0)) && !flag) {
      return Result(result.getTy(), result.isConst(), -result.getValue());
    }
    return Result();
  }

  Result checkExprPath(ExprPath &E) {
    auto &path = *E.path1;
    if (E.path2 || path.type != PathType::Identifier) {
      return Result();
    }
    return getValue(path.identifier);
  }

  long getBopValue(int left, int right, ExprOpBinaryType ty) {
    switch (ty) {
    default:
      throw std::runtime_error("Invalid binary operator in const expr");
    case PLUS_:
      return left + right;
    case MINUS_:
      return left - right;
    case MUL_:
      return left * right;
    case DIV_:
      return left / right;
    }
  }

  bool isBoolOp(ExprOpUnaryType ty) {
    switch (ty) {
    case BORROW_:
    case MUT_BORROW_:
    case DEREFERENCE_:
      throw std::runtime_error("Invalid unary operator in const expr");
    case NEGATE_:
      return false;
    case NOT_:
      return true;
    }
    return false;
  }

  void solveItem() {
    bool Fixed = false;
    collectItem();
    // fixed point
    while (!Fixed) {
      size_t len = question.itemLen();
      for (auto it = question.items.begin(); it != question.items.end(); ) {
        if (checkItem(*it)) {
          it = question.items.erase(it); // checkItem 成功就移除
        } else {
          ++it;
        }
      }
      Fixed = len == question.itemLen();
    }
  }

  void solveExpr() {
    bool Fixed = false;
    // fixed point
    while (!Fixed) {
      size_t len = question.exprLen();
      for (auto it = question.exprs.begin(); it != question.exprs.end(); ) {
        Result result = checkExpr(**it);
        if (result.isConst()) {
          solution.insert(*it, result);
          it = question.exprs.erase(it); // 求值成功就移除
        } else {
          ++it;
        }
      }
      Fixed = len == question.exprLen();
    }
  }

public:
  ConstSolver() : Pass(true) {}

  bool solve() {
    solveItem();
    solveExpr();
    return Pass && question.empty();
  }
};

#endif