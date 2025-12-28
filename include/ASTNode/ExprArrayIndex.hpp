#ifndef EXPRARRAYINDEX_HPP
#define EXPRARRAYINDEX_HPP
#include "ExprNode.hpp"

class ExprArrayNode : public ExprWithoutBlockNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprArrayExpand : public ExprArrayNode
{
public:
  std::vector<std::shared_ptr<ExprNode>> elements;

  ExprArrayExpand(std::vector<std::shared_ptr<ExprNode>> &&elements):
    elements(std::move(elements)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprArrayAbbreviate : public ExprArrayNode
{
public:
  std::shared_ptr<ExprNode> value;
  std::shared_ptr<ExprNode> size;

  ExprArrayAbbreviate(std::shared_ptr<ExprNode> value, std::shared_ptr<ExprNode> size):
    value(std::move(value)), size(std::move(size)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprIndex : public ExprWithoutBlockNode
{
public:
  std::shared_ptr<ExprNode> array;
  std::shared_ptr<ExprNode> index;

  ExprIndex(std::shared_ptr<ExprNode> array, std::shared_ptr<ExprNode> index):
    array(std::move(array)), index(std::move(index)) {}
  void accept(ASTVisitor &visitor)override {visitor.visit(*this);}
};

#endif