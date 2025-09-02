#ifndef EXPRPATH_HPP
#define EXPRPATH_HPP
#include "ExprNode.hpp"
class Path;

class ExprPath :public ExprWithoutBlockNode
{
private:
  std::vector<std::unique_ptr<Path>> paths;
public:
  ExprPath(std::vector<std::unique_ptr<Path>> &&paths): paths(std::move(paths)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif