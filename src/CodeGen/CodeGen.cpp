#include "../../include/CodeGen/CodeGen.hpp"
#include "../../include/ASTNode/ASTNode.hpp"
#include "../../include/ASTNode/ExprArrayIndex.hpp"
#include "../../include/ASTNode/ExprCall.hpp"
#include "../../include/ASTNode/ExprPath.hpp"
#include "../../include/ASTNode/ItemConst.hpp"
#include "../../include/ASTNode/ItemFn.hpp"
#include "../../include/ASTNode/ItemNode.hpp"
#include "../../include/ASTNode/ItemStruct.hpp"
#include "../../include/ASTNode/Path.hpp"
#include "../../include/Semantic/Type.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <llvm/IR/Verifier.h>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

CodeGen::CodeGen(const Crate &Prog, const SymTable &Syms,
                 llvm::LLVMContext &Context, llvm::Module &Module)
    : Prog(Prog), Syms(Syms), Module(Module), Context(Context),
      Builder(Context) {}

bool CodeGen::emit() {
  emitCrate(Prog);

  if (llvm::verifyModule(Module, &llvm::errs())) {
    throw std::runtime_error("module verification failed");
  }
  return true;
}

void CodeGen::emitCrate(const Crate &N) {
  emitStructDefination();
  emitFunctionDefination();

  for (auto &Item : N.children) {
    emitItemNode(*Item);
  }
}

void CodeGen::emitStructDefination() {
  std::queue<std::pair<std::string, StructQualType *>> q;
  for (auto It : Syms.structTable.getTable()) {
    q.push(It);
  }

  for (unsigned i = 0; i < 5; i++) {
    int len = q.size();
    for (int i = 0; i < len; i++) {
      auto [Name, Ty] = q.front();
      q.pop();

      bool canCreate = true;
      std::vector<llvm::Type*> Types;
      for (auto &F : Ty->getFields()) {
        llvm::Type *FieldTy = convertType(F.Type);
        if (!FieldTy) {
          canCreate = false;
          break;
        }
        Types.emplace_back(FieldTy);
      }
      if (canCreate) {
        StructTyDef[Name] = llvm::StructType::create(Context, Types, Name);
      } else {
        q.push({Name, Ty});
      }
    }
  }
  assert(q.empty() && "failed to create struct defination");
}

std::string CodeGen::mangleFnName(std::string StructName, std::string FnName) {
  return StructName + "::" + FnName;
}

std::string CodeGen::extractManglePathIdentifier(const ExprPath &N) {
  if (!N.path2)
    return N.path1->identifier;
  std::string Prefix = N.path1->identifier == "Self" ? CurrentImpl->getName()
                                                     : N.path1->identifier;
  return Prefix + "::" + N.path2->identifier;
}

void CodeGen::emitFunctionDefination() {
  for (auto [Name, FnTy] : Syms.fnTable.getTable()) {
    std::vector<llvm::Type *> ParamTypes;
    for (auto Ty : FnTy->getParamTypes()) {
      ParamTypes.push_back(convertType(Ty));
    }

    llvm::Type *RetType;
    if (FnTy->getReturnType()->isStruct() || FnTy->getReturnType()->isArray()) {
      RetType = llvm::Type::getVoidTy(Context);
      ParamTypes.push_back(llvm::PointerType::get(Context, 0));
    } else {
      RetType = convertType(FnTy->getReturnType());
    }
    
    llvm::FunctionType *FTy = llvm::FunctionType::get(RetType, ParamTypes, false);
    llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, Name, Module);
  }


  for (auto It : Syms.structTable.getTable()) {
    std::string SName = It.first;
    for (auto [Name, FnTy] : It.second->getMethods()) {
      std::string MangledName = mangleFnName(SName, Name);
      
      std::vector<llvm::Type *> ParamTypes;
      for (auto Ty : FnTy->getParamTypes())
        ParamTypes.push_back(convertType(Ty));

      llvm::Type *RetType;
      if (FnTy->getReturnType()->isStruct() || FnTy->getReturnType()->isArray()) {
        RetType = llvm::Type::getVoidTy(Context);
        ParamTypes.push_back(llvm::PointerType::get(Context, 0));
      } else {
        RetType = convertType(FnTy->getReturnType());
      }
      
      llvm::FunctionType *FTy = llvm::FunctionType::get(RetType, ParamTypes, false);
      llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, MangledName, Module);
    }
  }
}

llvm::Value *CodeGen::getDerefValue(llvm::Value *Val, const QualType *Ty) {
  if (Val == nullptr || !Val->getType()->isPointerTy()) {
    return Val;
  }

  if (Ty->isPointer()) {
    Ty = dynamic_cast<const PointerQualType *>(Ty)->getElemType();
  }

  return Builder.CreateLoad(convertType(Ty), Val);
}

const QualType *CodeGen::getDerefedQualType(const QualType *Ty) {
  assert(Ty != nullptr);
  if (const PointerQualType *PT = dynamic_cast<const PointerQualType*>(Ty)) {
    return PT->getElemType();
  }    
  return Ty;
}

llvm::AllocaInst *CodeGen::createAlloca(llvm::Type *Ty, llvm::Value *ArraySize,
                                  const llvm::Twine &Name) {
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Ty, ArraySize, Name);
}

void CodeGen::createMemCpy(llvm::Value *Dest, llvm::Value *Src, llvm::Type *Ty) {
  llvm::DataLayout DL = Module.getDataLayout();
  llvm::Function *Fn = Module.getFunction("memcpy");
  if (!Fn) {
    llvm::Type *PtrTy = llvm::PointerType::getUnqual(Context);
    llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Context);
    llvm::FunctionType *FTy = llvm::FunctionType::get(PtrTy, {PtrTy, PtrTy, Int32Ty}, false);
    Fn = llvm::Function::Create(FTy, llvm::Function::ExternalLinkage, "memcpy", Module);
  }
  uint64_t SizeInBytes = DL.getTypeAllocSize(Ty);

  std::vector<llvm::Value *> Args = {
      Dest, Src, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), SizeInBytes)};
  Builder.CreateCall(Fn, Args);
}

