#ifndef EXPRSTRUCT_HPP
#define EXPRSTRUCT_HPP
#include "ExprNode.hpp"
class ExprPath;

struct StructExprField
{
  std::string identifier;
  std::unique_ptr<ExprNode> expr;
};


class ExprStruct : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprPath> path;
  std::vector<StructExprField> fields;
public:
  ExprStruct(std::unique_ptr<ExprPath> path, std::vector<StructExprField> &fields):
    path(std::move(path)), fields(fields){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif