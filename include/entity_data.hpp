#pragma once

#include "tanim/include/includes.hpp"
#include "tanim/include/helpers.hpp"
#include <string>

namespace tanim
{

struct EntityData
{
    std::string m_uid{};      // unique identifier
    std::string m_display{};  // optional
};

struct SequenceId
{
    EntityData m_entity_data{};
    std::string m_struct_name{};
    std::string m_field_name{};

    std::string FullName() const { return helpers::MakeFullName(m_entity_data.m_uid, m_struct_name, m_field_name); }
    std::string StructFieldName() const { return helpers::MakeStructFieldName(m_struct_name, m_field_name); }
};

}  // namespace tanim
