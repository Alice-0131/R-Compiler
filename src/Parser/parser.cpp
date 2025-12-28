#include <memory>
#include <stdexcept>
#include <map>
#include "../../include/Parser/parser.hpp"

struct precedence{
  int left;
  int right;
};

std::map<tokenType, precedence> nudPrecedence = {
  {MINUS,   {0, 22}},
  {NOT,     {0, 22}},
  {STAR,    {0, 22}},
  {AND,     {0, 22}},
  {AND_AND, {0, 22}},
};

std::map<tokenType, precedence> ledPrecedence = {
  {DOT,        {23, 24}},
  {L_PAREN,    {22, 22}},
  {L_BRACE,    {22, 23}},
  {L_BRACKET,  {22, 23}},
  {AS,         {20, 21}},
  {STAR,       {18, 19}},
  {SLASH,      {18, 19}},
  {PERCENT,    {18, 19}},
  {PLUS,       {16, 17}},
  {MINUS,      {16, 17}},
  {SHL,        {14, 15}},
  {SHR,        {14, 15}},
  {AND,        {12, 13}},
  {CARET,      {10, 11}},
  {OR,         {8, 9}},
  {EQ_EQ,      {7, 7}}, 
  {NE,         {7, 7}},
  {LT,         {7, 7}},
  {GT,         {7, 7}},
  {LE,         {7, 7}},
  {GE,         {7, 7}},
  {AND_AND,    {5, 6}},
  {OR_OR,      {3, 4}},
  {EQ,         {2, 1}},
  {PLUS_EQ,    {2, 1}},
  {MINUS_EQ,   {2, 1}},
  {STAR_EQ,    {2, 1}},
  {SLASH_EQ,   {2, 1}},
  {PERCENT_EQ, {2, 1}},
  {AND_EQ,     {2, 1}},
  {OR_EQ,      {2, 1}}, 
  {CARET_EQ,   {2, 1}},
  {SHL_EQ,     {2, 1}},
  {SHR_EQ,     {2, 1}},
  {RETURN,     {0, 0}},
  {BREAK,      {0, 0}}
};

std::shared_ptr<Crate> Parser::parse() {
  std::vector<std::shared_ptr<ItemNode>> items;
  while (pos < tokens.size())
  {
    items.push_back(parseItemNode());
  }
  return std::make_shared<Crate>(std::move(items));
}

std::shared_ptr<Path> Parser::parsePath(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parsePath: out of range.");
  }
  switch (tokens[pos].type) {
  case IDENTIFIER: return std::make_shared<Path>(Identifier, tokens[pos++].str);
  case SELF_:      return std::make_shared<Path>(Self, tokens[pos++].str);
  case SELF:       return std::make_shared<Path>(self, tokens[pos++].str);
  default: throw std::runtime_error("parsePath: not match.");
  }
}

std::shared_ptr<ItemNode> Parser::parseItemNode() {
  if (pos + 1 >= tokens.size()) {
    throw std::runtime_error("parseItemNode: out of range.");
  }
  switch (tokens[pos].type)
  {
  case CONST:
    if (tokens[pos + 1].type == FN) {
      return parseItemFn();
    } else {
      return parseItemConst();
    }
    break;
  case FN:     return parseItemFn(); break;
  case STRUCT: return parseItemStruct(); break;
  case ENUM:   return parseItemEnum(); break;
  case TRAIT:  return parseItemTrait(); break;
  case IMPL:   return parseItemImpl(); break;
  default:
    throw std::runtime_error("parseItemNode: not match.");
    break;
  }
}

std::shared_ptr<ItemFn> Parser::parseItemFn() {
  bool is_const = false;
  std::string identifier;
  FnParameters function_parameters;
  std::shared_ptr<TypeNode> function_return_type = nullptr;
  std::shared_ptr<ExprBlock> block_expr = nullptr;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemFn: out of range.");
  }
  if (tokens[pos].type == CONST) {
    is_const = true;
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemFn: out of range.");
    }
  }
  if (tokens[pos++].type != FN) {
    throw std::runtime_error("parseItemFn: not match.");
  }
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemFn: not match.");
  }
  identifier = tokens[pos++].str;
  if (pos >= tokens.size() || tokens[pos++].type != L_PAREN) {
    throw std::runtime_error("parseItemFn: not match.");
  }
  parseItemFnParameters(function_parameters);
  if (pos >= tokens.size() || tokens[pos++].type != R_PAREN) {
    throw std::runtime_error("parseItemFn: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemFn: out of range.");
  }
  if (tokens[pos].type == R_ARROW) {
    ++pos;
    function_return_type = parseTypeNode();
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemFn: out of range.");
    }
  }
  if (tokens[pos].type == SEMI) {
    ++pos;
  } else {
    block_expr = parseExprBlock();
  }
  return std::make_shared<ItemFn>(is_const, identifier, std::move(function_parameters), 
    std::move(function_return_type), std::move(block_expr));
}

void Parser::parseItemFnParameters(FnParameters &function_parameters) {
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemParameters: out of range.");
  }
  if (tokens[pos].type == R_PAREN) {
    return;
  }
  int tmp = pos;
  try{
    if (parseItemFnSelfParam(function_parameters.self_param)) {
      return;
    }
    parseItemFnParams(function_parameters.fn_params);
    return;
  } catch(...) {
    pos = tmp;
    parseItemFnParams(function_parameters.fn_params);
    return;
  }
}

bool Parser::parseItemFnSelfParam(SelfParam &self_param){
  if (tokens[pos].type == SELF) {
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    // if (tokens[pos].type == COLON) {
    //   self_param.flag = 2;
    //   ++pos;
    //   self_param.typed_self.type = parseTypeNode();
    // } else {
    //   self_param.flag = 1;
    // }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
  } else if (tokens[pos].type == AND) {
    ++pos;
    //self_param.flag = 1;
    self_param.is_and = true;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == MUT) {
      ++pos;
      self_param.is_mut = true;
      if (pos >= tokens.size()) {
        throw std::runtime_error("parseItemParameters: out of range.");
      }
    }
    if (tokens[pos++].type != SELF) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
  } else if (tokens[pos].type == MUT) {
    ++pos;
    if (pos >= tokens.size() || tokens[pos++].type != SELF) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    // if (tokens[pos].type == COLON) {
    //   self_param.flag = 2;
    //   ++pos;
    //   self_param.typed_self.type = parseTypeNode();
    // } else {
    //   self_param.flag = 1;
    // }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return true;
    }
  }
  return false;
}

void Parser::parseItemFnParams(std::vector<FnParam> &fn_params){
  FnParam fn_param;
  fn_param.pattern = parsePatternNode();
  if (pos >= tokens.size() || tokens[pos].type != COLON) {
    throw std::runtime_error("parseItemParameters: not match.");
  }
  fn_param.type = parseTypeNode();
  fn_params.push_back(std::move(fn_param));
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return;
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemParameters: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return;
    }
    FnParam fn_param;
    fn_param.pattern = parsePatternNode();
    if (pos >= tokens.size() || tokens[pos].type != COLON) {
      throw std::runtime_error("parseItemParameters: not match.");
    }
    fn_param.type = parseTypeNode();
    fn_params.push_back(std::move(fn_param));
  }
}

std::shared_ptr<ItemStruct> Parser::parseItemStruct() {
  std::string identifier;
  std::vector<StructField> struct_fields;
  if (pos >= tokens.size() || tokens[pos++].type != STRUCT) {
    throw std::runtime_error("parseItemStruct: not match.");
  }
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemStruct: not match.");
  }
  identifier = tokens[pos++].str;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemStruct: out of range.");
  }
  switch (tokens[pos++].type) 
  {
  case L_BRACE: 
    parseItemStructFields(struct_fields);
    if (pos >= tokens.size() || tokens[pos++].type != R_BRACE) {
      throw std::runtime_error("parseItemStruct: not match.");
    }
  case SEMI: return std::make_shared<ItemStruct>(identifier, std::move(struct_fields));
  default:   throw std::runtime_error("parseItemStruct: not match.");
  }
}

void Parser::parseItemStructFields(std::vector<StructField> &struct_fields) {
  StructField struct_field;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemStructFields: out of range.");
  }
  if (tokens[pos].type == R_BRACE) {
    return;
  }
  if (tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemStructFields: not match.");
  }
  struct_field.identifier = tokens[pos++].str;
  if (pos >= tokens.size() || tokens[pos++].type != COLON) {
    throw std::runtime_error("parseItemStructFields: not match.");
  }
  struct_field.type = parseTypeNode();
  struct_fields.push_back(std::move(struct_field));
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemStructFields: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      return;
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemStructFields: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemStructFields: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return;
    }
    StructField struct_field;
    if (tokens[pos].type != IDENTIFIER) {
      throw std::runtime_error("parseItemStructFields: not match.");
    }
    struct_field.identifier = tokens[pos++].str;
    if (pos >= tokens.size() || tokens[pos++].type != COLON) {
      throw std::runtime_error("parseItemStructFields: not match.");
    }
    struct_field.type = parseTypeNode();
    struct_fields.push_back(std::move(struct_field));
  }
}

std::shared_ptr<ItemEnum> Parser::parseItemEnum() {
  std::string identifier;
  std::vector<std::string> enum_variants;
  if (pos >= tokens.size() || tokens[pos++].type != ENUM) {
    throw std::runtime_error("parseItemEnum: not match.");
  }
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemEnum: not match.");
  }
  identifier = tokens[pos++].str;
  if (pos >= tokens.size() || tokens[pos++].type != L_BRACE) {
    throw std::runtime_error("parseItemEnum: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemEnum: out of range.");
  }
  if (tokens[pos].type == R_BRACE) {
    ++pos;
    return std::make_shared<ItemEnum>(identifier, enum_variants);
  }
  if (tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemEnum: not match.");
  }
  enum_variants.push_back(tokens[pos++].str);
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemEnum: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      return std::make_shared<ItemEnum>(identifier, enum_variants);
    }
    if (tokens[pos++].type != COMMA) {
      throw std::runtime_error("parseItemEnum: not match.");
    }
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemEnum: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return std::make_shared<ItemEnum>(identifier, enum_variants);
    }
    if (tokens[pos].type != IDENTIFIER) {
      throw std::runtime_error("parseItemEnum: not match.");
    }
    enum_variants.push_back(tokens[pos++].str);
  }
  
}

