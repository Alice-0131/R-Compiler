#ifndef EXPRPATH_HPP
#define EXPRPATH_HPP
#include "ExprNode.hpp"
class Path;

class ExprPath :public ExprWithoutBlockNode
{
private:
  std::unique_ptr<Path> path1;
  std::unique_ptr<Path> path2;
public:
  ExprPath(std::unique_ptr<Path> path1, std::unique_ptr<Path> path2): 
    path1(std::move(path1)), path2(std::move(path2)){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif