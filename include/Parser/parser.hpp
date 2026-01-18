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
#include "../ASTNode/PatternNode.hpp"
#include "../ASTNode/PatternIdentifier.hpp"
#include "../ASTNode/PatternReference.hpp"
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

  std::shared_ptr<Path> parsePath();

  std::shared_ptr<ItemNode> parseItemNode();
  std::shared_ptr<ItemFn> parseItemFn();
  void parseItemFnParameters(FnParameters &function_parameters);
  bool parseItemFnSelfParam(SelfParam &self_param);
  void parseItemFnParams(std::vector<FnParam> &fn_params);
  std::shared_ptr<ItemStruct> parseItemStruct();
  void parseItemStructFields(std::vector<StructField> &struct_fields);
  std::shared_ptr<ItemEnum> parseItemEnum();
  std::shared_ptr<ItemConst> parseItemConst();
  std::shared_ptr<ItemTrait> parseItemTrait();
  std::shared_ptr<ItemImpl> parseItemImpl();
  std::shared_ptr<ItemAssociatedNode> parseItemAssociatedNode();

  std::shared_ptr<StmtNode> parseStmtNode();
  std::shared_ptr<StmtEmpty> parseStmtEmpty();
  std::shared_ptr<StmtItem> parseStmtItem();
  std::shared_ptr<StmtLet> parseStmtLet();
  std::shared_ptr<StmtExpr> parseStmtExpr();

  std::shared_ptr<ExprNode> parseExprNode(int ctxPrecedence = 0);
  std::shared_ptr<ExprNode> parseExprPrefix();
  std::shared_ptr<ExprNode> parseExprInfix(std::shared_ptr<ExprNode> &&left, const Token &token);
  std::shared_ptr<ExprLiteralNode> parseExprLiteralNode();
  std::shared_ptr<ExprLiteralChar> parseExprLiteralChar();
  std::shared_ptr<ExprLiteralString> parseExprLiteralString();
  std::shared_ptr<ExprLiteralInt> parseExprLiteralInt();
  std::shared_ptr<ExprLiteralBool> parseExprLiteralBool();
  std::shared_ptr<ExprPath> parseExprPath();
  std::shared_ptr<ExprBlock> parseExprBlock();
  ////std::shared_ptr<ExprOperatorNode> parseExprOperatorNode();
  std::shared_ptr<ExprOpUnary> parseExprOpUnary();
  std::shared_ptr<ExprOpBinary> parseExprOpBinary(std::shared_ptr<ExprNode> &&left, const Token &token);
  std::shared_ptr<ExprOpCast> parseExprOpCast(std::shared_ptr<ExprNode> &&left);
  std::shared_ptr<ExprGrouped> parseExprGrouped();
  std::shared_ptr<ExprArrayNode> parseExprArrayNode();
  //// std::shared_ptr<ExprArrayAbbreviate> parseExprArrayAbbreviate();
  //// std::shared_ptr<ExprArrayExpand> parseExprArrayExpand();
  std::shared_ptr<ExprIndex> parseExprIndex(std::shared_ptr<ExprNode> &&left);
  std::shared_ptr<ExprStruct> parseExprStruct(std::shared_ptr<ExprPath> &&left);
  void parseExprStructField(StructExprField &field);
  std::shared_ptr<ExprCall> parseExprCall(std::shared_ptr<ExprNode> &&left);
  std::shared_ptr<ExprNode> parseExprMethodAndField(std::shared_ptr<ExprNode> &&left);
  ////std::shared_ptr<ExprLoopNode> parseExprLoopNode();
  std::shared_ptr<ExprLoopInfinite> parseExprLoopInfinite();
  std::shared_ptr<ExprLoopPredicate> parseExprLoopPredicate();
  std::shared_ptr<ExprBreak> parseExprBreak();
  std::shared_ptr<ExprContinue> parseExprContinue();
  std::shared_ptr<ExprIf> parseExprIf();
  std::shared_ptr<ExprReturn> parseExprReturn();
  //std::shared_ptr<ExprUnderscore> parseExprUnderscore();

  std::shared_ptr<PatternNode> parsePatternNode();
  //std::shared_ptr<PatternLiteral> parsePatternLiteral();
  std::shared_ptr<PatternIdentifier> parsePatternIdentifier();
  //std::shared_ptr<PatternWildcard> parsePatternWildcard();
  std::shared_ptr<PatternReference> parsePatternReference();
  //std::shared_ptr<PatternPath> parsePatternPath();

  std::shared_ptr<TypeNode> parseTypeNode();
  std::shared_ptr<TypePath> parseTypePath();
  std::shared_ptr<TypeReference> parseTypeReference();
  std::shared_ptr<TypeArray> parseTypeArray();
  std::shared_ptr<TypeUnit> parseTypeUnit();

  void reportError(std::string msg);
  
public:
  Parser(const std::vector<Token>& tokens): tokens(tokens), pos(0){}
  std::shared_ptr<Crate> parse();
};


#endif