std::shared_ptr<ItemConst> Parser::parseItemConst() {
  std::string identifier;
  std::shared_ptr<TypeNode> type = nullptr;
  std::shared_ptr<ExprNode> expr = nullptr;
  if (pos >= tokens.size() || tokens[pos++].type != CONST) {
    throw std::runtime_error("parseItemConst: not match.");
  }
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemConst: not match.");
  }
  identifier = tokens[pos++].str;
  if (pos >= tokens.size() || tokens[pos++].type != COLON) {
    throw std::runtime_error("parseItemConst: not match.");
  }
  type = parseTypeNode();
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemConst: out of range.");
  }
  if (tokens[pos].type == EQ) {
    ++pos;
    expr = parseExprNode();
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemConst: out of range.");
    }
  }
  if (tokens[pos++].type != SEMI) {
    throw std::runtime_error("parseItemConst: not match.");
  }
  return std::make_shared<ItemConst>(identifier, std::move(type), std::move(expr));
}

std::shared_ptr<ItemTrait> Parser::parseItemTrait() {
  std::string identifier;
  std::vector<std::shared_ptr<ItemAssociatedNode>> associated_items;
  if (pos >= tokens.size() || tokens[pos++].type != TRAIT) {
    throw std::runtime_error("parseItemTrait: not match.");
  }
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseItemTrait: not match.");
  }
  identifier = tokens[pos++].str;
  if (pos >= tokens.size() || tokens[pos++].type != L_BRACE) {
    throw std::runtime_error("parseItemTrait: not match.");
  }
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemTrait: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      ++pos;
      return std::make_shared<ItemTrait>(identifier, std::move(associated_items));
    }
    associated_items.push_back(parseItemAssociatedNode());
  }
}

std::shared_ptr<ItemImpl> Parser::parseItemImpl() {
  std::string identifier;
  std::shared_ptr<TypeNode> type = nullptr;
  std::vector<std::shared_ptr<ItemAssociatedNode>> associated_items;
  if (pos >= tokens.size() || tokens[pos++].type != IMPL) {
    throw std::runtime_error("parseItemImpl: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseItemImpl: out of range.");
  }
  if (tokens[pos].type == IDENTIFIER) {
    identifier = tokens[pos++].str;
    if (pos >= tokens.size() || tokens[pos++].type != FOR) {
      throw std::runtime_error("parseItemImpl: not match.");
    }
  }
  type = parseTypeNode();
  if (pos >= tokens.size() || tokens[pos++].type != L_BRACE) {
    throw std::runtime_error("parseItemTrait: not match.");
  }
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseItemTrait: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      ++pos;
      return std::make_shared<ItemImpl>(identifier, std::move(type), std::move(associated_items));
    }
    associated_items.push_back(parseItemAssociatedNode());
  }
}

std::shared_ptr<ItemAssociatedNode> Parser::parseItemAssociatedNode(){
  if (pos >= tokens.size()){
    throw std::runtime_error("parseItemAssociatedNode: out of range.");
  }
  switch (tokens[pos].type) 
  {
  case CONST:
    if (pos + 1 >= tokens.size()){
      throw std::runtime_error("parseItemAssociatedNode: out of range.");
    }
    if (tokens[pos + 1].type == FN) {
      return parseItemFn();
    } else {
      return parseItemConst();
    }
  case FN: return parseItemFn();
  default: throw std::runtime_error("parseItemAssociatedNode: out of range.");
  }
}

std::shared_ptr<StmtNode> Parser::parseStmtNode(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseStmtNode: out of range.");
  }
  switch (tokens[pos].type) 
  {
  case SEMI:   return parseStmtEmpty();
  case FN:
  case STRUCT:
  case ENUM:
  case CONST:
  case TRAIT:
  case IMPL:   return parseStmtItem();
  case LET:    return parseStmtLet();
  default:     return parseStmtExpr();
  }
}

std::shared_ptr<StmtEmpty> Parser::parseStmtEmpty(){
  if (pos >= tokens.size() || tokens[pos].type != SEMI) {
    throw std::runtime_error("parseStmtEmpty: not match.");
  }
  return std::make_shared<StmtEmpty>();
}

std::shared_ptr<StmtItem> Parser::parseStmtItem(){
  std::shared_ptr<ItemNode> item = parseItemNode();
  return std::make_shared<StmtItem>(std::move(item));
}

