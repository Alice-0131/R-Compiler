#ifndef ASTNODE_HPP
#define ASTNODE_HPP
#include "../ASTVisitor/ASTVisitor.hpp"
#include <memory>
#include <string>
#include <vector>

class ASTNode {
public:
  enum TypeID {
    K_ASTNode = 0,
    K_Crate,
    K_ExprArrayAbbreviate,
    K_ExprArrayExpand,
    K_ExprBlock,
    K_ExprBreak,
    K_ExprCall,
    K_ExprContinue,
    K_ExprField,
    K_ExprGrouped,
    K_ExprIf,
    K_ExprIndex,
    K_ExprLiteralBool,
    K_ExprLiteralChar,
    K_ExprLiteralInt,
    K_ExprLiteralString,
    K_ExprLoopInfinite,
    K_ExprLoopPredicate,
    K_ExprMethodCall,
    K_ExprNode,
    K_ExprOpBinary,
    K_ExprOpCast,
    K_ExprOpUnary,
    K_ExprPath,
    K_ExprReturn,
    K_ExprStruct,
    K_ItemConst,
    K_ItemEnum,
    K_ItemFn,
    K_ItemImpl,
    K_ItemStruct,
    K_ItemTrait,
    K_Path,
    K_PatternIdentifier,
    K_PatternReference,
    K_StmtEmpty,
    K_StmtExpr,
    K_StmtItem,
    K_StmtLet,
    K_TypeArray,
    K_TypePath,
    K_TypeReference,
    K_TypeUnit,
    K_ExprUnderscore,
    K_PatternLiteral,
    K_PatternWildcard,
    K_PatternPath,
  };

private:
  const TypeID Kind;

public:
  ASTNode(TypeID Tid = K_ASTNode) : Kind(Tid) {}

  virtual void accept(ASTVisitor &visitor) = 0;
  virtual ~ASTNode() = default;

  TypeID getTypeID() const { return Kind; }
};

#endif