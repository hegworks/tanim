#pragma once

#include "tanim/include/includes.hpp"
#include "tanim/include/helpers.hpp"
#include <string>

namespace tanim
{

struct EntityData
{
    // entt::entity m_entity{entt::null};
    std::string m_uid{};      // unique identifier
    std::string m_display{};  // optional

    /*
    EntityData(entt::entity entity, const std::string& uid, const std::string& display) : m_entity(entity), m_uid(uid)
    {
        if (display.empty())
        {
            m_display = uid;
        }
    }

    EntityData(entt::entity entity, const std::string& uid) : m_entity(entity), m_uid(uid), m_display(uid) {}
    */
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