std::shared_ptr<StmtLet> Parser::parseStmtLet(){
  std::shared_ptr<PatternNode> pattern = nullptr;
  std::shared_ptr<TypeNode> type = nullptr;
  std::shared_ptr<ExprNode> expr = nullptr;
  if (pos >= tokens.size() || tokens[pos++].type != LET) {
    throw std::runtime_error("parseStmtLet: not match.");
  }
  pattern = parsePatternNode();
  if (pos >= tokens.size() || tokens[pos++].type != COLON) {
    throw std::runtime_error("parseStmtLet: not match.");
  }
  type = parseTypeNode();
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseStmtLet: out of range.");
  }
  if (tokens[pos].type == EQ) {
    ++pos;
    expr = parseExprNode();
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseStmtLet: out of range.");
    }
  }
  if (tokens[pos++].type != SEMI) {
    throw std::runtime_error("parseStmtLet: not match.");
  }
  return std::make_shared<StmtLet>(std::move(pattern), std::move(type), std::move(expr));
}

std::shared_ptr<StmtExpr> Parser::parseStmtExpr(){
  std::shared_ptr<ExprNode> expr = parseExprNode();
  if (pos >= tokens.size() || tokens[pos].type != SEMI) {
    if (dynamic_cast<ExprWithBlockNode*>(expr.get())) {
      return std::make_shared<StmtExpr>(std::move(expr));
    } else {
      throw std::runtime_error("parseStmtExpr: not match.");
    }
  } else {
    ++pos;
    return std::make_shared<StmtExpr>(std::move(expr));
  }
}

std::shared_ptr<ExprNode> Parser::parseExprNode(int ctxPrecedence){
  auto left = parseExprPrefix();
  while (true) {
    if (pos >= tokens.size()) break;
    const auto token = tokens[pos];
    if (ledPrecedence.find(token.type) == ledPrecedence.end()) break;
    if (ledPrecedence[token.type].left <= ctxPrecedence) break;
    ++pos;
    left = parseExprInfix(std::move(left), token);
  }
  return left;
}

std::shared_ptr<ExprNode> Parser::parseExprPrefix(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprPrefix: out of range.");
  }
  auto token = tokens[pos];
  switch (token.type) {
  case CHAR_LITERAL:        return parseExprLiteralChar();
  case STRING_LITERAL:
  case RAW_STRING_LITERAL:
  case CSTRING_LITERAL:
  case RAW_CSTRING_LITERAL: return parseExprLiteralString();
  case INTEGER_LITERAL:     return parseExprLiteralInt();
  case TRUE:
  case FALSE:               return parseExprLiteralBool();
  case IDENTIFIER:
  case SELF:
  case SELF_:               return parseExprPath();
  case L_BRACE:             return parseExprBlock();
  case AND:
  case AND_AND:
  case STAR:
  case MINUS:
  case NOT:                 return parseExprOpUnary();
  case L_PAREN:             return parseExprGrouped();
  case L_BRACKET:           return parseExprArrayNode();
  case LOOP:                return parseExprLoopInfinite();
  case WHILE:               return parseExprLoopPredicate();
  case BREAK:               return parseExprBreak();
  case CONTINUE:            return parseExprContinue();
  case IF:                  return parseExprIf();
  case RETURN:              return parseExprReturn();
  //case UNDERSCORE:          return parseExprUnderscore();
  default: throw std::runtime_error("parseExprPrefix: not match.");
  }
}

std::shared_ptr<ExprNode> Parser::parseExprInfix(std::shared_ptr<ExprNode> &&left, const Token &token){
  switch (token.type) {
  case PLUS:
  case MINUS:
  case STAR:
  case SLASH:
  case PERCENT:
  case AND:
  case OR:
  case CARET:
  case SHL:
  case SHR:
  case EQ_EQ:
  case NE:
  case GT:
  case LT:
  case GE:
  case LE:
  case OR_OR:
  case AND_AND:
  case EQ:
  case PLUS_EQ:
  case MINUS_EQ:
  case STAR_EQ:
  case SLASH_EQ:
  case PERCENT_EQ:
  case AND_EQ:
  case OR_EQ:
  case CARET_EQ:
  case SHL_EQ:
  case SHR_EQ:    return parseExprOpBinary(std::move(left), token);
  case AS:        return parseExprOpCast(std::move(left));
  case L_BRACKET: return parseExprIndex(std::move(left));
  case L_BRACE: {
    if (dynamic_cast<ExprPath*>(left.get())) {
      return parseExprStruct(std::shared_ptr<ExprPath>(dynamic_cast<ExprPath*>(left.get())));
    } else {
      throw std::runtime_error("parseExprInfix: not match.");
    }
  }
  case L_PAREN:   return parseExprCall(std::move(left));
  case DOT:       return parseExprMethodAndField(std::move(left));
  default: throw std::runtime_error("parseExprInfix: not match.");
  }
}

std::shared_ptr<ExprLiteralNode> Parser::parseExprLiteralNode(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprPrefix: out of range.");
  }
  auto token = tokens[pos];
  switch (token.type) {
  case CHAR_LITERAL:        return parseExprLiteralChar();
  case STRING_LITERAL:
  case RAW_STRING_LITERAL:
  case CSTRING_LITERAL:
  case RAW_CSTRING_LITERAL: return parseExprLiteralString();
  case INTEGER_LITERAL:     return parseExprLiteralInt();
  case TRUE:
  case FALSE:               return parseExprLiteralBool();
  default: throw std::runtime_error("parseExprLiteralNode: not match.");
  }
}

