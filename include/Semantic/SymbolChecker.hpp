#ifndef SYMBOLCHECKER_HPP
#define SYMBOLCHECKER_HPP

#include "../ASTNode/Crate.hpp"
#include "../ASTNode/ExprArrayIndex.hpp"
#include "../ASTNode/ExprBlock.hpp"
#include "../ASTNode/ExprCall.hpp"
#include "../ASTNode/ExprField.hpp"
#include "../ASTNode/ExprGrouped.hpp"
#include "../ASTNode/ExprIf.hpp"
#include "../ASTNode/ExprLiteral.hpp"
#include "../ASTNode/ExprLoop.hpp"
#include "../ASTNode/ExprMethodCall.hpp"
#include "../ASTNode/ExprNode.hpp"
#include "../ASTNode/ExprOperator.hpp"
#include "../ASTNode/ExprPath.hpp"
#include "../ASTNode/ExprReturn.hpp"
#include "../ASTNode/ExprStruct.hpp"
#include "../ASTNode/ItemConst.hpp"
#include "../ASTNode/ItemEnum.hpp"
#include "../ASTNode/ItemFn.hpp"
#include "../ASTNode/ItemImpl.hpp"
#include "../ASTNode/ItemNode.hpp"
#include "../ASTNode/ItemStruct.hpp"
#include "../ASTNode/ItemTrait.hpp"
#include "../ASTNode/Path.hpp"
#include "../ASTNode/PatternIdentifier.hpp"
#include "../ASTNode/PatternReference.hpp"
#include "../ASTNode/StmtEmpty.hpp"
#include "../ASTNode/StmtExpr.hpp"
#include "../ASTNode/StmtItem.hpp"
#include "../ASTNode/StmtLet.hpp"
#include "../ASTNode/StmtNode.hpp"
#include "../ASTNode/TypeArray.hpp"
#include "../ASTNode/TypeNode.hpp"
#include "../ASTNode/TypePath.hpp"
#include "../ASTNode/TypeReference.hpp"
#include "../ASTNode/TypeUnit.hpp"
#include "../Semantic/SymTable.hpp"
#include "../Semantic/Type.hpp"
#include <memory>
#include <unordered_set>
#include <vector>
#include <stdexcept>

class BlockCtx {
public:
  enum BlockType { Func, Loop };

  struct BlockInfo {
    BlockType Type;
    const QualType *RetType;

    void setReturnType(const QualType *Ty) { RetType = Ty; }

    const QualType *getReturnType() const { return RetType; }
  };

private:
  std::vector<BlockInfo> Scopes;

public:
  bool inScope(BlockType type) const {
    for (auto it = Scopes.rbegin(); it != Scopes.rend(); ++it) {
      if (it->Type == type) {
        return true;
      }
    }
    return false;
  }

BlockInfo &getLastScope(BlockType type) {
    for (auto it = Scopes.rbegin(); it != Scopes.rend(); ++it) {
      if (it->Type == type) {
        return *it;
      }
    }
    throw std::runtime_error("scope not found");
}

  BlockInfo &getLastScope() { return Scopes[Scopes.size() - 1]; }

  size_t getScopeSize() const { return Scopes.size(); }

  void enterScope(BlockType Type) { Scopes.push_back({Type, nullptr}); }

  void exitScope() {
    if (Scopes.empty()) {
      throw std::runtime_error("No scope to exit");
    }
    Scopes.pop_back();
  }
};

class Checker {
private:
  struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const {
      auto h1 = std::hash<T1>{}(p.first);
      auto h2 = std::hash<T2>{}(p.second);
      return h1 ^ (h2 << 1);
    }
  };

private:
  BlockCtx BCtx;

private:
  std::shared_ptr<Crate> &Prog;
  SymTable &Syms;

  QualType *CurImplTy;// The structure currently being processed
  ItemFn *CurFunction = nullptr;// The function currently being processed
  bool islocal = false;// Is the current scope local
  LocalScope *scopes = nullptr;