void CodeGen::emitItemNode(const ItemNode &N) {
  switch (N.getTypeID()) {
  default:
    return;
  case ASTNode::K_ItemFn:
    return emitItemFn(static_cast<const ItemFn&>(N));
  case ASTNode::K_ItemImpl:
    return emitItemImpl(static_cast<const ItemImpl&>(N));
  }
}

llvm::Value *CodeGen::getValue(llvm::Value *value, const QualType *Ty) {
  if (value->getType()->isPointerTy()) {
    return Builder.CreateLoad(convertType(Ty), value);
  }
  return value;
}

llvm::Value *CodeGen::emitExprNode(const ExprNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("not implemented expr node");
  case ASTNode::K_ExprArrayAbbreviate:
    return emitExprArrayAbbreviate(static_cast<const ExprArrayAbbreviate&>(N));
  case ASTNode::K_ExprArrayExpand:
    return emitExprArrayExpand(static_cast<const ExprArrayExpand&>(N));
  case ASTNode::K_ExprBlock:
    return emitExprBlock(static_cast<const ExprBlock&>(N));
  case ASTNode::K_ExprBreak:
    return emitExprBreak(static_cast<const ExprBreak&>(N));
  case ASTNode::K_ExprCall:
    return emitExprCall(static_cast<const ExprCall&>(N));
  case ASTNode::K_ExprContinue:
    return emitExprContinue(static_cast<const ExprContinue&>(N));
  case ASTNode::K_ExprField:
    return emitExprField(static_cast<const ExprField&>(N));
  case ASTNode::K_ExprGrouped:
    return emitExprGrouped(static_cast<const ExprGrouped&>(N));
  case ASTNode::K_ExprIf:
    return emitExprIf(static_cast<const ExprIf&>(N));
  case ASTNode::K_ExprIndex:
    return emitExprIndex(static_cast<const ExprIndex&>(N));
  case ASTNode::K_ExprLiteralBool:
    return emitExprLiteralBool(static_cast<const ExprLiteralBool&>(N));
  case ASTNode::K_ExprLiteralChar:
    return emitExprLiteralChar(static_cast<const ExprLiteralChar&>(N));
  case ASTNode::K_ExprLiteralInt:
    return emitExprLiteralInt(static_cast<const ExprLiteralInt&>(N));
  case ASTNode::K_ExprLiteralString:
    return emitExprLiteralString(static_cast<const ExprLiteralString&>(N));
  case ASTNode::K_ExprLoopInfinite:
    return emitExprLoopInfinite(static_cast<const ExprLoopInfinite&>(N));
  case ASTNode::K_ExprLoopPredicate:
    return emitExprLoopPredicate(static_cast<const ExprLoopPredicate&>(N));
  case ASTNode::K_ExprMethodCall:
    return emitExprMethodCall(static_cast<const ExprMethodCall&>(N));
  case ASTNode::K_ExprNode:
    return emitExprNode(static_cast<const ExprNode&>(N));
  case ASTNode::K_ExprOpBinary:
    return emitExprOpBinary(static_cast<const ExprOpBinary&>(N));
  case ASTNode::K_ExprOpCast:
    return emitExprOpCast(static_cast<const ExprOpCast&>(N));
  case ASTNode::K_ExprOpUnary:
    return emitExprOpUnary(static_cast<const ExprOpUnary&>(N));
  case ASTNode::K_ExprPath:
    return emitExprPath(static_cast<const ExprPath&>(N));
  case ASTNode::K_ExprReturn:
    return emitExprReturn(static_cast<const ExprReturn&>(N));
  case ASTNode::K_ExprStruct:
    return emitExprStruct(static_cast<const ExprStruct&>(N));
  }
  return nullptr;
}

llvm::Type *CodeGen::emitTypeNode(const TypeNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("not expected node.");
  case ASTNode::K_TypeArray:
    return getArrayType(static_cast<const TypeArray&>(N));
  case ASTNode::K_TypePath:
    return getPathType(static_cast<const TypePath&>(N));
  case ASTNode::K_TypeReference:
    return getReferenceType(static_cast<const TypeReference&>(N));
  case ASTNode::K_TypeUnit:
    return getUnitType(static_cast<const TypeUnit&>(N));
  }
}

void CodeGen::emitPatternNode(const PatternNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("not expected node.");
  case ASTNode::K_PatternIdentifier:
    return emitPatternIdentifier(static_cast<const PatternIdentifier&>(N));
  case ASTNode::K_PatternReference:
    return emitPatternReference(static_cast<const PatternReference&>(N));
  }
}

void CodeGen::emitStmtNode(const StmtNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("not expected node.");
  case ASTNode::K_StmtEmpty:
    return emitStmtEmpty(static_cast<const StmtEmpty&>(N));
  case ASTNode::K_StmtItem:
    return emitStmtItem(static_cast<const StmtItem&>(N));
  case ASTNode::K_StmtLet:
    return emitStmtLet(static_cast<const StmtLet&>(N));
  case ASTNode::K_StmtExpr:
    return emitStmtExpr(static_cast<const StmtExpr&>(N));
  }
}

void CodeGen::emitPath(const Path &N) {}

llvm::Type *CodeGen::convertType(const QualType *Ty) {
  switch (Ty->getTypeID()) {
  default:
    throw std::runtime_error("not expected node.");
  case QualType::T_char:
    return llvm::Type::getInt8Ty(Context);
  case QualType::T_bool:
    return llvm::Type::getInt1Ty(Context);
  case QualType::T_enum:
  case QualType::T_intLiteral:
  case QualType::T_i32:
  case QualType::T_u32:
  case QualType::T_isize:
  case QualType::T_usize:
    return llvm::Type::getInt32Ty(Context);
  case QualType::T_string:
    return llvm::PointerType::get(llvm::Type::getInt8Ty(Context), 0);
  case QualType::T_struct: {
    const StructQualType *T = dynamic_cast<const StructQualType*>(Ty);
    return StructTyDef[T->getName()];
  }
  case QualType::T_void:
    return llvm::Type::getVoidTy(Context);
  case QualType::T_ptr: {
    const PointerQualType *P = dynamic_cast<const PointerQualType*>(Ty);
    llvm::Type *ElemType = convertType(P->getElemType());
    return llvm::PointerType::get(ElemType, 0);
  }
  case QualType::T_array: {
    const ArrayQualType *A = dynamic_cast<const ArrayQualType*>(Ty);
    llvm::Type *ElemType = convertType(A->getElemType());
    if (ElemType == nullptr) {
      return nullptr;
    }
    return llvm::ArrayType::get(ElemType, A->getLength());
  }
  }
}

