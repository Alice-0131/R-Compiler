#ifndef TYPE_HPP
#define TYPE_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class QualType {
public:
  enum TypeEnum{
    T_i32,
    T_u32,
    T_usize,
    T_isize,
    T_bool,
    T_char,
    T_string,
    T_void,
    T_ptr,
    T_struct,
    T_enum,
    T_array,
    T_func,
    T_intLiteral,
  };

private:
  const std::size_t TypeIden; // 每个类型拥有唯一编号
  static std::size_t IdenCounter;

  const TypeEnum TypeID; // 记录类型

  static QualType I_i32;
  static QualType I_u32;
  static QualType I_isize;
  static QualType I_usize;
  static QualType I_bool;
  static QualType I_void;
  static QualType I_char;
  static QualType I_string;

public:
  QualType(TypeEnum Tid) : TypeIden(IdenCounter++), TypeID(Tid){}
  
  virtual ~QualType() = default;

  TypeEnum getTypeID() const { return TypeID; }

  std::size_t getTypeIden() const { return TypeIden; }

  virtual bool equals(const QualType *Other) const { // 类型相等性比较
    switch(Other->getTypeID()) {
    case T_intLiteral:
      return Other->equals(this);
    default:
      return this->getTypeIden() == Other->getTypeIden();
    }
  }

  static QualType *getI32Type() { return &I_i32; }
  static QualType *getU32Type() { return &I_u32; }
  static QualType *getIsizeType() { return &I_isize; }
  static QualType *getUsizeType() { return &I_usize; }
  static QualType *getBoolType() { return &I_bool; }
  static QualType *getVoidType() { return &I_void; }
  static QualType *getCharType() { return &I_char; }
  static QualType *getStringType() { return &I_string; }

  bool isI32() const { return TypeID == T_i32; }
  bool isU32() const { return TypeID == T_u32; }
  bool isIsize() const { return TypeID == T_isize; }
  bool isUsize() const { return TypeID == T_usize; }
  bool isBool() const { return TypeID == T_bool; }
  bool isVoid() const { return TypeID == T_void; }
  bool isChar() const { return TypeID == T_char; }
  bool isString() const { return TypeID == T_string; }
  bool isStruct() const { return TypeID == T_struct; }
  bool isEnum() const { return TypeID == T_enum; }
  bool isPointer() const { return TypeID == T_ptr; }
  bool isArray() const { return TypeID == T_array; }
  bool isFunc() const { return TypeID == T_func; }
  bool isIntLiteral() const { return TypeID == T_intLiteral; }
};

using MutQualType = std::pair<bool, const QualType*>; //（是否可变，类型指针）

struct MutQualTypeHash {
  std::size_t operator()(const MutQualType &MQT) const {
    std::size_t value = (MQT.first << 10) + MQT.second->getTypeIden();
    return std::hash<std::size_t>{}(value);
  }
};

class PointerQualType : public QualType {
private:
  bool mut; //是否可变指针
  const QualType *ElemType; //指针指向的元素类型

  static std::unordered_map<MutQualType, const PointerQualType*, MutQualTypeHash> Instances; // 驻留表

public:
  PointerQualType(bool isMut, const QualType *ElemTy)
      : QualType(T_ptr), mut(isMut), ElemType(ElemTy) {}

  const QualType *getElemType() const { return ElemType; }

  static const PointerQualType *create(bool isMut, const QualType *ElemTy) {
    MutQualType key = std::make_pair(isMut, ElemTy);
    auto it = Instances.find(key);
    if (it != Instances.end()) {
      return it->second;
    } else {
      const PointerQualType *newPtrType = new PointerQualType(isMut, ElemTy);
      Instances[key] = newPtrType;
      return newPtrType;
    }
  }

  bool equals(const QualType *Other) const override {
    if (!Other->isPointer()) return false;
    const PointerQualType *OtherPtr =static_cast<const PointerQualType *>(Other);
    return this->ElemType->equals(OtherPtr->ElemType);
  }

  bool isMut() const { return mut; }

  static const PointerQualType *get(bool mut, const QualType *ElemTy) {
    return create(mut, ElemTy);
  }
};

class FuncQualType : public QualType {
  using Signature = std::pair<std::vector<const QualType*>, const QualType*>; //（参数类型列表，返回值类型）

  struct SignatureHash{
    std::size_t operator()(const Signature &Sig) const {
      std::size_t hashValue = Sig.second->getTypeIden();
      for (const auto &ParamType : Sig.first) {
        hashValue += ParamType->getTypeIden();
      }
      return std::hash<std::size_t>{}(hashValue);
    }
  };

private:
  bool isAssociated = false; //是否为关联函数
  Signature FuncSig; //函数签名
  static std::unordered_map<Signature, const FuncQualType*, SignatureHash> Instances; //驻留表

public:
  FuncQualType(const Signature &Sig, bool isAssoc)
      : QualType(T_func), isAssociated(isAssoc), FuncSig(Sig) {}

  const std::vector<const QualType *> &getParamTypes() const {
    return FuncSig.first;
  }

  const QualType *getReturnType() const { return FuncSig.second; }

  static const FuncQualType *create(bool isAssociate, std::vector<const QualType *> Params, const QualType *Ret) {
    if (!Ret) {
      throw std::runtime_error("Return type cannot be null");
    }
    auto Sig = std::make_pair(Params, Ret);
    auto it = Instances.find(Sig);
    if (it != Instances.end()) {
      return it->second;
    } else {
      const FuncQualType *newFuncType = new FuncQualType(Sig, isAssociate);
      Instances[Sig] = newFuncType;
      return newFuncType;
    }
  }
};