std::shared_ptr<ExprLiteralChar> Parser::parseExprLiteralChar(){
  return std::make_shared<ExprLiteralChar>(tokens[pos++].str[0]);
}

std::shared_ptr<ExprLiteralString> Parser::parseExprLiteralString(){
  return std::make_shared<ExprLiteralString>(tokens[pos++].str);
}

std::shared_ptr<ExprLiteralInt> Parser::parseExprLiteralInt(){
  return std::make_shared<ExprLiteralInt>(std::stoi(tokens[pos++].str));
}

std::shared_ptr<ExprLiteralBool> Parser::parseExprLiteralBool(){
  if (tokens[pos++].type == TRUE) {
    return std::make_shared<ExprLiteralBool>(true);
  } else {
    return std::make_shared<ExprLiteralBool>(false);
  }
}

std::shared_ptr<ExprPath> Parser::parseExprPath(){
  std::shared_ptr<Path> path1 = nullptr;
  std::shared_ptr<Path> path2 = nullptr;
  path1 = parsePath();
  if (pos >= tokens.size() || tokens[pos].type != PATH_SEP) {
    return std::make_shared<ExprPath>(std::move(path1), std::move(path2));
  }
  ++pos;
  path2 = parsePath();
  return std::make_shared<ExprPath>(std::move(path1), std::move(path2));
}

std::shared_ptr<ExprBlock> Parser::parseExprBlock(){
  std::vector<std::shared_ptr<StmtNode>> stmts;
  std::shared_ptr<ExprWithoutBlockNode> expr = nullptr;
  if (tokens[pos++].type == R_BRACE) {
    return std::make_shared<ExprBlock>(std::move(stmts), std::move(expr));
  }
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprBlock: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      ++pos;
      return std::make_shared<ExprBlock>(std::move(stmts), std::move(expr));
    }
    int tmp = pos;
    try {
      stmts.push_back(parseStmtNode());
    } catch (...) {
      pos = tmp;
      break;
    }
  }
  auto e = parseExprNode();
  if (dynamic_cast<ExprWithoutBlockNode*>(e.get())) {
    return std::make_shared<ExprBlock>(std::move(stmts), 
      std::shared_ptr<ExprWithoutBlockNode>(dynamic_cast<ExprWithoutBlockNode*>(e.get())));
  } else {
    throw std::runtime_error("parseExprBlock: not match.");
  }
}

std::shared_ptr<ExprOpUnary> Parser::parseExprOpUnary(){
  ExprOpUnaryType type;
  std::shared_ptr<ExprNode> expr;
  switch (tokens[pos].type) {
  case AND:
  case AND_AND:{
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprOpUnary: out of range.");
    }
    if (tokens[pos].type == MUT) {
      ++pos;
      type = MUT_BORROW_;
    } else {
      type = BORROW_;
    }
    break;
  }
  case STAR: {
    ++pos;
    type = DEREFERENCE_;
    break;
  }
  case MINUS: {
    ++pos;
    type = NEGATE_;
    break;
  }
  case NOT:{
    ++pos;
    type = NOT_;
    break;
  }
  default: throw std::runtime_error("parseExprOpUnary: not match.");
  }
  expr = parseExprNode(nudPrecedence[tokens[pos].type].right);
  return std::make_shared<ExprOpUnary>(type, std::move(expr));
}

std::shared_ptr<ExprOpBinary> Parser::parseExprOpBinary(std::shared_ptr<ExprNode> &&left, const Token &token){
  ExprOpBinaryType type;
  std::shared_ptr<ExprNode> right;
  switch (token.type) {
  case PLUS:       type = PLUS_; break;
  case MINUS:      type = MINUS_; break;
  case STAR:       type = MUL_; break;
  case SLASH:      type = DIV_; break;
  case PERCENT:    type = MOD_; break;
  case AND:        type = AND_; break;
  case OR:         type = OR_; break;
  case CARET:      type = XOR_; break;
  case SHL:        type = SHL_; break;
  case SHR:        type = SHR_; break;
  case EQ_EQ:      type = EQUAL_; break;
  case NE:         type = NOT_EQUAL_; break;
  case GT:         type = GREATER_; break;
  case LT:         type = LESS_; break;
  case GE:         type = GREATER_EQUAL_; break;
  case LE:         type = LESS_EQUAL_; break;
  case OR_OR:      type = OR_OR_; break;
  case AND_AND:    type = AND_AND_; break;
  case EQ:         type = ASSIGN_; break;
  case PLUS_EQ:    type = PLUS_EQ_; break;
  case MINUS_EQ:   type = MINUS_EQ_; break;
  case STAR_EQ:    type = MUL_EQ_; break;
  case SLASH_EQ:   type = DIV_EQ_; break;
  case PERCENT_EQ: type = MOD_EQ_; break;
  case AND_EQ:     type = AND_EQ_; break;
  case OR_EQ:      type = OR_EQ_; break;
  case CARET_EQ:   type = XOR_EQ_; break;
  case SHL_EQ:     type = SHL_EQ_; break;
  case SHR_EQ:     type = SHR_EQ_; break;
  default: throw std::runtime_error("parseExprOpBinary: not match.");
  }
  right = parseExprNode(ledPrecedence[token.type].right);
  return std::make_shared<ExprOpBinary>(type, std::move(left), std::move(right));
}

