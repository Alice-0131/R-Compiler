#include "../../include/Semantic/SymbolChecker.hpp"
#include "../../include/ASTNode/ASTNode.hpp"
#include "../../include/ASTNode/ExprField.hpp"
#include "../../include/ASTNode/ExprLiteral.hpp"
#include "../../include/ASTNode/ExprNode.hpp"
#include "../../include/ASTNode/ExprReturn.hpp"
#include "../../include/ASTNode/ItemConst.hpp"
#include "../../include/ASTNode/ItemFn.hpp"
#include "../../include/ASTNode/ItemNode.hpp"
#include "../../include/ASTNode/ItemStruct.hpp"
#include "../../include/ASTNode/Path.hpp"
#include "../../include/ASTNode/PatternIdentifier.hpp"
#include "../../include/ASTNode/PatternNode.hpp"
#include "../../include/ASTNode/TypeArray.hpp"
#include "../../include/ASTNode/TypePath.hpp"
#include "../../include/ASTNode/TypeReference.hpp"
#include "../../include/ASTNode/TypeUnit.hpp"
#include "../../include/Semantic/ConstSolver.hpp"
#include "../../include/Semantic/Type.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// type check the body of "function" and "method"
unsigned LocalScope::Counter = 0;

Checker::Checker(std::shared_ptr<Crate> &Prog, SymTable &Syms)
    : Prog(Prog), Syms(Syms){
  // Built-in functions
  Syms.fnTable.create("printInt", 
    FuncQualType::create(false,
      std::vector<const QualType *>{QualType::getI32Type()}, QualType::getVoidType()));
  Syms.fnTable.create("printlnInt", 
    FuncQualType::create(false,
      std::vector<const QualType *>{QualType::getI32Type()}, QualType::getVoidType()));

  Syms.fnTable.create( "exit", 
    FuncQualType::create(false, 
      std::vector<const QualType *>{QualType::getI32Type()},QualType::getVoidType()));
  Syms.fnTable.create( "getInt", 
    FuncQualType::create(false, std::vector<const QualType *>(), QualType::getI32Type()));
}

void Checker::check() {
  initRun();// move nested items (except const) to global
  firstRun();// collect struct, enum, const, trait and merge impl
  secondRun();// calculate the value of const items in global
  thirdRun();// collect field and impl of struct, signature of fn
  forthRun();// check impl trait of struct & trait
  fifthRun();// check each fn & impl in detail
}

void Checker::removeItem(std::unordered_set<ItemNode *> &remove) {
  auto &vec = Prog->children;
  auto it = vec.begin();
  while (it != vec.end()) {
    if (remove.count(it->get())) {
      it = vec.erase(it);
    } else {
      ++it;
    }
  }
}

void Checker::initRun() {
  std::vector<std::shared_ptr<ItemNode>> nestedItems;
  for (auto &item : Prog->children) {
    if (item->getTypeID() != ASTNode::K_ItemFn) continue;
    ItemFn &itemFn = dynamic_cast<ItemFn &>(*item);
    if (itemFn.block_expr == nullptr)
      continue;
    auto &stmts = itemFn.block_expr->stmts;
    auto it = stmts.begin();
    while (it != stmts.end()) {
      if (it->get()->getTypeID() != ASTNode::K_StmtItem) {
        it++;
        continue;
      }
      StmtItem &stmtItem = dynamic_cast<StmtItem&>(*it->get());
      switch (stmtItem.item->getTypeID()) {
      // can be dealed preciously by scope
      case ASTNode::K_ItemConst:
      default:
        it++;
        break;
      case ASTNode::K_ItemEnum:
      case ASTNode::K_ItemFn:
      case ASTNode::K_ItemImpl:
      case ASTNode::K_ItemStruct:
      case ASTNode::K_ItemTrait:
        auto s = *it;
        auto &item = dynamic_cast<StmtItem&>(*s);
        nestedItems.push_back(item.item);
        it = stmts.erase(it);
        break;
      }
    }
  }
  for (int i = 0; i < nestedItems.size(); ++i)
    Prog->children.push_back(nestedItems[i]);
}

void Checker::firstRun() {
  std::unordered_set<ItemNode *> remove;
  // used to merge all impls for a specific struct to one
  std::unordered_map<std::string, ItemImpl *> inhImplInfo;
  // used to merge all impls for a specific struct & trait to one
  std::unordered_map<std::pair<std::string, std::string>, ItemImpl *, PairHash>
      traitImplInfo;
  // visit all items in crate
  for (auto &item : Prog->children) {
    switch (item->getTypeID()) {
    default:
      throw std::runtime_error("unexpected item node.");
    case ASTNode::K_ItemFn:
      // deal in the second run
      break;
    case ASTNode::K_ItemEnum: {
      auto &enumItem = dynamic_cast<ItemEnum&>(*item);
      std::string &enumName = enumItem.identifier;
      const EnumQualType *Ty =
          EnumQualType::create(enumName, enumItem.enum_variants);
      if (!Syms.enumTable.create(enumName, Ty))
        throw std::runtime_error("duplicated enum.");
      break;
    }
    case ASTNode::K_ItemTrait: {
      auto &itemTrait = dynamic_cast<ItemTrait&>(*item);
      if (!Syms.traitTable.create(itemTrait.identifier)) {
        throw std::runtime_error("duplicated trait.");
      }
      break;
    }
    case ASTNode::K_ItemStruct: {
      auto &itemStruct = dynamic_cast<ItemStruct&>(*item);
      std::string &structName = itemStruct.identifier;
      if (!Syms.structTable.create(structName)) {
        throw std::runtime_error("duplicated struct.");
      }
      const StructQualType *Ty = Syms.structTable.getTy(structName);
      itemStruct.setQualType(Ty);
      break;
    }
    case ASTNode::K_ItemImpl: {
      auto &itemImpl = dynamic_cast<ItemImpl&>(*item);
      std::string &traitName = itemImpl.identifier;
      auto &typePath = dynamic_cast<TypePath &>(*itemImpl.type);
      std::string typeName = typePath.getTypeName();
      ItemImpl *&whichImpl =
          traitName.empty()
              ? inhImplInfo[typeName]
              : traitImplInfo[std::make_pair(traitName, typeName)];

      if (whichImpl == nullptr) {
        whichImpl = &itemImpl;
      } else {
        auto &f1 = whichImpl->associated_items;
        auto &f2 = itemImpl.associated_items;
        for (int i = 0; i < f2.size(); ++i)
          f1.push_back(f2[i]);
        // remove duplicated impl
        remove.insert(item.get());
      }
      break;
    }
    case ASTNode::K_ItemConst: {
      auto &constItem = dynamic_cast<ItemConst&>(*item);
      const QualType *Ty = getType(*constItem.type);
      std::string &constName = constItem.identifier;
      if (!Syms.constTable.create(constName, Ty)) {
        throw std::runtime_error("duplicated const.");
      }
      break;
    }
    }
  }
  // remove unnecessary item from Prog.chirldren
  removeItem(remove);
}

