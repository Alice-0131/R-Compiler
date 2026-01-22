#ifndef CODEGEN_HPP
#define CODEGEN_HPP

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
#include "../ASTNode/PatternNode.hpp"
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
#include <cassert>
#include <cstdint> // fix missing uint64_t
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class CodeGen {
private:
  const Crate &Prog;
  const SymTable &Syms;

  llvm::LLVMContext &Context;
  llvm::Module &Module;
  llvm::IRBuilder<> Builder;
  llvm::BasicBlock *Exit;
  llvm::Value *ReturnValue;

private:
  std::unordered_map<std::string, llvm::StructType *> StructTyDef;
  std::unordered_map<std::string, llvm::Value *> AllocaAddr;

  const StructQualType *CurrentImpl = nullptr;
  ItemFn *CurrentFn = nullptr;
  llvm::BasicBlock *CurrentHeadBB = nullptr;
  llvm::BasicBlock *CurrentAfterBB = nullptr;
  llvm::Value *CurrentLoopRes = nullptr;

  llvm::Type *ImplType;

public:
  CodeGen(const Crate &Prog, const SymTable &Syms, llvm::LLVMContext &Context,
          llvm::Module &Module);

  ~CodeGen() = default;
  bool emit();

private:
  std::vector<llvm::Type *> getFnParamTypes(const FnParameters &FnParams);

  llvm::Type *convertType(const QualType *Ty);

  void emitItemNode(const ItemNode &N);
  llvm::Value *emitExprNode(const ExprNode &N);
  llvm::Value *getAssign(const ExprNode &N);
  llvm::Type *emitTypeNode(const TypeNode &N);
  void emitPatternNode(const PatternNode &N);
  llvm::Value *getValue(llvm::Value *value, const QualType *Ty);

  void createMemCpy(llvm::Value *Dest, llvm::Value *Src, llvm::Type *Ty);

  llvm::AllocaInst *createAlloca(llvm::Type *Ty, llvm::Value *ArraySize = nullptr,
                           const llvm::Twine &Name = "");
  // Value *emitExprWithoutBlockNode(const ExprWithoutBlockNode &N);
  // Value *emitExprWithBlockNode(const ExprWithBlockNode &N);
  // Value *emitExprArrayNode(const ExprArrayNode &N);

  // Value *emitExprLiteralNode(const ExprLiteralNode &N);

  // Value *emitExprOperatorNode(const ExprOperatorNode &N);
  void emitStmtNode(const StmtNode &N);

  void emitStructDefination();

  std::string mangleFnName(std::string StructName, std::string FnName);

  std::string extractManglePathIdentifier(const ExprPath &N);

  llvm::Value *getDerefValue(llvm::Value *Val, const QualType *Ty);
  const QualType *getDerefedQualType(const QualType *Ty);

  void emitFunctionDefination();

  void emitCrate(const Crate &N);
  void emitPath(const Path &N);

  void emitItemFn(const ItemFn &N);
  void emitItemImpl(const ItemImpl &N);

  void emitStmtEmpty(const StmtEmpty &N);
  void emitStmtItem(const StmtItem &N);
  void emitStmtLet(const StmtLet &N);
  void emitStmtExpr(const StmtExpr &N);
  void emitFnParam(const FnParameters &FnParams, const FuncQualType *FnType);

  llvm::Value *emitExprLiteralChar(const ExprLiteralChar &N);
  llvm::Value *emitExprLiteralString(const ExprLiteralString &N);
  llvm::Value *emitExprLiteralInt(const ExprLiteralInt &N);
  llvm::Value *emitExprLiteralBool(const ExprLiteralBool &N);
  llvm::Value *emitExprPath(const ExprPath &N);
  llvm::Value *emitExprBlock(const ExprBlock &N);
  llvm::Value *emitExprOpUnary(const ExprOpUnary &N);
  llvm::Value *emitExprOpBinary(const ExprOpBinary &N);
  llvm::Value *emitExprOpCast(const ExprOpCast &N);
  llvm::Value *emitExprGrouped(const ExprGrouped &N);
  llvm::Value *emitExprArrayExpand(const ExprArrayExpand &N);
  llvm::Value *emitExprArrayAbbreviate(const ExprArrayAbbreviate &N);
  llvm::Value *emitExprIndex(const ExprIndex &N);
  llvm::Value *emitExprStruct(const ExprStruct &N);
  llvm::Value *emitExprCall(const ExprCall &N);
  llvm::Value *emitExprMethodCall(const ExprMethodCall &N);
  llvm::Value *emitExprField(const ExprField &N);
  llvm::Value *emitExprLoopInfinite(const ExprLoopInfinite &N);
  llvm::Value *emitExprLoopPredicate(const ExprLoopPredicate &N);
  llvm::Value *emitExprBreak(const ExprBreak &N);
  llvm::Value *emitExprContinue(const ExprContinue &N);
  llvm::Value *emitExprIf(const ExprIf &N);
  llvm::Value *emitExprReturn(const ExprReturn &N);
  llvm::Value *emitExprUnderscore(const ExprUnderscore &N);

  void emitPatternIdentifier(const PatternIdentifier &N);
  void emitPatternReference(const PatternReference &N);

  llvm::Type *getType(const TypeNode *N);

  llvm::Type *getPathType(const TypePath &N);
  llvm::Type *getReferenceType(const TypeReference &N);
  llvm::Type *getArrayType(const TypeArray &N);
  llvm::Type *getUnitType(const TypeUnit &N);
};

#endif