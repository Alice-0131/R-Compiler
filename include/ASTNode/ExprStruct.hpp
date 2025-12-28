#ifndef EXPRSTRUCT_HPP
#define EXPRSTRUCT_HPP
#include "ExprNode.hpp"
class ExprPath;

struct StructExprField
{
  std::string identifier;
  std::shared_ptr<ExprNode> expr;
};


class ExprStruct : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprPath> path;
  std::vector<StructExprField> fields;

  ExprStruct(std::shared_ptr<ExprPath> path, std::vector<StructExprField> fields):
    path(std::move(path)), fields(std::move(fields)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif