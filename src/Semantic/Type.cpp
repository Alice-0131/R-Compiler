#include "../../include/Semantic/Type.hpp"

std::size_t QualType::IdenCounter = 1;

QualType QualType::I_i32(QualType::T_i32);
QualType QualType::I_u32(QualType::T_u32);
QualType QualType::I_isize(QualType::T_isize);
QualType QualType::I_usize(QualType::T_usize);
QualType QualType::I_bool(QualType::T_bool);
QualType QualType::I_void(QualType::T_void);
QualType QualType::I_char(QualType::T_char);

std::unordered_map<MutQualType, const PointerQualType *, MutQualTypeHash>
    PointerQualType::Instances;