void Checker::secondRun(void) {
  ConstSolver solver;
  for (auto &item : Prog->children) {
    if (item->getTypeID() == ASTNode::K_ItemConst) {
      auto constItem = &dynamic_cast<ItemConst&>(*item);
      solver.question.insert(*constItem);
    }
  }
  if (!solver.solve()) {
    throw std::runtime_error("const solver failed.");
  }
  // get results

  auto s = solver.solution.takeItemSolution();
  while (s.first) {
    std::string &name = s.first->identifier;
    long value = s.second.getValue();
    Syms.constTable.setValue(name, value);
    s = solver.solution.takeItemSolution();
  }
}

void Checker::thirdRun(void) {
  std::unordered_set<ItemNode *> remove;
  for (auto &item : Prog->children) {
    switch (item->getTypeID()) {
    default: throw std::runtime_error("unexpected item node.");
    case ASTNode::K_ItemEnum:
    case ASTNode::K_ItemConst:
    case ASTNode::K_ItemImpl:
      break;
    case ASTNode::K_ItemTrait: {
      auto &itemTrait = dynamic_cast<ItemTrait&>(*item);
      collectTraitMethod(itemTrait);
      // remove trait
      remove.insert(item.get());
      break;
    }
    case ASTNode::K_ItemFn: {
      auto &itemFn = dynamic_cast<ItemFn&>(*item);
      collectFunction(itemFn);
      break;
    }
    case ASTNode::K_ItemStruct: {
      auto &itemStruct = dynamic_cast<ItemStruct&>(*item);
      collectStructField(itemStruct);
      break;
    }
    }
  }
  // remove unnecessary item from Prog.chirldren
  removeItem(remove);
}

void Checker::forthRun(void) {
  std::unordered_set<ItemNode *> remove;
  // merge trait impl for struct to impl of struct
  std::unordered_map<std::string, ItemImpl *> ImplInfo;
  for (auto &item : Prog->children) {
    if (item->getTypeID() != ASTNode::K_ItemImpl)continue;
    auto &itemImpl = dynamic_cast<ItemImpl&>(*item);
    std::string &traitName = itemImpl.identifier;
    auto &typePath = dynamic_cast<TypePath &>(*itemImpl.type);
    std::string typeName = typePath.getTypeName();
    if (!traitName.empty()) {
      // trait impl for struct
      if (!Syms.traitTable.count(traitName)) {
        throw std::runtime_error("implement undefined trait for struct.");
      }
      size_t traitSize = Syms.traitTable.getTrait(traitName).size();
      size_t traitImplSize = itemImpl.associated_items.size();
      if (traitSize != traitImplSize) {
        throw std::runtime_error("incompleted implementation of trait for struct.");
      }
      // trait name is useless
      traitName.clear();
    }
    collectStructMethod(itemImpl);
    if (ImplInfo.count(typeName)) {
      auto &itemList = ImplInfo[typeName]->associated_items;
      auto &traitImplItem = itemImpl.associated_items;
      for (int i = 0; i < traitImplItem.size(); ++i)
        itemList.push_back(std::move(traitImplItem[i]));
      remove.insert(&itemImpl);
    } else {
      ImplInfo[typeName] = &itemImpl;
    }
  }
  // remove unnecessary item from Prog.chirldren
  removeItem(remove);
}

void Checker::fifthRun(void) {
  for (auto &Item : Prog->children) {
    checkItemNode(*Item);
  }
}

const FuncQualType *Checker::setFnSignature(ItemFn &N, bool isImpl) {
  const QualType *retTy = N.function_return_type
                              ? getType(*N.function_return_type)
                              : QualType::getVoidType();
  std::vector<const QualType *> paramTy;
  auto &self = N.function_parameters.self_param;
  // push self type as the first paramter type
  if (self.flag == 1) {
    // ShorthandSelf
    const QualType *SelfTy =
        self.shorthand_self.is_and
            ? PointerQualType::create(self.shorthand_self.is_mut, CurImplTy)
            : CurImplTy;
    paramTy.push_back(SelfTy);
  } else if (self.flag == 2) {
    // TypedSelf
    throw std::runtime_error("TypedSelf in function parameter unsupported.");
  }

  for (auto &param : N.function_parameters.fn_params) {
    // FIXME: should consider pattern (ref & mut)
    const QualType *Ty = getType(*param.type);
    paramTy.push_back(Ty);
  }
  // attach fn signature to its Node
  const FuncQualType *Ty = FuncQualType::create(isImpl, paramTy, retTy);
  N.setFunctionType(Ty);
  return Ty;
}

void Checker::collectStructField(ItemStruct &N) {
  std::string &structName = N.identifier;
  if (!Syms.structTable.count(structName)) {
    throw std::runtime_error("Undefined struct.");
  }
  for (auto &F : N.struct_fields) {
    const QualType *FTy = getType(*F.type);
    auto field = StructQualType::Field(F.identifier, FTy);
    Syms.structTable.insertField(structName, field);
  }
}

void Checker::collectStructMethod(ItemImpl &N) {
  auto &typePath = dynamic_cast<TypePath &>(*N.type);
  std::string structName = typePath.getTypeName();
  const QualType *selfTy = getType(*N.type);
  if (!selfTy->isStruct()) {
    throw std::runtime_error("Impl type is not struct.");
  }
  CurImplTy = dynamic_cast<StructQualType*>(const_cast<QualType *>(selfTy));

  if (!Syms.structTable.count(structName)) {
    throw std::runtime_error("Undefined struct.");
  }
  for (auto &item : N.associated_items) {
    if (item->getTypeID() != ASTNode::K_ItemFn) {
      // FIXME: actually, item can be ItemConst, but we do not care.
      throw std::runtime_error("Invalid.");
    }
    auto &itemFn = dynamic_cast<ItemFn&>(*item);
    const FuncQualType *fnSig = setFnSignature(itemFn, true);
    Syms.structTable.insertMethod(structName, itemFn.identifier, fnSig);
  }
  CurImplTy = nullptr;
}

void Checker::collectTraitMethod(ItemTrait &N) {
  std::string &traitName = N.identifier;
  CurImplTy = QualType::getVoidType();

  if (!Syms.traitTable.count(traitName)) {
    throw std::runtime_error("Undefined trait.");
  }
  for (auto &item : N.associated_items) {
    if (item->getTypeID() != ASTNode::K_ItemFn) {
      // FIXME: actually, item can be ItemConst, but we do not care.
      throw std::runtime_error("Invalid.");
    }
    auto &itemFn = dynamic_cast<ItemFn&>(*item);
    const FuncQualType *fnSig = setFnSignature(itemFn, true);
    Syms.traitTable.insertMethod(traitName,
                                 std::make_pair(itemFn.identifier, fnSig));
  }
  CurImplTy = nullptr;
}

void Checker::collectFunction(ItemFn &N) {
  std::string &fnName = N.identifier;
  CurImplTy = nullptr;

  const FuncQualType *fnSig = setFnSignature(N, false);
  const QualType *retTy = fnSig->getReturnType();
  if (fnName == "main" && !retTy->isVoid()) {
    throw std::runtime_error("function main has non-void return type.");
  }
  if (!Syms.fnTable.create(fnName, fnSig)) {
    throw std::runtime_error("duplicated function.");
  }
}

void Checker::checkItemNode(ItemNode &N) {
  if (islocal) {
    switch (N.getTypeID()) {
    case ASTNode::K_ItemTrait:
    default: 
      throw std::runtime_error("unexpected item node in local scope.");
    // has been checked in the first run
    case ASTNode::K_ItemStruct:
    case ASTNode::K_ItemEnum:
    case ASTNode::K_ItemFn:
    case ASTNode::K_ItemImpl:
      throw std::runtime_error("nested item unsupported.");
    // item const in fn different from const in global
    case ASTNode::K_ItemConst:
      return checkItemConst(dynamic_cast<ItemConst&>(N));
    }
  } else {
    switch (N.getTypeID()) {
    case ASTNode::K_ItemTrait:
    default:
      throw std::runtime_error("unexpected item node.");
    // has been checked in the first run
    case ASTNode::K_ItemStruct:
    // has been checked in the second run
    case ASTNode::K_ItemConst:
      break;
    case ASTNode::K_ItemEnum:
      return checkItemEnum(dynamic_cast<ItemEnum&>(N));
    case ASTNode::K_ItemFn:
      return checkItemFn(dynamic_cast<ItemFn&>(N));
    case ASTNode::K_ItemImpl:
      return checkItemImpl(dynamic_cast<ItemImpl&>(N));
    }
  }
}

const QualType *Checker::checkExprNode(ExprNode &N) {

  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("unexpected expr node.");
  case ASTNode::K_ExprArrayAbbreviate:
    return checkExprArrayAbbreviate(dynamic_cast<ExprArrayAbbreviate&>(N));
  case ASTNode::K_ExprArrayExpand:
    return checkExprArrayExpand(dynamic_cast<ExprArrayExpand&>(N));
  case ASTNode::K_ExprBlock:
    return checkExprBlock(dynamic_cast<ExprBlock&>(N));
  case ASTNode::K_ExprBreak:
    return checkExprBreak(dynamic_cast<ExprBreak&>(N));
  case ASTNode::K_ExprCall:
    return checkExprCall(dynamic_cast<ExprCall&>(N));
  case ASTNode::K_ExprContinue:
    return checkExprContinue(dynamic_cast<ExprContinue&>(N));
  case ASTNode::K_ExprField:
    return checkExprField(dynamic_cast<ExprField&>(N));
  case ASTNode::K_ExprGrouped:
    return checkExprGrouped(dynamic_cast<ExprGrouped&>(N));
  case ASTNode::K_ExprIf:
    return checkExprIf(dynamic_cast<ExprIf&>(N));
  case ASTNode::K_ExprIndex:
    return checkExprIndex(dynamic_cast<ExprIndex&>(N));
  case ASTNode::K_ExprLiteralBool:
    return checkExprLiteralBool(dynamic_cast<ExprLiteralBool&>(N));
  case ASTNode::K_ExprLiteralChar:
    return checkExprLiteralChar(dynamic_cast<ExprLiteralChar&>(N));
  case ASTNode::K_ExprLiteralInt:
    return checkExprLiteralInt(dynamic_cast<ExprLiteralInt&>(N));
  case ASTNode::K_ExprLiteralString:
    return checkExprLiteralString(dynamic_cast<ExprLiteralString&>(N));
  case ASTNode::K_ExprLoopInfinite:
    return checkExprLoopInfinite(dynamic_cast<ExprLoopInfinite&>(N));
  case ASTNode::K_ExprLoopPredicate:
    return checkExprLoopPredicate(dynamic_cast<ExprLoopPredicate&>(N));
  case ASTNode::K_ExprMethodCall:
    return checkExprMethodCall(dynamic_cast<ExprMethodCall&>(N));
  case ASTNode::K_ExprOpBinary:
    return checkExprOpBinary(dynamic_cast<ExprOpBinary&>(N));
  case ASTNode::K_ExprOpCast:
    return checkExprOpCast(dynamic_cast<ExprOpCast&>(N));
  case ASTNode::K_ExprOpUnary:
    return checkExprOpUnary(dynamic_cast<ExprOpUnary&>(N));
  case ASTNode::K_ExprPath:
    return checkExprPath(dynamic_cast<ExprPath&>(N));
  case ASTNode::K_ExprReturn:
    return checkExprReturn(dynamic_cast<ExprReturn&>(N));
  case ASTNode::K_ExprStruct:
    return checkExprStruct(dynamic_cast<ExprStruct&>(N));
  }
}