std::vector<llvm::Type *> CodeGen::getFnParamTypes(const FnParameters &FnParams) {
  std::vector<llvm::Type *> Types;

  if (FnParams.self_param.flag) {
    assert(ImplType != nullptr);
    Types.emplace_back(llvm::PointerType::getUnqual(ImplType));
  }

  for (const FnParam &I : FnParams.fn_params) {
    Types.emplace_back(getType(I.type.get()));
  }
  return Types;
}

void CodeGen::emitFnParam(const FnParameters &FnParams,
                          const FuncQualType *FnType) {
  llvm::Function *Fn = Builder.GetInsertBlock()->getParent();
  std::vector<const QualType *> paramTypes = FnType->getParamTypes();
  std::vector<std::string> paramNames;

  if (FnParams.self_param.flag) {
    paramNames.push_back("self");
  }

  for (const FnParam &I : FnParams.fn_params) {
    PatternIdentifier *Iden = dynamic_cast<PatternIdentifier*>(I.pattern.get());
    assert(Iden != nullptr);
    paramNames.push_back(Iden->identifier);
  }

  assert(paramNames.size() == paramTypes.size());
  for (size_t Idx = 0; Idx < paramNames.size(); Idx++) {
    llvm::Type *Ty = convertType(paramTypes[Idx]);
    std::string &Name = paramNames[Idx];
    llvm::AllocaInst *Alloca = createAlloca(Ty, nullptr, Name);

    Builder.CreateStore(Fn->getArg(Idx), Alloca);
    AllocaAddr[Name] = Alloca;
  }
  if (FnType->getReturnType()->isStruct() ||
      FnType->getReturnType()->isArray()) {
    ReturnValue = Fn->getArg(Fn->arg_size() - 1);
  } else if (!FnType->getReturnType()->isVoid()) {
    ReturnValue = Builder.CreateAlloca(convertType(FnType->getReturnType()),
                                       nullptr, "returnvalue");
  } else {
    ReturnValue = nullptr;
  }
}

void CodeGen::emitItemFn(const ItemFn &N) {
  CurrentFn = const_cast<ItemFn *>(&N);
  AllocaAddr.clear();

  std::string FnName = CurrentImpl == nullptr
                           ? N.identifier
                           : mangleFnName(CurrentImpl->getName(), N.identifier);
  llvm::Function *Fn = Module.getFunction(FnName);
  assert(Fn != nullptr);

  // EntryBlock
  llvm::BasicBlock *Entry = llvm::BasicBlock::Create(Context, "entry", Fn);
  Exit = llvm::BasicBlock::Create(Context, "exit", Fn);

  Builder.SetInsertPoint(Entry);

  const FuncQualType *FnType = N.getQualType();

  emitFnParam(N.function_parameters, FnType);

  llvm::Value *V = emitExprBlock(*N.block_expr);
  if (V) {
    if (N.block_expr->getQualType()->isStruct() ||
        N.block_expr->getQualType()->isArray()) {
      llvm::Type *DeclTy = convertType(N.block_expr->getQualType());
      createMemCpy(ReturnValue, V, DeclTy);
      V = nullptr;
    } else {
      V = getValue(V, N.getQualType()->getReturnType());
    }
  }

  llvm::BasicBlock *BB = Builder.GetInsertBlock();
  if (BB->getTerminator() == nullptr) {
    if (V) {
      Builder.CreateStore(V, ReturnValue);
    }
    Builder.CreateBr(Exit);
  }

  Builder.SetInsertPoint(Exit);
  if (!Fn->getReturnType()->isVoidTy()) {
    Builder.CreateRet(Builder.CreateLoad(
        convertType(N.getQualType()->getReturnType()), ReturnValue));
  } else {
    Builder.CreateRetVoid();
  }

  if (verifyFunction(*Fn, &llvm::errs())) {
    Fn->print(llvm::errs(), nullptr);
    exit(1);
  }

  CurrentFn = nullptr;
  ReturnValue = nullptr;
}

void CodeGen::emitItemImpl(const ItemImpl &N) {
  const StructQualType *Ty = dynamic_cast<const StructQualType*>(N.type->getQualType());
  assert(Ty != nullptr);

  CurrentImpl = Ty;
  ImplType = static_cast<llvm::StructType *>(getType(N.type.get()));
  if (!ImplType) {
    throw std::runtime_error("not impl for struct");
  }

  for (auto &I : N.associated_items) {
    emitItemNode(*I);
  }
  ImplType = nullptr;
  CurrentImpl = nullptr;
}

void CodeGen::emitStmtEmpty(const StmtEmpty &N) {}

void CodeGen::emitStmtItem(const StmtItem &N) {}

void CodeGen::emitStmtLet(const StmtLet &N) {
  auto *Pat = dynamic_cast<const PatternIdentifier*>(N.pattern.get());
  assert(Pat != nullptr);

  llvm::Type *Ty = getType(N.type.get());
  llvm::Value *InitVal = emitExprNode(*N.expr);
  if (Builder.GetInsertBlock()->getTerminator()) {
    return;
  }
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());

  const VarDecl &Decl = CurrentFn->getVarDecl(Pat->identifier);
  llvm::AllocaInst *Alloca1 =
      TmpB.CreateAlloca(convertType(Decl.Ty), nullptr, Pat->identifier);
  AllocaAddr[Pat->identifier] = Alloca1;

  if (N.expr->getQualType()->isStruct() || N.expr->getQualType()->isArray()) {
    createMemCpy(Alloca1, InitVal, convertType(Decl.Ty));
    return;
  }
  Builder.CreateStore(getValue(InitVal, N.expr->getQualType()), Alloca1);
}