class ArrayQualType : public QualType {
  using ArrayType = std::pair<const QualType*, std::size_t>; //（元素类型，长度）

  struct ArrayTypeHash {
    std::size_t operator()(const ArrayType &ArrTy) const {
      std::size_t hashValue = ArrTy.first->getTypeIden() + ArrTy.second;
      return std::hash<std::size_t>{}(hashValue);
    }
  };

private:
  ArrayType Ty; //数组类型
  static std::unordered_map<ArrayType, const ArrayQualType*, ArrayTypeHash> Instances; //驻留表

public:
  ArrayQualType(ArrayType &Ty) : Ty(Ty), QualType(T_array) {}

  const QualType *getElemType() const { return Ty.first; }

  std::size_t getLength() const { return Ty.second; }

  static const ArrayQualType *create(const QualType *ElemTy, std::size_t Len) {
    ArrayType key = std::make_pair(ElemTy, Len);
    auto it = Instances.find(key);
    if (it != Instances.end()) {
      return it->second;
    } else {
      const ArrayQualType *newArrType = new ArrayQualType(key);
      Instances[key] = newArrType;
      return newArrType;
    }
  }

  bool equals(const QualType *Other) const override {
    if (!Other->isArray()) return false;
    const ArrayQualType *OtherArr = static_cast<const ArrayQualType *>(Other);
    if (this->Ty.second != OtherArr->Ty.second) return false;
    return this->Ty.first->equals(OtherArr->Ty.first);
  }
};

class StructQualType : public QualType {
public:
struct Field {
    std::string Name;
    const QualType *Type;

    Field(const std::string &name, const QualType *type) : Name(name), Type(type) {}
  };

private:
  std::string Name;
  std::vector<Field> Fields;
  std::unordered_map<std::string, const FuncQualType *> Methods;

  static std::unordered_map<std::string, StructQualType*> Instances; //驻留表

public:
  StructQualType(const std::string &name): QualType(T_struct), Name(name) {}

  const std::string &getName() const { return Name; }

  const std::vector<Field> &getFields() const { return Fields; }

  const QualType *getFieldType(const std::string &fieldName) const {
    for (const auto &field : Fields) {
      if (field.Name == fieldName) {
        return field.Type;
      }
    }
    return nullptr; //字段不存在
  }

  const std::unordered_map<std::string, const FuncQualType *> &getMethods() const {return Methods;}

  const FuncQualType *getMethodSig(std::string fnName) const {
    auto it = Methods.find(fnName);
    return it == Methods.end() ? nullptr : it->second;
  }

  void insertField(Field field) {
    Fields.push_back(field);
  }

  void insertMethod(const std::string &methodName, const FuncQualType *methodSig) {
    if (Methods.find(methodName) != Methods.end()) {
      throw std::runtime_error("Method " + methodName + " already exists in struct " + Name);
    }
    Methods[methodName] = methodSig;
  }

  static StructQualType *create(const std::string &name) {
    auto it = Instances.find(name);
    if (it != Instances.end()) {
      return it->second;
    } else {
      StructQualType *newStructType = new StructQualType(name);
      Instances[name] = newStructType;
      return newStructType;
    }
  }

  static const StructQualType *get(std::string Name) { return Instances[Name]; }
};

class EnumQualType : public QualType {
private:
  std::string Name;
  std::vector<std::string> Fields;

  static std::unordered_map<std::string, const EnumQualType*> Instances; //驻留表

public:
  EnumQualType(const std::string &name, std::vector<std::string> Fields) : QualType(T_enum), Name(name), Fields(Fields) {}

  static const EnumQualType *create(std::string Name, std::vector<std::string> Fields) {
    if (Instances.find(Name) != Instances.end())
      return Instances[Name];
    return Instances[Name] = new EnumQualType(Name, Fields);
  }

  const std::vector<std::string> &getFields() const { return Fields; }

  size_t indexOf(std::string Name) const {
    int Idx = 0;
    for (auto& F : Fields) {
      if (Name == F)
        return Idx;
      ++Idx;
    }
    return -1;
  }

  bool contains(std::string Name) const { return indexOf(Name) >= 0; }
};

class IntLiteralQualType : public QualType {
private:
  int64_t Num;

  static std::unordered_map<int64_t, IntLiteralQualType *> Instances;

  IntLiteralQualType(int64_t Num) : Num(Num), QualType(T_intLiteral) {}

public:
  static const IntLiteralQualType *create(int64_t Num) {
    if (Instances.count(Num))
      return Instances[Num];
    return Instances[Num] = new IntLiteralQualType(Num);
  }

  bool equals(const QualType *Other) const override {
    switch (Other->getTypeID()) {
    default:
      return false;
    case T_i32:
    case T_isize:
      return Num >= INT32_MIN && Num <= INT32_MAX;
    case T_u32:
    case T_usize:
      return Num >= 0 && Num <= UINT32_MAX;
    case T_intLiteral:
      return true;
    }
  }

  int64_t getValue() const { return Num; }
};

class StringQualType : public QualType {
private:
  std::string s;

  static std::unordered_map<std::string, StringQualType*> Instances;

public:
  StringQualType(const std::string &str) : QualType(T_string), s(str) {}

  std::string getString() const { return s; }
  
  static const StringQualType *create(std::string s) {
    if (Instances.count(s)) return Instances[s];
    return Instances[s] = new StringQualType(s);
  }
};

#endif // TYPE_HPP