const QualType *Checker::checkTypeNode(TypeNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("unexpected type node.");
  case ASTNode::K_TypeArray:
    return getArrayType(dynamic_cast<TypeArray&>(N));
  case ASTNode::K_TypePath:
    return getPathType(dynamic_cast<TypePath&>(N));
  case ASTNode::K_TypeReference:
    return getReferenceType(dynamic_cast<TypeReference&>(N));
  case ASTNode::K_TypeUnit:
    return getUnitType(dynamic_cast<TypeUnit&>(N));
  }
}

const QualType *Checker::checkPatternNode(PatternNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("unexpected pattern node.");
  case ASTNode::K_PatternIdentifier:
    return checkPatternIdentifier(dynamic_cast<PatternIdentifier&>(N));
  case ASTNode::K_PatternReference:
    return checkPatternReference(dynamic_cast<PatternReference&>(N));
  }
}

void Checker::checkStmtNode(StmtNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("unexpected stmt node.");
  case ASTNode::K_StmtEmpty:
    return checkStmtEmpty(dynamic_cast<StmtEmpty&>(N));
  case ASTNode::K_StmtItem:
    return checkStmtItem(dynamic_cast<StmtItem&>(N));
  case ASTNode::K_StmtLet:
    return checkStmtLet(dynamic_cast<StmtLet&>(N));
  case ASTNode::K_StmtExpr:
    return checkStmtExpr(dynamic_cast<StmtExpr&>(N));
  }
}

const QualType *Checker::checkPath(Path &N) {
  switch (N.type) {
  default:
    throw std::runtime_error("unexpected path node.");
  case PathType::Identifier:
  case PathType::self:
  case PathType::Self:
    return CurImplTy;
  }
}

void Checker::checkItemFn(ItemFn &N) {

  CurFunction = &N;
  islocal = true;
  scopes = new LocalScope;
  if (CurImplTy) {
    bool mut = N.function_parameters.self_param.shorthand_self.is_mut;
    bool ref = N.function_parameters.self_param.shorthand_self.is_and;
    const QualType *ArgTy = ref ? PointerQualType::create(mut, CurImplTy) : CurImplTy;
    CurFunction->createVarDecl("self", ArgTy, ref ? false : mut);
  }

  for (const FnParam &I : N.function_parameters.fn_params) {
    const QualType *ArgTy = getType(*I.type);
    if (PatternIdentifier *Iden =dynamic_cast<PatternIdentifier*>(I.pattern.get())) {
      CurFunction->createVarDecl(Iden->identifier, ArgTy, Iden->is_mut);
      continue;
    }
    throw std::runtime_error("unsupported pattern in function parameter.");
  }
  if (!N.getQualType()) {
    throw std::runtime_error("function has no function type.");
  }
  CurFunction->setFunctionType(N.getQualType());
  const QualType *RetTy = N.getQualType()->getReturnType();

  BCtx.enterScope(BlockCtx::Func);
  const QualType *R = checkExprBlock(*N.block_expr);

  BlockCtx::BlockInfo &BI = BCtx.getLastScope();
  auto &lastExpr = N.block_expr->expr;
  if ((lastExpr && !lastExpr->hasRet() &&
       lastExpr->getTypeID() != ASTNode::K_ExprReturn && !R->equals(RetTy)) ||
      (BI.getReturnType() && !RetTy->equals(BI.getReturnType()))) {
        throw std::runtime_error("return type of function " + N.identifier + " is not match.");
      }

  BCtx.exitScope();
  CurFunction = nullptr;
  islocal = false;
  scopes = nullptr;
}

void Checker::checkItemEnum(ItemEnum &N) {}

void Checker::checkItemConst(ItemConst &N) {
  std::string constName = N.identifier;
  if (scopes) {
    constName = scopes->createLocal(constName);
    N.identifier = constName;
  }
  if (Syms.constTable.count(constName)) {
    throw std::runtime_error("duplicated const.");
  }

  ConstSolver solver;
  solver.prioriKnowledge.insert(Syms.constTable);

  solver.question.insert(N);
  if (!solver.solve()) {
    throw std::runtime_error("const solver failed.");
  }
  auto s = solver.solution.takeItemSolution();
  const QualType *Ty = s.second.getTy();
  long value = s.second.getValue();
  Syms.constTable.create(constName, Ty, value);
}

void Checker::checkItemImpl(ItemImpl &N) {
  CurImplTy = const_cast<QualType *>(getType(*N.type));
  if (CurImplTy == nullptr) {
    throw std::runtime_error("impl type is null.");
  }

  for (auto &I : N.associated_items) {
    checkItemNode(*I);
  }
  CurImplTy = nullptr;
}

void Checker::checkStmtEmpty(StmtEmpty &N) {}

void Checker::checkStmtItem(StmtItem &N) { checkItemNode(*N.item); }

void Checker::checkStmtLet(StmtLet &N) {

  PatternIdentifier *PI = dynamic_cast<PatternIdentifier*>(N.pattern.get());
  if (!PI) {
    throw std::runtime_error("invalid variable name of let statement.");
  }

  const QualType *LTy = checkTypeNode(*N.type);
  const QualType *RTy = checkExprNode(*N.expr);
  if (LTy && RTy) {
      bool eq = LTy->equals(RTy);
      if (!eq) {
        throw std::runtime_error("the type of let statement is not match.");
      }
  } else {
      throw std::runtime_error("Null type in let statement");
  }

  if (RTy->isPointer()) {
    const PointerQualType *PTR = dynamic_cast<const PointerQualType*>(RTy);
    if (PTR->getElemType()->isString() &&
        N.expr->getTypeID() == ASTNode::K_ExprMethodCall) {
      // replace .to_string to a string literal
      const StringQualType *STy = dynamic_cast<const StringQualType*>(PTR->getElemType());
      std::string s = STy->getString();
      if (!s.empty()) {
        auto newExpr =
            std::make_shared<ExprLiteralString>(ExprLiteralString(s));
        N.expr = std::move(newExpr);
        checkExprNode(*N.expr);
      }
    }
  }

  std::string name = PI->identifier;
  if (scopes) {
    // may re-write current variable
    name = scopes->createLocal(name);
    // rewrite local to a new name
    PI->identifier = name;
  }
  CurFunction->createVarDecl(name, LTy, PI->is_mut);
}