void CodeGen::emitStmtExpr(const StmtExpr &N) { emitExprNode(*N.expr); }

llvm::Value *CodeGen::emitExprLiteralChar(const ExprLiteralChar &N) {
  return llvm::ConstantInt::get(llvm::Type::getInt8Ty(Context), N.literal);
}

llvm::Value *CodeGen::emitExprLiteralString(const ExprLiteralString &N) {
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::AllocaInst *Alloca = TmpB.CreateAlloca(
      llvm::PointerType::get(llvm::PointerType::get(llvm::Type::getInt8Ty(Context), 0), 0),
      nullptr, "globalstring");
  llvm::Value *value = Builder.CreateGlobalStringPtr(llvm::StringRef(N.literal));
  Builder.CreateStore(value, Alloca);
  return Alloca;
}

llvm::Value *CodeGen::emitExprLiteralInt(const ExprLiteralInt &N) {
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), N.literal);
}

llvm::Value *CodeGen::emitExprLiteralBool(const ExprLiteralBool &N) {
  return llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), N.literal);
}

llvm::Value *CodeGen::emitExprPath(const ExprPath &N) {
  if (!N.path2) {
    switch (N.path1->type) {
    case PathType::Identifier:
      // This only process local variable and const
      if (AllocaAddr.count(N.path1->identifier))
        return AllocaAddr[N.path1->identifier];
      if (Syms.constTable.count(N.path1->identifier)) {
        int64_t Value = Syms.constTable.getValue(N.path1->identifier);
        llvm::Type *ITy = llvm::Type::getInt32Ty(Context);
        return llvm::ConstantInt::get(ITy, Value);
      }
      llvm_unreachable("unexpected type here");
    case PathType::self: {
      return AllocaAddr["self"];
    }
    case PathType::Self:
      llvm_unreachable("TODO");
    }
  }

  // only process enum here
  const EnumQualType *Ty = dynamic_cast<const EnumQualType*>(N.getQualType());
  assert(Ty != nullptr && "Bug");

  // consider enum as i32
  int32_t Value = Ty->indexOf(N.path2->identifier);
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), Value);
}

llvm::Value *CodeGen::emitExprBlock(const ExprBlock &N) {
  for (auto &ST : N.stmts) {
    emitStmtNode(*ST);
  }
  if (N.expr) {
    return emitExprNode(*N.expr);
  }
  return nullptr;
}

llvm::Value *CodeGen::emitExprOpUnary(const ExprOpUnary &N) {
  llvm::Value *Val = emitExprNode(*N.expr);
  switch (N.type) {
  case BORROW_:       // & | &&
  case MUT_BORROW_: { // & | && mut
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    llvm::Type *Ty = llvm::PointerType::get(Context, 0);
    llvm::AllocaInst *Alloca = TmpB.CreateAlloca(Ty, nullptr, "arrayinit");
    Builder.CreateStore(Val, Alloca);
    return Alloca;
  }
  case DEREFERENCE_: { // *
    assert(Val->getType()->isPointerTy());
    return getValue(Val, N.expr->getQualType());
  }
  case NEGATE_: // -
    Val = getValue(Val, N.expr->getQualType());
    assert(Val->getType()->isIntegerTy());

    return Builder.CreateNeg(Val);
  case NOT_: // !
    Val = getValue(Val, N.expr->getQualType());
    return Builder.CreateNot(Val);
  default:
    llvm_unreachable("TODO");
  }
}