public:
  Checker(std::shared_ptr<Crate> &Prog, SymTable &Syms);

  // return checked programs with type info
  void check();

private:
  void initRun();// move nested items (except const) to global
  void firstRun();// collect define of data struct (struct, const, enum, trait)
  void secondRun();// calculate the value of const items in global
  void thirdRun();// collect field of struct & signature of fn
  void forthRun();// check impl trait of struct & trait
  void fifthRun();// check each fn & impl in detail

private:
  void removeItem(std::unordered_set<ItemNode *> &remove);
  // specific the type of self
  // self of trait has no exact type
  const FuncQualType *setFnSignature(ItemFn &N, bool isImp);

  void collectConst(ItemConst &N);
  void collectStructField(ItemStruct &N);
  void collectStructMethod(ItemImpl &N);
  void collectTraitMethod(ItemTrait &N);
  void collectFunction(ItemFn &N);

  void checkItemNode(ItemNode &N);
  const QualType *checkExprNode(ExprNode &N);
  const QualType *checkTypeNode(TypeNode &N);
  const QualType *checkPatternNode(PatternNode &N);

  void checkStmtNode(StmtNode &N);

  const QualType *checkPath(Path &N);

  void checkItemFn(ItemFn &N);
  // void checkItemStruct(ItemStruct &N);
  void checkItemEnum(ItemEnum &N);
  void checkItemConst(ItemConst &N);
  void checkItemImpl(ItemImpl &N);

  std::vector<const QualType *> checkFnParameters(FnParameters &N);

  void checkStmtEmpty(StmtEmpty &N);
  void checkStmtItem(StmtItem &N);
  void checkStmtLet(StmtLet &N);
  void checkStmtExpr(StmtExpr &N);

  const QualType *checkExprLiteralChar(ExprLiteralChar &N);
  const QualType *checkExprLiteralString(ExprLiteralString &N);
  const QualType *checkExprLiteralInt(ExprLiteralInt &N);
  const QualType *checkExprLiteralBool(ExprLiteralBool &N);
  const QualType *checkExprPath(ExprPath &N);
  const QualType *checkExprBlock(ExprBlock &N);
  const QualType *checkExprOpUnary(ExprOpUnary &N);
  const QualType *checkExprOpBinary(ExprOpBinary &N);
  const QualType *checkExprOpCast(ExprOpCast &N);
  const QualType *checkExprGrouped(ExprGrouped &N);
  const QualType *checkExprArrayExpand(ExprArrayExpand &N);
  const QualType *checkExprArrayAbbreviate(ExprArrayAbbreviate &N);
  const QualType *checkExprIndex(ExprIndex &N);
  const QualType *checkExprStruct(ExprStruct &N);
  const QualType *checkExprCall(ExprCall &N);
  const QualType *checkExprMethodCall(ExprMethodCall &N);
  const QualType *checkExprField(ExprField &N);
  const QualType *checkExprLoopInfinite(ExprLoopInfinite &N);
  const QualType *checkExprLoopPredicate(ExprLoopPredicate &N);
  const QualType *checkExprBreak(ExprBreak &N);
  const QualType *checkExprContinue(ExprContinue &N);
  const QualType *checkExprIf(ExprIf &N);
  const QualType *checkExprReturn(ExprReturn &N);

  const QualType *checkPatternIdentifier(PatternIdentifier &N);
  const QualType *checkPatternReference(PatternReference &N);

  const QualType *getType(TypeNode &N);

  const QualType *getPathType(TypePath &N);
  const QualType *getReferenceType(TypeReference &N);
  const QualType *getArrayType(TypeArray &N);
  const QualType *getUnitType(TypeUnit &N);

  long evaluateExprNode(ExprNode &N);
};

#endif // SYMBOLCHECKER_HPP