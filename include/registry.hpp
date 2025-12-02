#pragma once

#include "tanim/include/timeline.hpp"
#include "tanim/include/includes.hpp"

#include <functional>
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
    std::string m_struct_name{};
    std::vector<std::string> m_field_names{};
    std::function<void(Timeline& timeline, const std::string& seq_name)> m_add_sequence;
    std::function<void(Timeline& timeline, float sample_time)> m_sample;
};

template <typename T>
static void AddSequence(T& ecs_component, Timeline& timeline, const std::string& seq_name)
{
    visit_struct::context<VSContext>::for_each(ecs_component,
                                               [&timeline, &seq_name](const char* field_name, auto& field)
                                               {
                                                   using FieldType = std::decay_t<decltype(field)>;
                                                   const std::string field_name_str = field_name;
                                                   if (field_name_str == seq_name)
                                                   {
                                                       if constexpr (std::is_same_v<FieldType, glm::vec3>)
                                                       {
                                                           // TODO(tanim) set based on metadata sequence type (position, color,
                                                           // etc.)
                                                           timeline.AddSequence(0);

                                                           Sequence& seq =
                                                               timeline.GetSequence(timeline.GetSequenceCount() - 1);
                                                           const std::string struct_name = visit_struct::get_name<T>();
                                                           seq.m_name = struct_name + "::" + field_name;
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "X";
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Y";
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Z";
                                                           }
                                                       }
                                                   }
                                               });
}

template <typename T>
static void Sample(T& ecs_component, Timeline& timeline, float sample_time)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&timeline, &sample_time](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            // if (field_name_str == seq_name)
            //{
            if constexpr (std::is_same_v<FieldType, glm::vec3>)
            {
                Sequence& seq = timeline.GetSequence(timeline.GetSequenceCount() - 1);
                field.x = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));
                field.y = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1), sample_time, seq.GetCurveLerpType(1));
                field.z = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2), sample_time, seq.GetCurveLerpType(2));
            }
            //}
        });
}

class Registry
{
public:
    Registry() = default;

    template <typename T>
    void RegisterComponent(entt::registry& registry)
    {
        const std::string type_name = visit_struct::get_name<T>();
        if (IsRegisteredOnce(type_name)) return;

        RegisteredComponent registered_component;

        registered_component.m_struct_name = type_name;

        visit_struct::context<VSContext>::for_each(T{},
                                                   [&](const char* field_name, auto&& field)
                                                   { registered_component.m_field_names.emplace_back(field_name); });

        registered_component.m_add_sequence = [&registry](Timeline& timeline, const std::string& seq_name)
        { AddSequence(registry.get<T>(timeline.m_data->m_entity), timeline, seq_name); };

        registered_component.m_sample = [&registry](Timeline& timeline, float sample_time)
        { Sample(registry.get<T>(timeline.m_data->m_entity), timeline, sample_time); };

        m_components.push_back(std::move(registered_component));
    }

    const std::vector<RegisteredComponent>& GetComponents() { return m_components; }

private:
    std::vector<RegisteredComponent> m_components;

    bool IsRegisteredOnce(const std::string& name)
    {
        for (const auto& component : m_components)
        {
            if (component.m_struct_name == name) return true;
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