llvm::Value *CodeGen::emitExprOpBinary(const ExprOpBinary &N) {
  if (N.type == ExprOpBinaryType::AND_AND_) {
    llvm::Value *L = getValue(emitExprNode(*N.left), N.left->getQualType());
    if (!L) {
      return nullptr;
    }
    if (L->getType()->isIntegerTy(32)) {
      L = Builder.CreateICmpNE(L,
                               llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
    }
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *LHSBB = Builder.GetInsertBlock();
    llvm::BasicBlock *RHSBB = llvm::BasicBlock::Create(Context, "and.rhs", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(Context, "and.merge");

    Builder.CreateCondBr(L, RHSBB, MergeBB);

    Builder.SetInsertPoint(RHSBB);
    llvm::Value *R = getValue(emitExprNode(*N.right), N.right->getQualType());
    if (!R) {
      return nullptr;
    }
    if (R->getType()->isIntegerTy(32)) {
      R = Builder.CreateICmpNE(R,
                               llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
    }
    Builder.CreateBr(MergeBB);
    RHSBB = Builder.GetInsertBlock();

    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(llvm::Type::getInt1Ty(Context), 2, "andtmp");
    PN->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 0), LHSBB);
    PN->addIncoming(R, RHSBB);
    return PN;
  }

  if (N.type == ExprOpBinaryType::OR_OR_) {
    llvm::Value *L = getValue(emitExprNode(*N.left), N.left->getQualType());
    if (!L)
      return nullptr;
    if (L->getType()->isIntegerTy(32))
      L = Builder.CreateICmpNE(L,
                               llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));

    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *LHSBB = Builder.GetInsertBlock();
    llvm::BasicBlock *RHSBB = llvm::BasicBlock::Create(Context, "or.rhs", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(Context, "or.merge");

    Builder.CreateCondBr(L, MergeBB, RHSBB);

    Builder.SetInsertPoint(RHSBB);
    llvm::Value *R = getValue(emitExprNode(*N.right), N.right->getQualType());
    if (!R)
      return nullptr;
    if (R->getType()->isIntegerTy(32))
      R = Builder.CreateICmpNE(R,
                               llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
    Builder.CreateBr(MergeBB);
    RHSBB = Builder.GetInsertBlock();

    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder.CreatePHI(llvm::Type::getInt1Ty(Context), 2, "ortmp");
    PN->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 1), LHSBB);
    PN->addIncoming(R, RHSBB);
    return PN;
  }

  llvm::Value *LAddr = emitExprNode(*N.left);
  llvm::Value *RHS = getValue(emitExprNode(*N.right), N.right->getQualType());

  if (N.type == ExprOpBinaryType::ASSIGN_) {
    Builder.CreateStore(RHS, LAddr);
    return nullptr;
  }

  llvm::Value *LHS = getValue(LAddr, N.left->getQualType());
  llvm::Value *Result = nullptr;
  switch (N.type) {
  case ExprOpBinaryType::PLUS_EQ_:
    Result = Builder.CreateAdd(LHS, RHS);
    break;
  case ExprOpBinaryType::MINUS_EQ_:
    Result = Builder.CreateSub(LHS, RHS);
    break;
  case ExprOpBinaryType::MUL_EQ_:
    Result = Builder.CreateMul(LHS, RHS);
    break;
  case ExprOpBinaryType::DIV_EQ_:
    Result = Builder.CreateSDiv(LHS, RHS);
    break;
  case ExprOpBinaryType::MOD_EQ_:
    Result = Builder.CreateSRem(LHS, RHS);
    break;
  case ExprOpBinaryType::AND_EQ_:
    Result = Builder.CreateAnd(LHS, RHS);
    break;
  case ExprOpBinaryType::OR_EQ_:
    Result = Builder.CreateOr(LHS, RHS);
    break;
  case ExprOpBinaryType::XOR_EQ_:
    Result = Builder.CreateXor(LHS, RHS);
    break;
  case ExprOpBinaryType::SHL_EQ_:
    Result = Builder.CreateShl(LHS, RHS);
    break;
  case ExprOpBinaryType::SHR_EQ_:
    Result = Builder.CreateAShr(LHS, RHS);
    break;
  default:
    break;
  }
  if (Result) {
    Builder.CreateStore(Result, LAddr);
    return nullptr;
  }

  switch (N.type) {
  case ExprOpBinaryType::PLUS_:
    return Builder.CreateAdd(LHS, RHS, "addtmp");
  case ExprOpBinaryType::MINUS_:
    return Builder.CreateSub(LHS, RHS, "subtmp");
  case ExprOpBinaryType::MUL_:
    return Builder.CreateMul(LHS, RHS, "multmp");
  case ExprOpBinaryType::DIV_:
    return Builder.CreateSDiv(LHS, RHS, "divtmp");
  case ExprOpBinaryType::MOD_:
    return Builder.CreateSRem(LHS, RHS, "modtmp");
  case ExprOpBinaryType::AND_:
    return Builder.CreateAnd(LHS, RHS, "andtmp");
  case ExprOpBinaryType::OR_:
    return Builder.CreateOr(LHS, RHS, "ortmp");
  case ExprOpBinaryType::XOR_:
    return Builder.CreateXor(LHS, RHS, "xortmp");
  case ExprOpBinaryType::SHL_:
    return Builder.CreateShl(LHS, RHS, "shltmp");
  case ExprOpBinaryType::SHR_:
    return Builder.CreateAShr(LHS, RHS, "shrtmp");
  case ExprOpBinaryType::EQUAL_:
    return Builder.CreateICmpEQ(LHS, RHS, "cmptmp");
  case ExprOpBinaryType::NOT_EQUAL_:
    return Builder.CreateICmpNE(LHS, RHS, "cmptmp");
  case ExprOpBinaryType::LESS_:
    return Builder.CreateICmpSLT(LHS, RHS, "cmptmp");
  case ExprOpBinaryType::LESS_EQUAL_:
    return Builder.CreateICmpSLE(LHS, RHS, "cmptmp");
  case ExprOpBinaryType::GREATER_:
    return Builder.CreateICmpSGT(LHS, RHS, "cmptmp");
  case ExprOpBinaryType::GREATER_EQUAL_:
    return Builder.CreateICmpSGE(LHS, RHS, "cmptmp");
  default:
    llvm_unreachable("");
  }
}

llvm::Value *CodeGen::emitExprOpCast(const ExprOpCast &N) {
  llvm::Value *Val = emitExprNode(*N.expr);
  Val = getDerefValue(Val, N.expr->getQualType());
  return Builder.CreateIntCast(Val, convertType(N.getQualType()), true);
}

llvm::Value *CodeGen::emitExprGrouped(const ExprGrouped &N) {
  return emitExprNode(*N.expr);
}

llvm::Value *CodeGen::emitExprArrayExpand(const ExprArrayExpand &N) {
  // TODO: need checked type
  const ArrayQualType *QTy = dynamic_cast<const ArrayQualType*>(N.getQualType());
  if (QTy == nullptr) {
    throw std::runtime_error("not array type for array expand expr");
  }

  llvm::Type *ArrayTy =
      llvm::ArrayType::get(convertType(QTy->getElemType()), N.elements.size());

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::AllocaInst *Alloca = TmpB.CreateAlloca(ArrayTy, nullptr, "arrayinit");

  for (size_t i = 0; i < N.elements.size(); ++i) {
    llvm::Value *Val = emitExprNode(*N.elements[i]);
    Val = getDerefValue(Val, N.elements[i]->getQualType());
    assert(Val != nullptr);

    std::vector<llvm::Value *> IdxList;
    IdxList.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
    IdxList.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), i));

    llvm::Value *ElemPtr = Builder.CreateGEP(ArrayTy, Alloca, IdxList, "elemptr");
    Builder.CreateStore(Val, ElemPtr);
  }

  return Alloca;
}