std::shared_ptr<ExprOpCast> Parser::parseExprOpCast(std::shared_ptr<ExprNode> &&left){
  std::shared_ptr<TypeNode> type = parseTypeNode();
  return std::make_shared<ExprOpCast>(std::move(left), std::move(type));
}

std::shared_ptr<ExprGrouped> Parser::parseExprGrouped(){
  ++pos;
  std::shared_ptr<ExprNode> expr = parseExprNode();
  if (pos >= tokens.size() || tokens[pos++].type != R_PAREN) {
    throw std::runtime_error("parseExprGrouped: not match.");
  }
  return std::make_shared<ExprGrouped>(std::move(expr));
}

std::shared_ptr<ExprArrayNode> Parser::parseExprArrayNode(){
  ++pos;
  std::shared_ptr<ExprNode> expr = parseExprNode();
  if (pos < tokens.size() && tokens[pos].type == SEMI) {
    ++pos;
    std::shared_ptr<ExprNode> size = parseExprNode();
    if (pos >= tokens.size() || tokens[pos].type != R_BRACKET) {
      throw std::runtime_error("parseExprArrayNode: not match.");
    }
    return std::make_shared<ExprArrayAbbreviate>(std::move(expr), std::move(size));
  }
  std::vector<std::shared_ptr<ExprNode>> elements;
  elements.push_back(std::move(expr));
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseEXprArrayNode: out of range.");
    }
    if (tokens[pos].type == R_BRACKET) {
      ++pos;
      return std::make_shared<ExprArrayExpand>(std::move(elements));
    }
    if (tokens[pos].type != COMMA) {
      throw std::runtime_error("parseExprArrayNode: not match");
    }
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseEXprArrayNode: out of range.");
    }
    if (tokens[pos].type == R_BRACKET) {
      ++pos;
      return std::make_shared<ExprArrayExpand>(std::move(elements));
    }
    std::shared_ptr<ExprNode> expr = parseExprNode();
    elements.push_back(std::move(expr));
  }
}

std::shared_ptr<ExprIndex> Parser::parseExprIndex(std::shared_ptr<ExprNode> &&left){
  std::shared_ptr<ExprNode> index = parseExprNode();
  if (pos >= tokens.size() || tokens[pos].type != R_BRACKET) {
    throw std::runtime_error("parseExprIndex: not match.");
  }
  ++pos;
  return std::make_shared<ExprIndex>(std::move(left), std::move(index));
}

std::shared_ptr<ExprStruct> Parser::parseExprStruct(std::shared_ptr<ExprPath> &&left){
  std::vector<StructExprField> fields;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprStruct: out of range.");
  }
  if (tokens[pos].type == R_BRACE) {
    ++pos;
    return std::make_shared<ExprStruct>(std::move(left), std::move(fields));
  }
  StructExprField field;
  parseExprStructField(field);
  fields.push_back(std::move(field));
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprStruct: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      ++pos;
      return std::make_shared<ExprStruct>(std::move(left), std::move(fields));
    }
    if (tokens[pos].type != COMMA) {
      throw std::runtime_error("parseExprStruct: not match.");
    }
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprStruct: out of range.");
    }
    if (tokens[pos].type == R_BRACE) {
      ++pos;
      return std::make_shared<ExprStruct>(std::move(left), std::move(fields));
    }
    StructExprField field;
    parseExprStructField(field);
    fields.push_back(std::move(field));
  }
}

void Parser::parseExprStructField(StructExprField &field){
  if (pos >= tokens.size() || tokens[pos].type != IDENTIFIER) {
    throw std::runtime_error("parseExprStructField: not match.");
  }
  field.identifier = tokens[pos++].str;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprStructField: out of range.");
  }
  if (tokens[pos].type == COLON) {
    ++pos;
    field.expr = parseExprNode();
  }
}

std::shared_ptr<ExprCall> Parser::parseExprCall(std::shared_ptr<ExprNode> &&left){
  std::vector<std::shared_ptr<ExprNode>> params;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprCall: out of range.");
  }
  if (tokens[pos].type == R_PAREN) {
    return std::make_shared<ExprCall>(std::move(left), std::move(params));
  }
  params.push_back(parseExprNode());
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprCall: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      ++pos;
      return std::make_shared<ExprCall>(std::move(left), std::move(params));
    }
    if (tokens[pos].type != COMMA) {
      throw std::runtime_error("parseExprCall: not match.");
    }
    ++pos;
    params.push_back(parseExprNode());
  }
}

