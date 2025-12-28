#ifndef ASTVISITOR_HPP
#define ASTVISITOR_HPP
class Crate;
class Path;

class ItemNode;
class ItemFn;
class ItemStruct;
class ItemEnum;
class ItemConst;
class ItemTrait;
class ItemImpl;

class StmtNode;
class StmtEmpty;
class StmtItem;
class StmtLet;
class StmtExpr;

class ExprNode;
class ExprLiteralChar;
class ExprLiteralString;
class ExprLiteralInt;
class ExprLiteralBool;
class ExprPath;
class ExprBlock;
class ExprOpUnary;
class ExprOpBinary;
class ExprOpCast;
class ExprGrouped;
class ExprArrayExpand;
class ExprArrayAbbreviate;
class ExprIndex;
class ExprStruct;
class ExprCall;
class ExprMethodCall;
class ExprField;
class ExprLoopInfinite;
class ExprLoopPredicate;
class ExprBreak;
class ExprContinue;
class ExprIf;
class ExprReturn;
class ExprUnderscore;

class PatternNode;
class PatternLiteral;
class PatternIdentifier;
class PatternWildcard;
class PatternReference;
class PatternPath;

class TypeNode;
class TypePath;
class TypeReference;
class TypeArray;
class TypeUnit;

class ASTVisitor
{
public:
  virtual void visit(Crate &node) = 0;
  virtual void visit(Path &node) = 0;

  virtual void visit(ItemFn &node) = 0;
  virtual void visit(ItemStruct &node) = 0;
  virtual void visit(ItemEnum &node) = 0;
  virtual void visit(ItemConst &node) = 0;
  virtual void visit(ItemTrait &node) = 0;
  virtual void visit(ItemImpl &node) = 0;

  virtual void visit(StmtEmpty &node) = 0;
  virtual void visit(StmtItem &node) = 0;
  virtual void visit(StmtLet &node) = 0;
  virtual void visit(StmtExpr &node) = 0;

  virtual void visit(ExprLiteralChar &node) = 0;
  virtual void visit(ExprLiteralString &node) = 0;
  virtual void visit(ExprLiteralInt &node) = 0;
  virtual void visit(ExprLiteralBool &node) = 0;
  virtual void visit(ExprPath &node) = 0;
  virtual void visit(ExprBlock &node) = 0;
  virtual void visit(ExprOpUnary &node) = 0;
  virtual void visit(ExprOpBinary &node) = 0;
  virtual void visit(ExprOpCast &node) = 0;
  virtual void visit(ExprGrouped &node) = 0;
  virtual void visit(ExprArrayExpand &node) = 0;
  virtual void visit(ExprArrayAbbreviate &node) = 0;
  virtual void visit(ExprIndex &node) = 0;
  virtual void visit(ExprStruct &node) = 0;
  virtual void visit(ExprCall &node) = 0;
  virtual void visit(ExprMethodCall &node) = 0;
  virtual void visit(ExprField &node) = 0;
  virtual void visit(ExprLoopInfinite &node) = 0;
  virtual void visit(ExprLoopPredicate &node) = 0;
  virtual void visit(ExprBreak &node) = 0;
  virtual void visit(ExprContinue &node) = 0;
  virtual void visit(ExprIf &node) = 0;
  virtual void visit(ExprReturn &node) = 0;

  virtual void visit(PatternIdentifier &node) = 0;
  virtual void visit(PatternReference &node) = 0;

  virtual void visit(TypePath &node) = 0;
  virtual void visit(TypeReference &node) = 0;
  virtual void visit(TypeArray &node) = 0;
  virtual void visit(TypeUnit &node) = 0;
};

#endif