llvm::Value *CodeGen::emitExprArrayAbbreviate(const ExprArrayAbbreviate &N) {
  const ArrayQualType *QTy = dynamic_cast<const ArrayQualType*>(N.getQualType());
  if (QTy == nullptr) {
    throw std::runtime_error("not array type for array expand expr");
  }
  llvm::Type *ArrayTy = convertType(QTy);

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::AllocaInst *Alloca = TmpB.CreateAlloca(ArrayTy, nullptr, "arrayinit");
  llvm::Value *InitVal = getValue(emitExprNode(*N.value), N.value->getQualType());

  llvm::Type *Int32Ty = llvm::Type::getInt32Ty(Context);
  llvm::Value *ArraySize = llvm::ConstantInt::get(Int32Ty, QTy->getLength());

  // create a loop to init array
  llvm::BasicBlock *CurBB = Builder.GetInsertBlock();
  llvm::BasicBlock *LoopHeader =
      llvm::BasicBlock::Create(Context, "loop_header", TheFunction);
  llvm::BasicBlock *LoopBody = llvm::BasicBlock::Create(Context, "loop_body", TheFunction);
  llvm::BasicBlock *LoopExit = llvm::BasicBlock::Create(Context, "loop_exit", TheFunction);

  llvm::Value *LoopIndex = llvm::ConstantInt::get(Int32Ty, 0); // loop index = 0
  Builder.CreateBr(LoopHeader);

  // loop header
  Builder.SetInsertPoint(LoopHeader);
  llvm::PHINode *IndexPhi = Builder.CreatePHI(Int32Ty, 2, "i");
  IndexPhi->addIncoming(LoopIndex, CurBB);
  llvm::Value *Cond = Builder.CreateICmpULT(IndexPhi, ArraySize, "cond");
  Builder.CreateCondBr(Cond, LoopBody, LoopExit);

  // loop body
  Builder.SetInsertPoint(LoopBody);

  std::vector<llvm::Value *> GEPIndices = {
      llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), 0), IndexPhi};
  llvm::Value *ElemAddr = Builder.CreateGEP(ArrayTy, Alloca, GEPIndices);
  Builder.CreateStore(InitVal, ElemAddr);

  llvm::Value *NextIndex = Builder.CreateAdd(IndexPhi, llvm::ConstantInt::get(Int32Ty, 1));

  Builder.CreateBr(LoopHeader);
  IndexPhi->addIncoming(NextIndex, LoopBody);

  Builder.SetInsertPoint(LoopExit);
  return Alloca;
}

llvm::Value *CodeGen::emitExprIndex(const ExprIndex &N) {
  llvm::Value *BaseV = emitExprNode(*N.array);

  if (N.array->getQualType()->isPointer()) {
    BaseV = Builder.CreateLoad(convertType(N.array->getQualType()), BaseV);
  }
  llvm::Value *IdxV = getValue(emitExprNode(*N.index), N.index->getQualType());
  std::vector<llvm::Value *> IdxList;
  IdxList.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
  IdxList.push_back(IdxV);
  const ArrayQualType *QTy = dynamic_cast<const ArrayQualType*>(N.array->getQualType());
  if (QTy == nullptr) {
    const PointerQualType *PT =
        dynamic_cast<const PointerQualType*>(N.array->getQualType());
    if (PT != nullptr) {
      QTy = dynamic_cast<const ArrayQualType*>(PT->getElemType());
    }
  }
  if (QTy == nullptr) {
    throw std::runtime_error("not array type for array index expr");
  }
  llvm::Type *ArrayTy = convertType(QTy);
  llvm::Value *ElemPtr = Builder.CreateGEP(ArrayTy, BaseV, IdxList, "elemptr");

  return ElemPtr;
}
llvm::Value *CodeGen::emitExprStruct(const ExprStruct &N) {
  const StructQualType *QTy = dynamic_cast<const StructQualType*>(N.getQualType());
  assert(QTy != nullptr);

  llvm::StructType *Ty = StructTyDef[QTy->getName()];

  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::AllocaInst *Alloca = TmpB.CreateAlloca(Ty, nullptr, "structinit");

  for (auto &Field : N.fields) {
    const std::string &FieldName = Field.identifier;
    llvm::Value *FieldVal = emitExprNode(*Field.expr);
    assert(FieldVal != nullptr);

    int Idx = QTy->getFieldIndex(FieldName);
    assert(Idx >= 0);

    llvm::Value *FieldPtr = Builder.CreateStructGEP(Ty, Alloca, Idx);
    const QualType *FieldTy = QTy->getFieldType(FieldName);
    if (FieldTy->isArray() || FieldTy->isStruct()) {
      createMemCpy(FieldPtr, FieldVal, convertType(FieldTy));
    } else {
      FieldVal = getValue(FieldVal, QTy->getFieldType(FieldName));
      Builder.CreateStore(FieldVal, FieldPtr);
    }
  }
  return Alloca;
}

llvm::Value *CodeGen::emitExprCall(const ExprCall &N) {
  const FuncQualType *FnTy = dynamic_cast<const FuncQualType*>(N.expr->getQualType());
  assert(FnTy != nullptr);

  const ExprPath *EP = dynamic_cast<const ExprPath*>(N.expr.get());
  assert(EP != nullptr);

  std::string FnName = extractManglePathIdentifier(*EP);
  llvm::Function *Fn = Module.getFunction(FnName);
  assert(Fn != nullptr);

  std::vector<llvm::Value *> ArgsV;
  for (auto &Arg : N.params) {
    llvm::Value *ArgV = emitExprNode(*Arg);
    ArgV = getValue(ArgV, Arg->getQualType());
    ArgsV.push_back(ArgV);
  }

  if (FnTy->getReturnType()->isArray() || FnTy->getReturnType()->isStruct()) {
    llvm::Type *RetTy = convertType(FnTy->getReturnType());
    llvm::AllocaInst *Alloca = createAlloca(RetTy, nullptr, "rettmp");
    ArgsV.push_back(Alloca);

    Builder.CreateCall(Fn, ArgsV, "");
    return Alloca;
  }

  return Builder.CreateCall(Fn, ArgsV, "");
}