std::shared_ptr<ExprNode> Parser::parseExprMethodAndField(std::shared_ptr<ExprNode> &&left){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprMethod: out of range.");
  }
  if (tokens[pos].type == IDENTIFIER) {
    if (pos + 1 >= tokens.size() || tokens[pos + 1].type != L_PAREN) {
      return std::make_shared<ExprField>(std::move(left), tokens[pos++].str);
    }
  }
  std::shared_ptr<Path> path = parsePath();
  if (pos >= tokens.size() || tokens[pos++].type != L_PAREN) {
    throw std::runtime_error("parseExprMethod: not match.");
  }
  std::vector<std::shared_ptr<ExprNode>> params;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprCall: out of range.");
  }
  if (tokens[pos].type == R_PAREN) {
    return std::make_shared<ExprMethodCall>(std::move(left), std::move(path), std::move(params));
  }
  params.push_back(parseExprNode());
  while (true) {
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprCall: out of range.");
    }
    if (tokens[pos].type == R_PAREN) {
      return std::make_shared<ExprMethodCall>(std::move(left), std::move(path), std::move(params));
    }
    if (tokens[pos].type != COMMA) {
      throw std::runtime_error("parseExprCall: not match.");
    }
    ++pos;
    params.push_back(parseExprNode());
  }
}

std::shared_ptr<ExprLoopInfinite> Parser::parseExprLoopInfinite(){
  ++pos;
  return std::make_shared<ExprLoopInfinite>(parseExprBlock());
}

std::shared_ptr<ExprLoopPredicate> Parser::parseExprLoopPredicate(){
  std::shared_ptr<ExprNode> condition = nullptr;
  std::shared_ptr<ExprBlock> block = nullptr;
  ++pos;
  if (pos >= tokens.size() || tokens[pos++].type != L_PAREN) {
    throw std::runtime_error("parseExprLoopPredicate: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprLoopPredicate: out of range.");
  }
  if (tokens[pos].type != R_PAREN) {
    condition = parseExprNode();
  }
  if (tokens[pos++].type != R_PAREN) {
    throw std::runtime_error("parseExprLoopPredicate: not match.");
  }
  block = parseExprBlock();
  return std::make_shared<ExprLoopPredicate>(std::move(condition), std::move(block));
}

std::shared_ptr<ExprBreak> Parser::parseExprBreak(){
  std::shared_ptr<ExprNode> expr = nullptr;
  ++pos;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprBreak: out of range.");
  }
  if (tokens[pos].type == SEMI) {
    return std::make_shared<ExprBreak>(std::move(expr));
  }
  expr = parseExprNode();
  return std::make_shared<ExprBreak>(std::move(expr));
}

std::shared_ptr<ExprContinue> Parser::parseExprContinue(){
  ++pos;
  return std::make_shared<ExprContinue>();
}

std::shared_ptr<ExprIf> Parser::parseExprIf(){
  std::shared_ptr<ExprNode> condition = nullptr;
  std::shared_ptr<ExprBlock> if_block = nullptr;
  std::shared_ptr<ExprNode> else_block = nullptr;
  ++pos;
  if (pos >= tokens.size() || tokens[pos++].type != L_PAREN) {
    throw std::runtime_error("parseExprIf: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprIf: out of range.");
  }
  if (tokens[pos].type != R_PAREN) {
    condition = parseExprNode();
  }
  if (tokens[pos++].type != R_PAREN) {
    throw std::runtime_error("parseExprIf: not match.");
  }
  if_block = parseExprBlock();
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprIf: out of range.");
  }
  if (tokens[pos].type == ELSE) {
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parseExprIf: out of range.");
    }
    if (tokens[pos].type == L_PAREN) {
      else_block = parseExprBlock();
    } else if (tokens[pos].type == IF) {
      else_block = parseExprIf();
    } else {
      throw std::runtime_error("parseExprIf: not match.");
    }
  }
  return std::make_shared<ExprIf>(std::move(condition), std::move(if_block), std::move(else_block));
}

std::shared_ptr<ExprReturn> Parser::parseExprReturn(){
  std::shared_ptr<ExprNode> expr = nullptr;
  ++pos;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseExprReturn: out of range.");
  }
  if (tokens[pos].type == SEMI) {
    return std::make_shared<ExprReturn>(std::move(expr));
  }
  expr = parseExprNode();
  return std::make_shared<ExprReturn>(std::move(expr));
}

// std::shared_ptr<ExprUnderscore> Parser::parseExprUnderscore(){
//   ++pos;
//   return std::make_shared<ExprUnderscore>();
// }

std::shared_ptr<PatternNode> Parser::parsePatternNode(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parsePatternNode: out of range.");
  }
  switch (tokens[pos].type) {
  // case MINUS:
  // case CHAR_LITERAL:
  // case STRING_LITERAL:
  // case RAW_STRING_LITERAL:
  // case CSTRING_LITERAL:
  // case RAW_CSTRING_LITERAL: 
  // case INTEGER_LITERAL: 
  // case TRUE:
  // case FALSE:               return parsePatternLiteral();
  case REF:
  case MUT:        return parsePatternIdentifier();
  case IDENTIFIER: return parsePatternIdentifier();
  //case UNDERSCORE:          return parsePatternWildcard();
  case AND:
  case AND_AND:    return parsePatternReference();
  // case SELF:
  // case SELF_:               return parsePatternPath();
  default: throw std::runtime_error("parsePatternNode: not match");
  }
}
// std::shared_ptr<PatternLiteral> Parser::parsePatternLiteral(){
//   bool is_minus = false;
//   std::shared_ptr<ExprLiteralNode> pattern = nullptr;
//   if (tokens[pos].type == MINUS) {
//     is_minus = true;
//     ++pos;
//   }
//   pattern = parseExprLiteralNode();
//   return std::make_shared<PatternLiteral>(is_minus, std::move(pattern));
// }