void Checker::checkStmtExpr(StmtExpr &N) {
  checkExprNode(*N.expr);
  N.setRet(N.expr->hasRet());
}

const QualType *Checker::checkExprLiteralChar(ExprLiteralChar &N) {
  return N.setQualType(QualType::getCharType());
}

const QualType *Checker::checkExprLiteralString(ExprLiteralString &N) {
  // FIXME: is it correct to force string literal to a pointer?
  return N.setQualType(
      PointerQualType::create(false, StringQualType::create(N.literal)));
}

const QualType *Checker::checkExprLiteralInt(ExprLiteralInt &N) {
  return N.setQualType(IntLiteralQualType::create(N.literal));
}

const QualType *Checker::checkExprLiteralBool(ExprLiteralBool &N) {
  return N.setQualType(QualType::getBoolType());
}

const QualType *Checker::checkExprPath(ExprPath &N) {
  if (!N.path2) {
    const QualType *Ty;
    switch (N.path1->type) {
    case PathType::Identifier: {
      std::string name = N.path1->identifier;
      // change the name to its local name
      if (scopes && scopes->count(name)) {
        name = scopes->getNewName(name);
        N.path1->identifier = name;
      }
      if (CurFunction && CurFunction->containVarDecl(name)) {
        N.setMut(CurFunction->getVarMut(name));
        Ty = CurFunction->getVarDecl(name).Ty;
        break;
      } else if (Syms.fnTable.count(name)) {
        Ty = Syms.fnTable.getTy(name);
        break;
      } else if (Syms.structTable.count(name)) {
        Ty = Syms.structTable.getTy(name);
        break;
      } else if (Syms.constTable.count(name)) {
        Ty = Syms.constTable.getTy(name);
        break;
      }
      throw std::runtime_error("not found " + N.path1->identifier);
    }
    case PathType::self:
      N.setMut(CurFunction->getVarMut(N.path1->identifier));
      Ty = CurFunction->getVarDecl("self").Ty;
      break;
    case PathType::Self:
      Ty = CurImplTy;
      break;
    }
    return N.setQualType(Ty);
  }

  const QualType *Ty;
  switch (N.path1->type) {
  case PathType::Identifier:
    if (Syms.structTable.count(N.path1->identifier)) {
      Ty = Syms.structTable.getTy(N.path1->identifier);
      break;
    } else if (Syms.enumTable.count(N.path1->identifier)) {
      Ty = Syms.enumTable.getTy(N.path1->identifier);
      break;
    }
    throw std::runtime_error("struct/enum " + N.path1->identifier + " not found.");
    break;
  case PathType::self:
    throw std::runtime_error("invalid path expr.");
    break;
  case PathType::Self:
    Ty = dynamic_cast<const StructQualType*>(CurImplTy);
    break;
  }
  if (Ty == nullptr) {
    throw std::runtime_error("invalid path expr.");
  }
  if (N.path2->type != PathType::Identifier) {
    throw std::runtime_error("invalid path expr.");
  }

  switch (Ty->getTypeID()) {
  case QualType::T_struct: {
    const StructQualType *STy = dynamic_cast<const StructQualType*>(Ty);
    const FuncQualType *FTy = STy->getMethodSig(N.path2->identifier);
    if (!FTy) {
      throw std::runtime_error("struct " + N.path1->identifier + " does not have method " +
                             N.path2->identifier);
    }
    return N.setQualType(FTy);
  }
  case QualType::T_enum: {
    const EnumQualType *ETy = dynamic_cast<const EnumQualType*>(Ty);
    if (!ETy->contains(N.path2->identifier)) {
      throw std::runtime_error("enum " + N.path1->identifier + " does not have " + N.path2->identifier);
    }
    return N.setQualType(ETy);
  }
  default:
    throw std::runtime_error("invalid path expr.");
  }
}

const QualType *Checker::checkExprBlock(ExprBlock &N) {
  // enter a new scope
  scopes->enterScope();
  const QualType *Ty = QualType::getVoidType();
  bool ret = false;
  for (auto &ST : N.stmts) {
    checkStmtNode(*ST);
    ret = ST->hasRet();
  }
  if (N.expr) {
    Ty = checkExprNode(*N.expr);
    ret = N.expr->hasRet();
  }
  N.setRet(ret);
  // exit the scope
  scopes->exitScope();
  return N.setQualType(Ty);
}

const QualType *Checker::checkExprOpUnary(ExprOpUnary &N) {
  const QualType *Ty = checkExprNode(*N.expr);
  switch (N.type) {
  case BORROW_: // & | &&
    return N.setQualType(PointerQualType::create(false, Ty));
  case MUT_BORROW_: // & | && mut
    return N.setQualType(PointerQualType::create(true, Ty));
  case DEREFERENCE_: // *
    if (const PointerQualType *PTy = dynamic_cast<const PointerQualType*>(Ty)) {
      N.setMut(PTy->isMut());
      return N.setQualType(PTy->getElemType());
    }
    throw std::runtime_error("can not dereference non-pointer type.");
  case NEGATE_: // -
    if (!Ty->isI32() && !Ty->isIsize() && !Ty->isIntLiteral()) {
      throw std::runtime_error("negate(-) a non-integer value.");
    }
    if (const IntLiteralQualType *ITy = dynamic_cast<const IntLiteralQualType*>(Ty))
      Ty = IntLiteralQualType::create(-ITy->getValue());
    return N.setQualType(Ty);
  case NOT_: // !
    if (!Ty->isBool() && !Ty->equals(IntLiteralQualType::create(0))) {
      throw std::runtime_error("not(!) a non-bool/integer value.");
    }
    return N.setQualType(Ty);
  }
  throw std::runtime_error("unexpected unary operator.");
}