llvm::Value *CodeGen::emitExprMethodCall(const ExprMethodCall &N) {
  llvm::Value *SelfV = emitExprNode(*N.expr);
  const QualType *Ty = N.expr->getQualType();

  // dereference
  if (const PointerQualType *PT = dynamic_cast<const PointerQualType*>(Ty)) {
    Ty = PT->getElemType();
    SelfV = Builder.CreateLoad(llvm::PointerType::get(Context, 0), SelfV);
  }

  if (const ArrayQualType *ATy = dynamic_cast<const ArrayQualType*>(Ty)) {
    assert(N.path->identifier == "len");
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), ATy->getLength());
  }

  const StructQualType *STy = dynamic_cast<const StructQualType*>(Ty);
  assert(STy != nullptr && "struct type not found");

  const FuncQualType *FnTy = STy->getMethodSig(N.path->identifier);
  assert(FnTy != nullptr);

  std::string FnName = mangleFnName(STy->getName(), N.path->identifier);
  llvm::Function *CalleeF = Module.getFunction(FnName);
  assert(CalleeF != nullptr && "method not found");

  std::vector<llvm::Value *> ArgsV;
  if (!FnTy->getParamTypes()[0]->isPointer()) { // pass as self value
    SelfV = Builder.CreateLoad(convertType(STy), SelfV);
  }
  ArgsV.push_back(SelfV);

  for (auto &Arg : N.params) {
    llvm::Value *ArgV = emitExprNode(*Arg);
    ArgV = getValue(ArgV, Arg->getQualType());
    ArgsV.push_back(ArgV);
  }

  if (FnTy->getReturnType()->isArray() || FnTy->getReturnType()->isStruct()) {
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
    llvm::Type *RetTy = convertType(FnTy->getReturnType());
    llvm::AllocaInst *Alloca = createAlloca(RetTy, nullptr, "rettmp");
    ArgsV.push_back(Alloca);
    Builder.CreateCall(CalleeF, ArgsV);
    return Alloca;
  }
  return Builder.CreateCall(CalleeF, ArgsV);
}

llvm::Value *CodeGen::emitExprField(const ExprField &N) {
  llvm::Value *Addr = emitExprNode(*N.expr);

  const QualType *AddrQTy = N.expr->getQualType();
  if (const PointerQualType *PT = dynamic_cast<const PointerQualType*>(AddrQTy)) {
    AddrQTy = PT->getElemType();
    Addr = Builder.CreateLoad(llvm::PointerType::get(Context, 0), Addr);
  }
  const StructQualType *ST = dynamic_cast<const StructQualType*>(AddrQTy);
  assert(ST != nullptr);

  size_t Idx = ST->getFieldIndex(N.identifier);
  return Builder.CreateStructGEP(convertType(ST), Addr, Idx);
}

llvm::Value *CodeGen::emitExprLoopInfinite(const ExprLoopInfinite &N) {
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(Context, "loop", TheFunction);
  llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(Context, "afterloop", TheFunction);

  // Save previous loop state
  llvm::BasicBlock *SavedHeadBB = CurrentHeadBB;
  llvm::BasicBlock *SavedAfterBB = CurrentAfterBB;
  llvm::Value *SavedLoopRes = CurrentLoopRes;

  Builder.CreateBr(LoopBB);
  Builder.SetInsertPoint(LoopBB);
  CurrentHeadBB = LoopBB;
  CurrentAfterBB = AfterBB;

  llvm::Type *Ty = convertType(N.getQualType());
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::AllocaInst *Alloca1 = nullptr;
  if (Ty->isVoidTy()) {
    CurrentLoopRes = nullptr;
  } else {
    Alloca1 = TmpB.CreateAlloca(Ty, nullptr, "iftmp");

    CurrentLoopRes = TmpB.CreateAlloca(Ty, nullptr, "loopres");
  }

  emitExprBlock(*N.block);

  if (!Builder.GetInsertBlock()->getTerminator())
    Builder.CreateBr(LoopBB);

  Builder.SetInsertPoint(AfterBB);

  // Restore state
  CurrentHeadBB = SavedHeadBB;
  CurrentAfterBB = SavedAfterBB;
  llvm::Value *RetVal = CurrentLoopRes;
  CurrentLoopRes = SavedLoopRes;

  return RetVal;
}

llvm::Value *CodeGen::emitExprLoopPredicate(const ExprLoopPredicate &N) {
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  // Create blocks
  llvm::BasicBlock *HeaderBB =
      llvm::BasicBlock::Create(Context, "loop.header", TheFunction);
  llvm::BasicBlock *BodyBB = llvm::BasicBlock::Create(Context, "loop.body", TheFunction);
  llvm::BasicBlock *LatchBB = llvm::BasicBlock::Create(Context, "loop.latch", TheFunction);
  llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(Context, "loop.exit", TheFunction);

  // Setup result storage
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  llvm::Type *Ty = convertType(N.block->getQualType());

  // Save previous loop state
  llvm::BasicBlock *SavedHeadBB = CurrentHeadBB;
  llvm::BasicBlock *SavedAfterBB = CurrentAfterBB;
  llvm::Value *SavedLoopRes = CurrentLoopRes;

  CurrentHeadBB = HeaderBB;
  CurrentAfterBB = ExitBB;
  if (!Ty->isVoidTy())
    CurrentLoopRes = TmpB.CreateAlloca(Ty, nullptr, "loop.res");

  // Entry -> Header
  Builder.CreateBr(HeaderBB);

  // Header: Check condition
  Builder.SetInsertPoint(HeaderBB);
  llvm::Value *CondV = emitExprNode(*N.condition);
  assert(CondV != nullptr);
  CondV = getValue(CondV, N.condition->getQualType());

  if (CondV->getType()->isIntegerTy(32)) {
    CondV = Builder.CreateICmpNE(
        CondV, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0), "loop.cond");
  }
  Builder.CreateCondBr(CondV, BodyBB, ExitBB);

  // Body
  Builder.SetInsertPoint(BodyBB);

  llvm::Value *body = emitExprBlock(*N.block);
  if (!N.block->getQualType()->isVoid())
    body = getValue(body, N.block->getQualType());
  if (body && !Ty->isVoidTy())
    Builder.CreateStore(body, CurrentLoopRes);

  // If not terminated (break/return), jump to Latch
  if (!Builder.GetInsertBlock()->getTerminator()) {
    Builder.CreateBr(LatchBB);
  }

  // Latch: Jump back to Header
  Builder.SetInsertPoint(LatchBB);
  Builder.CreateBr(HeaderBB);

  // Exit
  Builder.SetInsertPoint(ExitBB);

  // Load result (if break with value was used)
  llvm::Value *Result = nullptr;
  if (!Ty->isVoidTy())
    Result = Builder.CreateLoad(Ty, CurrentLoopRes, "loop.result");

  // Restore state
  CurrentHeadBB = SavedHeadBB;
  CurrentAfterBB = SavedAfterBB;
  CurrentLoopRes = SavedLoopRes;

  return Result;
}