std::shared_ptr<PatternIdentifier> Parser::parsePatternIdentifier(){
  bool is_ref = false;
  bool is_mut = false;
  std::string identifier;
  std::shared_ptr<PatternNode> pattern = nullptr;
  if (pos >= tokens.size()) {
    throw std::runtime_error("parsePatternIdentifier: out of range.");
  }
  if (tokens[pos].type == REF) {
    is_ref = true;
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parsePatternIdentifier: out of range.");
    }
  }
  if (tokens[pos].type == MUT) {
    is_mut = true;
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parsePatternIdentifier: out of range.");
    }
  }
  identifier = tokens[pos++].str;
  return std::make_shared<PatternIdentifier>(is_ref, is_mut, identifier);
}

// std::shared_ptr<PatternWildcard> Parser::parsePatternWildcard(){
//   ++pos;
//   return std::make_shared<PatternWildcard>();
// }

std::shared_ptr<PatternReference> Parser::parsePatternReference(){
  bool is_and;
  bool is_mut = false;
  std::shared_ptr<PatternNode> pattern = nullptr;
  switch (tokens[pos++].type) {
  case AND: is_and = true; break;
  case AND_AND: is_and = false; break;
  default: throw std::runtime_error("parsePatternReference: not match.");
  }
  if (pos >= tokens.size()) {
    throw std::runtime_error("parsePatternReference: out of range.");
  }
  if (tokens[pos].type == MUT) {
    is_mut = true;
    ++pos;
    if (pos >= tokens.size()) {
      throw std::runtime_error("parsePatternReference: out of range.");
    }
  }
  pattern = parsePatternNode();
  return std::make_shared<PatternReference>(is_and, is_mut, std::move(pattern));
}

// std::shared_ptr<PatternPath> Parser::parsePatternPath(){
//   std::shared_ptr<ExprPath> expr = parseExprPath();
//   return std::make_shared<PatternPath>(std::move(expr));
// }

std::shared_ptr<TypeNode> Parser::parseTypeNode(){
  if (pos >= tokens.size()) {
    throw std::runtime_error("parseTypeNode: out of range.");
  }
  switch (tokens[pos].type) {
  case IDENTIFIER:
  case SELF:
  case SELF_:     return parseTypePath();
  case AND:       return parseTypeReference();
  case L_BRACKET: return parseTypeArray();
  case L_PAREN:   return parseTypeUnit();
  default: throw std::runtime_error("parseTypeNode: not match.");
  }
}

std::shared_ptr<TypePath> Parser::parseTypePath(){
  std::shared_ptr<Path> path = parsePath();
  return std::make_shared<TypePath>(std::move(path));
}

std::shared_ptr<TypeReference> Parser::parseTypeReference(){
  bool is_mut = false;
  std::shared_ptr<TypeNode> type = nullptr;
  if (pos >= tokens.size() || tokens[pos++].type != AND){
    throw std::runtime_error("parseTypeReference: not match.");
  }
  if (pos >= tokens.size()){
    throw std::runtime_error("parseTypeReference: out of range.");
  }
  if (tokens[pos].type == MUT) {
    ++pos;
    is_mut = true;
    if (pos >= tokens.size()){
      throw std::runtime_error("parseTypeReference: out of range.");
    }
  }
  type = parseTypeNode();
  return std::make_shared<TypeReference>(is_mut, std::move(type));
}

std::shared_ptr<TypeArray> Parser::parseTypeArray(){
  std::shared_ptr<TypeNode> type = nullptr;
  std::shared_ptr<ExprNode> expr = nullptr;
  if (pos >= tokens.size() || tokens[pos++].type != L_BRACKET){
    throw std::runtime_error("parseTypeArray: not match.");
  }
  type = parseTypeNode();
  if (pos >= tokens.size() || tokens[pos++].type != COLON){
    throw std::runtime_error("parseTypeArray: not match.");
  }
  expr = parseExprNode();
  if (pos >= tokens.size() || tokens[pos++].type != R_BRACKET){
    throw std::runtime_error("parseTypeArray: not match.");
  }
  return std::make_shared<TypeArray>(std::move(type), std::move(expr));
}

std::shared_ptr<TypeUnit> Parser::parseTypeUnit(){
  if (pos >= tokens.size() || tokens[pos++].type != L_PAREN){
    throw std::runtime_error("parseTypeUnit: not match.");
  }
  if (pos >= tokens.size() || tokens[pos++].type != R_PAREN){
    throw std::runtime_error("parseTypeUnit: not match.");
  }
  return std::make_shared<TypeUnit>();
}