const QualType *Checker::checkExprOpBinary(ExprOpBinary &N) {
  const QualType *LTy = checkExprNode(*N.left);
  const QualType *RTy = checkExprNode(*N.right);

  switch (N.type) {
  default:
    if (!LTy->equals(RTy)) {
      throw std::runtime_error("the type of binary operator not match:");
    }
    break;
  case SHL_:    // <<
  case SHR_:    // >>
  case SHL_EQ_: // <<=
  case SHR_EQ_: // >>=
    if (!RTy->equals(IntLiteralQualType::create(0))) {
      throw std::runtime_error("the type of binary operator not match");
    }
  }

  if (RTy->isPointer()) {
    const PointerQualType *PTR = dynamic_cast<const PointerQualType*>(RTy);
    if (PTR->getElemType()->isString() &&
        N.right->getTypeID() == ASTNode::K_ExprMethodCall) {
      // replace .to_string to a string literal
      const StringQualType *STy = dynamic_cast<const StringQualType*>(PTR->getElemType());
      std::string s = STy->getString();
      if (!s.empty()) {
        auto newExpr =
            std::make_shared<ExprLiteralString>(ExprLiteralString(s));
        N.right = std::move(newExpr);
        checkExprNode(*N.right);
      }
    }
  }

  switch (N.type) {
  default:
    throw std::runtime_error("unexpected binary operator.");
  case PLUS_:                  // +
  case MINUS_:                 // -
  case MUL_:                   // *
  case DIV_:                   // /
  case MOD_:                   // %
  case AND_:                   // &
  case OR_:                    // |
  case XOR_:                   // ^
  case SHL_:                   // <<
  case SHR_:                   // >>
    return N.setQualType(LTy); // TODO: may check more details.
  case EQUAL_:                 // ==
  case NOT_EQUAL_:             // !=
  case GREATER_:               // >
  case LESS_:                  // <
  case GREATER_EQUAL_:         // >=
  case LESS_EQUAL_:            // <=
  case OR_OR_:                 // ||
  case AND_AND_:               // &&
    return N.setQualType(QualType::getBoolType());
  case ASSIGN_:   // =
  case PLUS_EQ_:  // +=
  case MINUS_EQ_: // -=
  case MUL_EQ_:   // *=
  case DIV_EQ_:   // /=
  case MOD_EQ_:   // %=
  case AND_EQ_:   // &=
  case OR_EQ_:    // |=
  case XOR_EQ_:   // ^=
  case SHL_EQ_:   // <<=
  case SHR_EQ_:   // >>=
    if (!N.left->isMut()) {
      throw std::runtime_error("can not assign to immutable.");
    }
    return N.setQualType(LTy);
  }
}

const QualType *Checker::checkExprOpCast(ExprOpCast &N) {
  // TODO: more details
  const QualType *OldTy = checkExprNode(*N.expr);
  const QualType *Ty = checkTypeNode(*N.type);

  bool OldIsInt = OldTy->isBool() || OldTy->isI32() || OldTy->isU32() ||
                  OldTy->isIsize() || OldTy->isUsize() || OldTy->isIntLiteral();
  bool TyIsInt = Ty->isBool() || Ty->isI32() || Ty->isU32() || Ty->isIsize() ||
                 Ty->isUsize();

  if (!OldIsInt || !TyIsInt) {
    throw std::runtime_error("invalid type cast op.");
  }
  return N.setQualType(Ty);
}

const QualType *Checker::checkExprGrouped(ExprGrouped &N) {
  const QualType *Ty = checkExprNode(*N.expr);
  return N.setQualType(Ty);
}

const QualType *Checker::checkExprArrayExpand(ExprArrayExpand &N) {

  size_t Length = N.elements.size();
  if (Length <= 0) {
    throw std::runtime_error("Wrong array expand.");
  }

  std::vector<const QualType *> Types;
  for (auto &I : N.elements) {
    Types.push_back(checkExprNode(*I));
    if (Types.size() >= 2 && !Types[Types.size() - 1]->equals(Types[Types.size() - 2])) {
      throw std::runtime_error("the type of array expand is not match.");
    }
  }
  
  auto AT = ArrayQualType::create(Types[0], Length);
  return N.setQualType(AT);
}

long Checker::evaluateExprNode(ExprNode &N) {
  ConstSolver solver;
  solver.prioriKnowledge.insert(Syms.constTable);
  solver.question.insert(N);
  if (!solver.solve()) {
    throw std::runtime_error("expression can not evaluate at compile time.");
  }
  auto solution = solver.solution.takeExprSolution();
  return solution.second.getValue();
}

const QualType *Checker::checkExprArrayAbbreviate(ExprArrayAbbreviate &N) {
  const QualType *Ty = checkExprNode(*N.value);
  checkExprNode(*N.size);
  long value = evaluateExprNode(*N.size);
  if (!(value >= 0 && value <= UINT32_MAX)) {
    throw std::runtime_error("array size out of range.");
  }
  unsigned Length = unsigned(value);

  return N.setQualType(ArrayQualType::create(Ty, Length));
}

const QualType *Checker::checkExprIndex(ExprIndex &N) {
  const QualType *Ty = checkExprNode(*N.array);
  bool mut = N.array->isMut();
  // dereference
  if (const PointerQualType *PT = dynamic_cast<const PointerQualType*>(Ty)) {
    Ty = PT->getElemType();
    mut = PT->isMut();
  }

  if (!Ty->isArray()) {
    throw std::runtime_error("index a non-array value.");
  }
  const QualType *ITy = checkExprNode(*N.index);
  if (!ITy->isUsize() && !ITy->isU32() && !ITy->isIntLiteral()) {
    throw std::runtime_error("index is not an unsigned integer.");
  }
  N.setMut(mut);
  return N.setQualType(dynamic_cast<const ArrayQualType*>(Ty)->getElemType());
}

const QualType *Checker::checkExprStruct(ExprStruct &N) {
  const QualType *Ty = checkExprPath(*N.path);
  if (!Ty->isStruct()) {
    throw std::runtime_error("invalid struct initialization.");
  }
  const StructQualType *STy = dynamic_cast<const StructQualType*>(Ty);

  if (N.fields.size() != STy->getFields().size()) {
    throw std::runtime_error("the field number of struct initialization is not match.");
  }

  for (auto &I : N.fields) {
    const QualType *FTy = checkExprNode(*I.expr);
    if (!FTy->equals(STy->getFieldType(I.identifier))) {
      throw std::runtime_error("the field type of " + I.identifier +" is not match");
    }
  }
  return N.setQualType(STy);
}

const QualType *Checker::checkExprCall(ExprCall &N) {
  const QualType *Ty = checkExprNode(*N.expr);
  if (!Ty->isFunc()) {
    throw std::runtime_error("ExprCall not on function.");
  }
  const FuncQualType *FTy = dynamic_cast<const FuncQualType*>(Ty);

  std::vector<const QualType *> ArgTys;
  for (auto &Arg : N.params) {
    ArgTys.push_back(checkExprNode(*Arg));
  }

  if (ArgTys.size() != FTy->getParamTypes().size()) {
    throw std::runtime_error("ExprCall argument number not match.");
  }

  if (!std::equal(ArgTys.begin(), ArgTys.end(), FTy->getParamTypes().begin(),
                  [&](const QualType *LHS, const QualType *RHS) {
                    return LHS->equals(RHS);
                  })) {
    throw std::runtime_error("ExprCall parameter types not match.");
  }
  N.setQualType(FTy->getReturnType());
  return FTy->getReturnType();
}

const QualType *Checker::checkExprMethodCall(ExprMethodCall &N) {
  const QualType *Ty = checkExprNode(*N.expr);
  bool mutSelf = N.expr->isMut();
  // dereference
  if (const PointerQualType *PT = dynamic_cast<const PointerQualType*>(Ty)) {
    Ty = PT->getElemType();
    mutSelf = PT->isMut();
  }

  // only support .len()
  if (Ty->isArray()) {
    if (N.path->identifier == "len") {
      return N.setQualType(QualType::getUsizeType());
    }
    throw std::runtime_error("unknown method " + N.path->identifier);
  }

  // only support .to_string() for int literal
  if (Ty->isIntLiteral()) {
    if (N.path->identifier == "to_string") {
      long value = evaluateExprNode(*N.expr);
      std::string s = std::to_string(value);
      return PointerQualType::create(false, StringQualType::create(s));
    }
    throw std::runtime_error("unknown method " + N.path->identifier);
  }

  if (!Ty->isStruct()) {
    throw std::runtime_error("wrong method call.");
  }

  const StructQualType *STy = dynamic_cast<const StructQualType*>(Ty);
  const FuncQualType *FTy = STy->getMethodSig(N.path->identifier);
  if (!FTy) {
    throw std::runtime_error("method " + N.path->identifier + " does not exist.");
  }

  auto &ParamTypes = FTy->getParamTypes();
  if (ParamTypes.size() != N.params.size() + 1) {
    throw std::runtime_error("the number of arguments of method " + N.path->identifier +
                             " is not match.");
  }
  const QualType *FSelf = ParamTypes[0];
  if (const PointerQualType *PFSelf = dynamic_cast<const PointerQualType*>(FSelf)) {
    auto mutFSelf = PFSelf->isMut();
    if (mutFSelf && !mutSelf) {
      throw std::runtime_error("can not convert immutable reference to mutable reference.");
    }
  }
  int Idx = 0;
  for (auto &Param : N.params) {
    const QualType *PTy = checkExprNode(*Param);
    const QualType *FTy = ParamTypes[Idx + 1];
    if (!PTy->equals(FTy)) {
      throw std::runtime_error("the type of argument " + std::to_string(Idx) +
                             " of method " + N.path->identifier + " is not match.");
    }
    Idx++;
  }
  return N.setQualType(FTy->getReturnType());
}

const QualType *Checker::checkExprField(ExprField &N) {
  const QualType *Ty = checkExprNode(*N.expr);
  bool mut = N.expr->isMut();
  if (const PointerQualType *P = dynamic_cast<const PointerQualType*>(Ty)) {
    Ty = P->getElemType();
    mut = P->isMut();
  }

  if (const StructQualType *St = dynamic_cast<const StructQualType*>(Ty)) {
    const QualType *FTy = St->getFieldType(N.identifier);
    if (!FTy) {
      throw std::runtime_error("field " + N.identifier + " not exist.");
    }
    N.setMut(mut);
    return N.setQualType(FTy);
  }
  throw std::runtime_error("not a struct.");
  return nullptr;
}

const QualType *Checker::checkExprLoopInfinite(ExprLoopInfinite &N) {
  BCtx.enterScope(BlockCtx::Loop);
  checkExprBlock(*N.block);

  const QualType *Ty = BCtx.getLastScope().getReturnType();

  BCtx.exitScope();
  // loop infinite can not return a value by non return or break expr
  N.setRet(Ty == nullptr);
  return N.setQualType(Ty ? Ty : QualType::getVoidType());
}

const QualType *Checker::checkExprLoopPredicate(ExprLoopPredicate &N) {
  const QualType *Ty = checkExprNode(*N.condition);
  if (!Ty->isBool()) {
    throw std::runtime_error("loop condition is not bool.");
  }

  BCtx.enterScope(BlockCtx::Loop);
  checkExprBlock(*N.block);

  const QualType *RTy = BCtx.getLastScope().getReturnType();

  BCtx.exitScope();
  return N.setQualType(RTy ? RTy : QualType::getVoidType());
}

