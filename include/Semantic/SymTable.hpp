#ifndef SYMTABLE_HPP
#define SYMTABLE_HPP

#include "Type.hpp"
#include <atomic>
#include <unordered_map>

template <class K, class V> 
class TableImpl{
public:
  std::unordered_map<K, V> Table;
};

class StructTable : public TableImpl<std::string, StructQualType *> {
public:
  bool count(std::string name) const {return Table.count(name);}

  bool create(std::string Name) {
    if (count(Name)) return false;
    Table[Name] = StructQualType::create(Name);
    return true;
  }

  void insertField(std::string Name, StructQualType::Field field) {
    auto s = Table[Name];
    s->insertField(field);
  }

  void insertMethod(std::string Name, std::string fnName, const FuncQualType *fnSig) {
    auto s = Table[Name];
    s->insertMethod(fnName, fnSig);
  }

  StructQualType *getTy(std::string Name) const {
    if (!count(Name))
      return nullptr;
    return Table.find(Name)->second;
  }
};

class FuncTable : public TableImpl<std::string, const FuncQualType *> {
public:
  bool count(std::string Name) { return Table.count(Name); }

  bool create(std::string Name, const FuncQualType *Ty) {
    if (count(Name)) return false;
    Table[Name] = Ty;
    return true;
  }

  const FuncQualType *getTy(std::string Name) { return Table[Name]; }
};

class EnumTable : public TableImpl<std::string, const EnumQualType *> {
public:
  bool count(std::string Name) { return Table.count(Name); }

  bool create(std::string Name, const EnumQualType *Ty) {
    if (count(Name))
      return false;
    Table[Name] = Ty;
    return true;
  }

  const EnumQualType *getTy(std::string Name) { return Table[Name]; }
};

class TraitTable : public TableImpl<std::string, std::vector<std::pair<std::string, const FuncQualType *>>> {
public:
  using Method = std::pair<std::string, const FuncQualType *>;

  bool count(std::string Name) { return Table.count(Name); }

  bool create(std::string Name) {
    if (count(Name)) return false;
    Table[Name] = std::vector<Method>();
    return true;
  }

  void insertMethod(std::string Name, Method method) {
    Table[Name].push_back(method);
  }

  std::vector<Method> &getTrait(std::string Name) { return Table[Name]; }
};

class ConstTable : public TableImpl<std::string, std::pair<const QualType *, long>> {
public:
  bool count(std::string Name) const { return Table.count(Name); }

  bool create(std::string Name, const QualType *Ty, long value = 0) {
    if (count(Name)) return false;
    Table[Name] = std::make_pair(Ty, value);
    return true;
  }

  const QualType *getTy(std::string Name) { return Table[Name].first; }

  void setValue(std::string Name, long value) { Table[Name].second = value; }

  long getValue(std::string Name) const {
    if (!count(Name)) {
      throw std::runtime_error("Constant " + Name + " not found.");
    }
    return Table.find(Name)->second.second;
  }
};

struct ScopeTable {
  std::size_t ID; // 唯一标识符
  std::unordered_map<std::string, std::string> local;// 原名 -> 新名
};

class LocalScope {
  private:
  static unsigned Counter;
  std::vector<ScopeTable> scopeStack;

  std::string genNewName(std::string Name, unsigned ID) {
    return "_" + Name + "_" + std::to_string(ID);
  }

public:
  LocalScope() : scopeStack({}) {};

  void enterScope() {
    ScopeTable curr = ScopeTable{Counter++};
    scopeStack.push_back(curr);
  }

  void exitScope() { scopeStack.pop_back(); }

  std::string createLocal(std::string Name) {
    auto &top = scopeStack.back();
    std::string newName = genNewName(Name, top.ID);
    if (top.local.count(Name))
      newName = genNewName(newName, top.ID);
    top.local[Name] = newName;
    return newName;
  }

  bool count(std::string Name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
      auto &scope = *it;
      if (scope.local.count(Name))
        return true;
    }
    return false;
  }

  std::string getNewName(std::string Name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
      auto &scope = *it;
      if (scope.local.count(Name))
        return scope.local[Name];
    }
    return "";
  }
};

struct SymTable {
  StructTable structTable; // record all struct with its type
  FuncTable fnTable; // record all functions (expect methods) and their signature
  EnumTable enumTable; // record all enums
  TraitTable traitTable; // record all traits with their methods
  ConstTable constTable; // record all const variables
};

#endif // SYMTABLE_HPP