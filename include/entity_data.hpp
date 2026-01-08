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
public:
    SequenceId() = default;

    SequenceId(const EntityData& entity_data, const std::string& struct_name, const std::string& field_name)
        : m_full_name(helpers::MakeFullName(entity_data.m_uid, struct_name, field_name)),
          m_struct_field_name(helpers::MakeStructFieldName(struct_name, field_name)),
          m_field_name(field_name),
          m_struct_name(struct_name),
          m_entity_data(entity_data)
    {
    }

    const std::string& FullName() const { return m_full_name; }
    const std::string& StructFieldName() const { return m_struct_field_name; }

    const EntityData& GetEntityData() const { return m_entity_data; }
    void SetUid(const std::string& uid) { m_entity_data.m_uid = uid; }

    const std::string& FieldName() const { return m_field_name; }
    const std::string& StructName() const { return m_struct_name; }

private:
    std::string m_full_name{};
    std::string m_struct_field_name{};
    std::string m_field_name{};
    std::string m_struct_name{};

    EntityData m_entity_data{};
};

}  // namespace tanim