llvm::Value *CodeGen::emitExprBreak(const ExprBreak &N) {
  llvm::Value *V = nullptr;
  if (N.expr) {
    V = getValue(emitExprNode(*N.expr), N.getQualType());
    Builder.CreateStore(V, CurrentLoopRes);
  }
  Builder.CreateBr(CurrentAfterBB);
  return V;
}

llvm::Value *CodeGen::emitExprContinue(const ExprContinue &N) {
  return Builder.CreateBr(CurrentHeadBB);
}

llvm::Value *CodeGen::emitExprIf(const ExprIf &N) {
  llvm::Value *CondV =
      getValue(emitExprNode(*N.condition), N.condition->getQualType());
  llvm::Type *Ty = nullptr;
  if (!N.getQualType()->isVoid()) {
    Ty = convertType(N.getQualType());
  }
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());

  llvm::AllocaInst *Alloca1 = nullptr;
  if (Ty)
    Alloca1 = TmpB.CreateAlloca(Ty, nullptr, "iftmp");

  if (!CondV)
    return nullptr;

  if (CondV->getType()->isIntegerTy(32)) {
    CondV = Builder.CreateICmpNE(
        CondV, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0), "ifcond");
  }

  llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(Context, "then", TheFunction);
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(Context, "ifcont", TheFunction);
  llvm::BasicBlock *ElseBB = nullptr;

  bool hasElse = (N.else_block != nullptr);
  if (hasElse) {
    ElseBB = llvm::BasicBlock::Create(Context, "else", TheFunction);
  }

  Builder.CreateCondBr(CondV, ThenBB, hasElse ? ElseBB : MergeBB);

  Builder.SetInsertPoint(ThenBB);

  llvm::Value *ThenV = emitExprBlock(*N.if_block);

  ThenBB = Builder.GetInsertBlock();

  if (!Builder.GetInsertBlock()->getTerminator()) {
    if (ThenV) {
      ThenV = getValue(ThenV, N.if_block->getQualType());
      Builder.CreateStore(ThenV, Alloca1);
    }
    Builder.CreateBr(MergeBB);
  }

  llvm::Value *ElseV = nullptr;
  if (hasElse) {
    Builder.SetInsertPoint(ElseBB);
    ElseV = emitExprNode(*N.else_block);
    if (ElseV && N.else_block->getQualType()->isVoid() == false) {
      ElseV = getValue(ElseV, N.else_block->getQualType());
      Builder.CreateStore(ElseV, Alloca1);
    }

    if (!Builder.GetInsertBlock()->getTerminator())
      Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();
  }

  Builder.SetInsertPoint(MergeBB);

  return Alloca1;
}

llvm::Value *CodeGen::emitExprReturn(const ExprReturn &N) {
  if (!N.expr) {
    Builder.CreateBr(Exit);
    return nullptr;
  }

  llvm::Value *V = emitExprNode(*N.expr);
  llvm::Value *RV = nullptr;
  const QualType *Ty = N.expr->getQualType();
  if (Ty->isStruct() || Ty->isArray()) {
    createMemCpy(ReturnValue, V, convertType(Ty));
  } else {
    RV = getValue(V, N.expr->getQualType());
    Builder.CreateStore(RV, ReturnValue);
  }

  Builder.CreateBr(Exit);
  llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *DEAD = llvm::BasicBlock::Create(Context, "dead", TheFunction);
  Builder.SetInsertPoint(DEAD);
  return RV;
}

llvm::Value *CodeGen::emitExprUnderscore(const ExprUnderscore &N) { return nullptr; }

void CodeGen::emitPatternIdentifier(const PatternIdentifier &N) {}
void CodeGen::emitPatternReference(const PatternReference &N) {}

llvm::Type *CodeGen::getType(const TypeNode *N) {
  if (const TypeReference *T = dynamic_cast<const TypeReference *>(N)) {
    return getReferenceType(*T);
  } else if (const TypePath *T = dynamic_cast<const TypePath *>(N)) {
    return getPathType(*T);
  } else if (const TypeArray *T = dynamic_cast<const TypeArray *>(N)) {
    return getArrayType(*T);
  } else if (const TypeUnit *T = dynamic_cast<const TypeUnit *>(N)) {
    return getUnitType(*T);
  }
  throw std::runtime_error("not expected node.");
}

llvm::Type *CodeGen::getPathType(const TypePath &N) {
  switch (N.path->type) {
  case PathType::Identifier: {
    // If a struct
    auto I = StructTyDef.find(N.path->identifier);
    if (I != StructTyDef.end()) {
      return I->second;
    }
    auto I1 = const_cast<SymTable &>(Syms).enumTable.getTy(N.path->identifier);
    if (I1 != nullptr) {
      return llvm::Type::getInt32Ty(Context);
    }

    llvm::Type *Ty = nullptr;
    if (N.path->identifier == "i32" || N.path->identifier == "u32" ||
        N.path->identifier == "usize" || N.path->identifier == "isize") {
      Ty = llvm::Type::getInt32Ty(Context);
    } else if (N.path->identifier == "bool") {
      Ty = llvm::Type::getInt1Ty(Context);
    } else if (N.path->identifier == "char") {
      Ty = llvm::Type::getInt8Ty(Context);
    } else if (N.path->identifier == "String") {
      Ty = llvm::PointerType::get(llvm::Type::getInt8Ty(Context), 0);
  }

    return Ty;
  }
  case PathType::Self:
  case PathType::self:
    throw std::runtime_error("not expected node.");
  }
  return nullptr;
}

llvm::Type *CodeGen::getReferenceType(const TypeReference &N) { return nullptr; }

llvm::Type *CodeGen::getArrayType(const TypeArray &N) {
  // TODO: need checked type
  return llvm::ArrayType::get(llvm::Type::getInt32Ty(Context), 10);
}

llvm::Type *CodeGen::getUnitType(const TypeUnit &N) { return nullptr; }