const QualType *Checker::checkExprBreak(ExprBreak &N) {
  if (!BCtx.inScope(BlockCtx::Loop)) {
    throw std::runtime_error("break not in a loop.");
  }

  const QualType *Ty;
  if (N.expr) {
    Ty = checkExprNode(*N.expr);
  } else {
    Ty = QualType::getVoidType();
  }

  BlockCtx::BlockInfo &S = BCtx.getLastScope(BlockCtx::Loop);
  if (S.getReturnType() && !S.getReturnType()->equals(Ty)) {
    throw std::runtime_error("the types of the two break are not match.");
  } else {
    S.setReturnType(Ty);
  }
  return N.setQualType(Ty);
}

const QualType *Checker::checkExprContinue(ExprContinue &N) {
  if (!BCtx.inScope(BlockCtx::Loop)) {
    throw std::runtime_error("continue not in a loop.");
  }
  N.setQualType(QualType::getVoidType());
  return N.getQualType();
}

const QualType *Checker::checkExprIf(ExprIf &N) {
  const QualType *CondTy = checkExprNode(*N.condition);
  if (!CondTy->isBool()) {
    throw std::runtime_error("the condition type of if expr is not bool.");
  }

  const QualType *IfTy = checkExprBlock(*N.if_block);
  bool ret = N.if_block->hasRet();
  const QualType *retTy = IfTy;

  if (N.else_block) {
    const QualType *ElseTy = checkExprNode(*N.else_block);
    bool elseRet = N.else_block->hasRet();

    if (ret && elseRet) {
      // both if and else returned
      retTy = QualType::getVoidType();
    } else if (ret && !elseRet) {
      // only if block returned
      ret = false;
      // set return type to elseTy
      retTy = ElseTy;
    } else if (!ret && !elseRet) {
      // both if and else not returned
      retTy = IfTy;
      if (!IfTy->equals(ElseTy)) {
        throw std::runtime_error("the return type of if-else expr is not match");
      }
    } // only else block returned (do nothing)
  } else {
    retTy = QualType::getVoidType();
    ret = false;
  }
  N.setRet(ret);
  return N.setQualType(retTy);
}

const QualType *Checker::checkExprReturn(ExprReturn &N) {
  if (!N.expr)
    return QualType::getVoidType();
  const QualType *Ty = checkExprNode(*N.expr);
  if (!BCtx.inScope(BlockCtx::Func)) {
    throw std::runtime_error("return not in a function.");
  }

  const QualType *RetTy = CurFunction->getQualType()->getReturnType();
  if (!Ty->equals(RetTy)) {
    throw std::runtime_error("return type not match.");
  }

  BlockCtx::BlockInfo &BI = BCtx.getLastScope(BlockCtx::Func);
  if (BI.getReturnType() && !BI.getReturnType()->equals(Ty)) {
    throw std::runtime_error("the types of the two return are not match.");
  } else {
    BI.setReturnType(Ty);
  }

  N.setRet(true);
  return N.setQualType(Ty);
}

const QualType *Checker::checkPatternIdentifier(PatternIdentifier &N) {
  if (CurFunction->containVarDecl(N.identifier)) {
    return CurFunction->getVarDecl(N.identifier).Ty;
  }
  if (Syms.constTable.count(N.identifier)) {
    return Syms.constTable.getTy(N.identifier);
  }
  throw std::runtime_error("unknown identifier: " + N.identifier);
}

const QualType *Checker::checkPatternReference(PatternReference &N) {
  return checkPatternNode(*N.pattern);
}

const QualType *Checker::getType(TypeNode &N) {
  switch (N.getTypeID()) {
  default:
    throw std::runtime_error("unexpected type node.");
  case ASTNode::K_TypeReference:
    return N.setQualType(getReferenceType(static_cast<TypeReference &>(N)));
  case ASTNode::K_TypeArray:
    return N.setQualType(getArrayType(static_cast<TypeArray&>(N)));
  case ASTNode::K_TypeUnit:
    return N.setQualType(getUnitType(static_cast<TypeUnit&>(N)));
  case ASTNode::K_TypePath:
    return N.setQualType(getPathType(static_cast<TypePath&>(N)));
  }
}

const QualType *Checker::getPathType(TypePath &N) {
  const QualType *Ty;
  switch (N.path->type) {
  case PathType::Identifier: {
    Ty = nullptr;
    const std::string &name = N.path->identifier;
    if (name == "bool")  Ty =  QualType::getBoolType();
    if (name == "i32")   Ty =  QualType::getI32Type();
    if (name == "u32")   Ty =  QualType::getU32Type();
    if (name == "usize") Ty =  QualType::getUsizeType();
    if (name == "isize") Ty =  QualType::getIsizeType();
    if (name == "char")  Ty =  QualType::getCharType();
    if (name == "str")   Ty =  StringQualType::create("");
    if (name == "String") Ty =  PointerQualType::create(false, StringQualType::create(""));
    if (Ty) break;
    if (Syms.structTable.count(N.path->identifier)) {
      Ty = Syms.structTable.getTy(N.path->identifier);
      break;
    }
    if (Syms.enumTable.count(N.path->identifier)) {
      Ty = Syms.enumTable.getTy(N.path->identifier);
      break;
    }
    if (Syms.constTable.count(N.path->identifier)) {
      Ty = Syms.constTable.getTy(N.path->identifier);
      break;
    }
    throw std::runtime_error("not found identifier " + N.path->identifier);
  }
  case PathType::self:
  case PathType::Self:
    Ty = CurImplTy ? CurImplTy : QualType::getVoidType();
    break;
  default:
    throw std::runtime_error("unexpected path type.");
  }
  return N.path->setQualType(Ty);
}

const QualType *Checker::getReferenceType(TypeReference &N) {
  return PointerQualType::create(N.is_mut, checkTypeNode(*N.type));
}

const QualType *Checker::getArrayType(TypeArray &N) {
  const QualType *Ty = checkTypeNode(*N.type);
  checkExprNode(*N.expr);
  long value = evaluateExprNode(*N.expr);
  if (!(value >= 0 && value <= UINT32_MAX)) {
    throw std::runtime_error("array size out of range.");
  }
  unsigned Length = unsigned(value);

  return ArrayQualType::create(Ty, Length);
}

const QualType *Checker::getUnitType(TypeUnit &N) {
  return QualType::getVoidType();
}