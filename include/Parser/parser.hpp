#ifndef PARSER_HPP
#define PARSER_HPP
#include <vector>
#include <memory>
#include "../Lexer/token.hpp"
#include "../ASTNode/ASTNode.hpp"
#include "../ASTNode/Crate.hpp"
#include "../ASTNode/Path.hpp"
#include "../ASTNode/ItemNode.hpp"
#include "../ASTNode/ItemFn.hpp"
#include "../ASTNode/ItemConst.hpp"
#include "../ASTNode/ItemStruct.hpp"
#include "../ASTNode/ItemEnum.hpp"
#include "../ASTNode/ItemConst.hpp"
#include "../ASTNode/ItemTrait.hpp"
#include "../ASTNode/ItemImpl.hpp"
#include "../ASTNode/StmtNode.hpp"
#include "../ASTNode/StmtEmpty.hpp"
#include "../ASTNode/StmtItem.hpp"
#include "../ASTNode/StmtLet.hpp"
#include "../ASTNode/StmtExpr.hpp"
#include "../ASTNode/ExprNode.hpp"
#include "../ASTNode/ExprLiteral.hpp"
#include "../ASTNode/ExprPath.hpp"
#include "../ASTNode/ExprBlock.hpp"
#include "../ASTNode/ExprOperator.hpp"
#include "../ASTNode/ExprGrouped.hpp"
#include "../ASTNode/ExprArrayIndex.hpp"
#include "../ASTNode/ExprStruct.hpp"
#include "../ASTNode/ExprCall.hpp"
#include "../ASTNode/ExprMethodCall.hpp"
#include "../ASTNode/ExprField.hpp"
#include "../ASTNode/ExprLoop.hpp"
#include "../ASTNode/ExprIf.hpp"
#include "../ASTNode/ExprReturn.hpp"
#include "../ASTNode/ExprUnderscore.hpp"
#include "../ASTNode/PatternNode.hpp"
#include "../ASTNode/PatternLiteral.hpp"
#include "../ASTNode/PatternIdentifier.hpp"
#include "../ASTNode/PatternWildcard.hpp"
#include "../ASTNode/PatternReference.hpp"
#include "../ASTNode/PatternPath.hpp"
#include "../ASTNode/TypeNode.hpp"
#include "../ASTNode/TypePath.hpp"
#include "../ASTNode/TypeReference.hpp"
#include "../ASTNode/TypeArray.hpp"
#include "../ASTNode/TypeUnit.hpp"

class PatternNode;
class TypeNode;

class Parser
{
private:
  const std::vector<Token>& tokens;
  int pos;

  std::unique_ptr<Path> parsePath();

  std::unique_ptr<ItemNode> parseItemNode();
  std::unique_ptr<ItemFn> parseItemFn();
  void parseItemFnParameters(FnParameters &function_parameters);
  bool parseItemFnSelfParam(SelfParam &self_param);
  void parseItemFnParams(std::vector<FnParam> &fn_params);
  std::unique_ptr<ItemStruct> parseItemStruct();
  void parseItemStructFields(std::vector<StructField> &struct_fields);
  std::unique_ptr<ItemEnum> parseItemEnum();
  std::unique_ptr<ItemConst> parseItemConst();
  std::unique_ptr<ItemTrait> parseItemTrait();
  std::unique_ptr<ItemImpl> parseItemImpl();
  std::unique_ptr<ItemAssociatedNode> parseItemAssociatedNode();

  std::unique_ptr<StmtNode> parseStmtNode();
  std::unique_ptr<StmtEmpty> parseStmtEmpty();
  std::unique_ptr<StmtItem> parseStmtItem();
  std::unique_ptr<StmtLet> parseStmtLet();
  std::unique_ptr<StmtExpr> parseStmtExpr();

  std::unique_ptr<ExprNode> parseExprNode(int ctxPrecedence = 0);
  std::unique_ptr<ExprNode> parseExprPrefix();
  std::unique_ptr<ExprNode> parseExprInfix(std::unique_ptr<ExprNode> &&left, const Token &token);
  std::unique_ptr<ExprLiteralNode> parseExprLiteralNode();
  std::unique_ptr<ExprLiteralChar> parseExprLiteralChar();
  std::unique_ptr<ExprLiteralString> parseExprLiteralString();
  std::unique_ptr<ExprLiteralInt> parseExprLiteralInt();
  std::unique_ptr<ExprLiteralBool> parseExprLiteralBool();
  std::unique_ptr<ExprPath> parseExprPath();
  std::unique_ptr<ExprBlock> parseExprBlock();
  //std::unique_ptr<ExprOperatorNode> parseExprOperatorNode();
  std::unique_ptr<ExprOpUnary> parseExprOpUnary();
  std::unique_ptr<ExprOpBinary> parseExprOpBinary(std::unique_ptr<ExprNode> &&left, const Token &token);
  std::unique_ptr<ExprOpCast> parseExprOpCast(std::unique_ptr<ExprNode> &&left);
  std::unique_ptr<ExprGrouped> parseExprGrouped();
  std::unique_ptr<ExprArrayNode> parseExprArrayNode();
  // std::unique_ptr<ExprArrayAbbreviate> parseExprArrayAbbreviate();
  // std::unique_ptr<ExprArrayExpand> parseExprArrayExpand();
  std::unique_ptr<ExprIndex> parseExprIndex(std::unique_ptr<ExprNode> &&left);
  std::unique_ptr<ExprStruct> parseExprStruct(std::unique_ptr<ExprPath> &&left);
  void parseExprStructField(StructExprField &field);
  std::unique_ptr<ExprCall> parseExprCall(std::unique_ptr<ExprNode> &&left);
  std::unique_ptr<ExprNode> parseExprMethodAndField(std::unique_ptr<ExprNode> &&left);
  //std::unique_ptr<ExprLoopNode> parseExprLoopNode();
  std::unique_ptr<ExprLoopInfinite> parseExprLoopInfinite();
  std::unique_ptr<ExprLoopPredicate> parseExprLoopPredicate();
  std::unique_ptr<ExprBreak> parseExprBreak();
  std::unique_ptr<ExprContinue> parseExprContinue();
  std::unique_ptr<ExprIf> parseExprIf();
  std::unique_ptr<ExprReturn> parseExprReturn();
  std::unique_ptr<ExprUnderscore> parseExprUnderscore();

  std::unique_ptr<PatternNode> parsePatternNode();
  std::unique_ptr<PatternLiteral> parsePatternLiteral();
  std::unique_ptr<PatternIdentifier> parsePatternIdentifier();
  std::unique_ptr<PatternWildcard> parsePatternWildcard();
  std::unique_ptr<PatternReference> parsePatternReference();
  std::unique_ptr<PatternPath> parsePatternPath();

  std::unique_ptr<TypeNode> parseTypeNode();
  std::unique_ptr<TypePath> parseTypePath();
  std::unique_ptr<TypeReference> parseTypeReference();
  std::unique_ptr<TypeArray> parseTypeArray();
  std::unique_ptr<TypeUnit> parseTypeUnit();
  
public:
  Parser(const std::vector<Token>& tokens): tokens(tokens), pos(0){}
  std::unique_ptr<Crate> parse();
};


#endif