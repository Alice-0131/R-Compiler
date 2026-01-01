#ifndef EXPRPATH_HPP
#define EXPRPATH_HPP
#include "ExprNode.hpp"
class Path;

class ExprPath :public ExprWithoutBlockNode
{
public:
  std::shared_ptr<Path> path1;
  std::shared_ptr<Path> path2;
public:
  ExprPath(std::shared_ptr<Path> path1, std::shared_ptr<Path> path2): 
    path1(std::move(path1)), path2(std::move(path2)),
    ExprWithoutBlockNode(K_ExprPath){}
  void accept(ASTVisitor &visitor) override {visitor.visit(*this);}
};

#endif