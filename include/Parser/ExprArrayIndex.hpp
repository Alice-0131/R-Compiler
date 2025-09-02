#ifndef EXPRARRAYINDEX_HPP
#define EXPRARRAYINDEX_HPP
#include "ExprNode.hpp"

class ExprArray : public ExprWithoutBlockNode
{
public:
  void accept(ASTVisitor &visitor) = 0;
};

class ExprArrayExpand : public ExprArray
{
private:
  std::vector<std::unique_ptr<ExprNode>> elements;
public:
  ExprArrayExpand(std::vector<std::unique_ptr<ExprNode>> &&elements):
    elements(std::move(elements)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprArrayAbbreviate : public ExprArray
{
private:
  std::unique_ptr<ExprNode> value;
  std::unique_ptr<ExprNode> size;
public:
  ExprArrayAbbreviate(std::unique_ptr<ExprNode> value, std::unique_ptr<ExprNode> size):
    value(std::move(value)), size(std::move(size)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

class ExprIndex : public ExprWithoutBlockNode
{
private:
  std::unique_ptr<ExprNode> array;
  std::unique_ptr<ExprNode> index;
public:
  ExprIndex(std::unique_ptr<ExprNode> array, std::unique_ptr<ExprNode> index):
    array(std::move(array)), index(std::move(index)) {}
  void accept(ASTVisitor &visitor)override {visitor.visit(*this);}
};

#endif