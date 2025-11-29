#pragma once

#include "tanim/include/includes.hpp"

#include <string>
#include <vector>

namespace tanim
{

/// VisitStructContext
struct VSContext
{
};

struct RegisteredComponent
{
    std::string m_name{};
};

class Registry
{
public:
    Registry() = default;

    template <typename T>
    inline static void RegisterComponent()
    {
        const std::string type_name = visit_struct::get_name<T>();
        if (IsRegisteredOnce(type_name)) return;

        RegisteredComponent registered_component;
        registered_component.m_name = type_name;

        m_components.push_back(std::move(registered_component));
    }

private:
    inline static std::vector<RegisteredComponent> m_components;

    static bool IsRegisteredOnce(const std::string& name)
    {
        for (const auto& component : m_components)
        {
            if (component.m_name == name) return true;
        }
        return false;
    }
};

inline Registry& GetRegistry()
{
    static Registry instance;
    return instance;
}

}  // namespace tanim
