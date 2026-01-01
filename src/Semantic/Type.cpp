#include "../../include/Semantic/Type.hpp"

std::size_t QualType::IdenCounter = 1;

QualType QualType::I_i32(QualType::T_i32);
QualType QualType::I_u32(QualType::T_u32);
QualType QualType::I_isize(QualType::T_isize);
QualType QualType::I_usize(QualType::T_usize);
QualType QualType::I_bool(QualType::T_bool);
QualType QualType::I_void(QualType::T_void);
QualType QualType::I_char(QualType::T_char);

std::unordered_map<std::string, StructQualType *> StructQualType::Instances;

std::unordered_map<MutQualType, const PointerQualType *, MutQualTypeHash>
    PointerQualType::Instances;

std::unordered_map<FuncQualType::Signature, const FuncQualType *, FuncQualType::SignatureHash>
    FuncQualType::Instances;

std::unordered_map<ArrayQualType::ArrayType, const ArrayQualType *, ArrayQualType::ArrayTypeHash>
    ArrayQualType::Instances;

std::unordered_map<std::string, const EnumQualType *> EnumQualType::Instances;

std::unordered_map<int64_t, IntLiteralQualType *> IntLiteralQualType::Instances;

std::unordered_map<std::string, StringQualType *> StringQualType::Instances;