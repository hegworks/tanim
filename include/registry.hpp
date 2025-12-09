#pragma once

#include "tanim/include/timeline.hpp"
#include "tanim/include/includes.hpp"
#include "tanim/include/enums.hpp"

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
    std::vector<std::string> m_full_names{};  // m_struct_name + "::" + m_field_name
    std::function<void(Timeline& timeline, const std::string& seq_name)> m_add_sequence;
    std::function<void(Timeline& timeline, float sample_time, Sequence& seq)> m_sample;
    std::function<void(Timeline& timeline, Sequence& seq)> m_inspect;
    std::function<bool(entt::entity entity)> m_entity_has;
    bool HasSequence(const std::string& seq_name) const
    {
        return std::find(m_full_names.begin(), m_full_names.end(), seq_name) != m_full_names.end();
    }
};

namespace reflection
{

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
                                                       const std::string struct_name = visit_struct::get_name<T>();
                                                       const std::string full_name = struct_name + "::" + field_name;

                                                       timeline.AddSequence(0);
                                                       Sequence& seq = timeline.GetSequence(timeline.GetSequenceCount() - 1);
                                                       seq.m_name = full_name;

                                                       if constexpr (std::is_same_v<FieldType, float>)
                                                       {
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = field_name_str;
                                                               curve.m_points = {{0, field}, {10, field}};
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, int>)
                                                       {
                                                           seq.m_type_meta = Sequence::TypeMeta::INT;

                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = field_name_str;
                                                               curve.m_points = {{0.0f, static_cast<float>(field)},
                                                                                 {10.0f, static_cast<float>(field)}};
                                                               curve.m_lerp_type = sequencer::LerpType::LINEAR;
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, bool>)
                                                       {
                                                           seq.m_type_meta = Sequence::TypeMeta::BOOL;

                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = field_name_str;
                                                               float f = field == true ? 1.0f : 0.0f;
                                                               curve.m_points = {{0.0f, f}, {10.0f, f}};
                                                               curve.m_lerp_type = sequencer::LerpType::DISCRETE;
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                                                       {
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "X";
                                                               curve.m_points = {{0, field.x}, {10, field.x}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Y";
                                                               curve.m_points = {{0, field.y}, {10, field.y}};
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                                                       {
                                                           seq.m_representation_meta = RepresentationMeta::VECTOR;

                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "X";
                                                               curve.m_points = {{0, field.x}, {10, field.x}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Y";
                                                               curve.m_points = {{0, field.y}, {10, field.y}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Z";
                                                               curve.m_points = {{0, field.z}, {10, field.z}};
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                                                       {
                                                           seq.m_representation_meta = RepresentationMeta::COLOR;

                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "R";
                                                               curve.m_points = {{0, field.r}, {10, field.r}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "G";
                                                               curve.m_points = {{0, field.g}, {10, field.g}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "B";
                                                               curve.m_points = {{0, field.b}, {10, field.b}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "A";
                                                               curve.m_points = {{0, field.a}, {10, field.a}};
                                                           }
                                                       }
                                                       else if constexpr (std::is_same_v<FieldType, glm::quat>)
                                                       {
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "W";
                                                               curve.m_points = {{0, field.w}, {10, field.w}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "X";
                                                               curve.m_points = {{0, field.x}, {10, field.x}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Y";
                                                               curve.m_points = {{0, field.y}, {10, field.y}};
                                                           }
                                                           {
                                                               Sequence::Curve& curve = seq.AddCurve();
                                                               curve.m_name = "Z";
                                                               curve.m_points = {{0, field.z}, {10, field.z}};
                                                           }
                                                       }
                                                       else
                                                       {
                                                           static_assert(false, "Unsupported Type");
                                                       }
                                                   }
                                               });
}

template <typename T>
static void Sample(T& ecs_component, float sample_time, Sequence& seq)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&seq, &sample_time](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            const std::string struct_name = visit_struct::get_name<T>();
            const std::string full_name = struct_name + "::" + field_name;
            if (seq.m_name == full_name)
            {
                if constexpr (std::is_same_v<FieldType, float>)
                {
                    field = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    field = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));
                    field = static_cast<int>(std::floorf(field));
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    field = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));
                    field = std::round(field) >= 0.5f ? true : false;
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    field.x =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));

                    field.y =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1), sample_time, seq.GetCurveLerpType(1));
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    field.x =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));

                    field.y =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1), sample_time, seq.GetCurveLerpType(1));

                    field.z =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2), sample_time, seq.GetCurveLerpType(2));
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::COLOR:
                        {
                            field.r = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(0));

                            field.g = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(1));

                            field.b = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(2));

                            field.a = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(3),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(3));

                            break;
                        }
                        case RepresentationMeta::QUAT:
                        {
                            field.w = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(0));

                            field.x = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(1));

                            field.y = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(2));

                            field.z = sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(3),
                                                                         sample_time,
                                                                         seq.GetCurveLerpType(3));

                            break;
                        }
                        case RepresentationMeta::VECTOR:
                        case RepresentationMeta::NONE:
                        default:
                            assert(0);  // unhandled RepresentaionMeta
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    field.w =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sample_time, seq.GetCurveLerpType(0));

                    field.x =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1), sample_time, seq.GetCurveLerpType(1));

                    field.y =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2), sample_time, seq.GetCurveLerpType(2));

                    field.z =
                        sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(3), sample_time, seq.GetCurveLerpType(3));
                }
                else
                {
                    static_assert(false, "Unsupported Type");
                }
            }
        });
}

template <typename T>
static void Inspect(T& ecs_component, Timeline& timeline, Sequence& seq)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&seq, &timeline](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            const std::string struct_name = visit_struct::get_name<T>();
            const std::string full_name = struct_name + "::" + field_name;
            if (seq.m_name == full_name)
            {
                const auto& curve_0_optional_point_idx = seq.GetPointIdx(0, timeline.GetPlayerFrame());
                const auto& curve_1_optional_point_idx = seq.GetPointIdx(1, timeline.GetPlayerFrame());
                const auto& curve_2_optional_point_idx = seq.GetPointIdx(2, timeline.GetPlayerFrame());
                const auto& curve_3_optional_point_idx = seq.GetPointIdx(3, timeline.GetPlayerFrame());

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputFloat(field_name, &field))
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field});
                    }

                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputInt(field_name, &field))
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), (float)field});
                    }

                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Checkbox(field_name, &field))
                    {
                        seq.EditPoint(0,
                                      curve_0_optional_point_idx.value(),
                                      {(float)timeline.GetPlayerFrame(), field == true ? 1.0f : 0.0f});
                    }

                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x))
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.x});
                    }
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }

                    if (!curve_1_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y))
                    {
                        seq.EditPoint(1, curve_1_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.y});
                    }
                    if (!curve_1_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    helpers::InspectEnum(seq.m_representation_meta, {RepresentationMeta::NONE, RepresentationMeta::QUAT});

                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::VECTOR:
                        {
                            if (!curve_0_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x))
                            {
                                seq.EditPoint(0,
                                              curve_0_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.x});
                            }
                            if (!curve_0_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            if (!curve_1_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y))
                            {
                                seq.EditPoint(1,
                                              curve_1_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.y});
                            }
                            if (!curve_1_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            if (!curve_2_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z))
                            {
                                seq.EditPoint(1,
                                              curve_2_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.z});
                            }
                            if (!curve_2_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            break;
                        }
                        case RepresentationMeta::COLOR:
                        {
                            const bool disabled = !curve_0_optional_point_idx.has_value() ||
                                                  !curve_1_optional_point_idx.has_value() ||
                                                  !curve_2_optional_point_idx.has_value();

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit3(field_name, &field.r) && !disabled)
                            {
                                seq.EditPoint(0,
                                              curve_0_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.r});

                                seq.EditPoint(1,
                                              curve_1_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.g});

                                seq.EditPoint(2,
                                              curve_2_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.b});
                            }

                            if (disabled)
                            {
                                ImGui::EndDisabled();
                            }

                            break;
                        }
                        case RepresentationMeta::QUAT:
                        case RepresentationMeta::NONE:
                        default:
                            assert(0);  // unhandled ReresentationMeta
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    if (helpers::InspectEnum(seq.m_representation_meta, {RepresentationMeta::NONE, RepresentationMeta::VECTOR}))
                    {
                        switch (seq.m_representation_meta)
                        {
                            case RepresentationMeta::COLOR:
                                seq.m_curves.at(0).m_name = "R";
                                seq.m_curves.at(1).m_name = "G";
                                seq.m_curves.at(2).m_name = "B";
                                seq.m_curves.at(3).m_name = "A";
                                break;
                            case RepresentationMeta::QUAT:
                                seq.m_curves.at(0).m_name = "W";
                                seq.m_curves.at(1).m_name = "X";
                                seq.m_curves.at(2).m_name = "Y";
                                seq.m_curves.at(3).m_name = "Z";
                                break;
                            case RepresentationMeta::VECTOR:
                            case RepresentationMeta::NONE:
                            default:
                                assert(0);  // unhandled ReresentationMeta
                        }
                    }

                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::QUAT:
                        {
                            if (!curve_0_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".w").c_str(), &field.w))
                            {
                                seq.EditPoint(0,
                                              curve_0_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.w});
                            }
                            if (!curve_0_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            if (!curve_1_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x))
                            {
                                seq.EditPoint(1,
                                              curve_1_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.x});
                            }
                            if (!curve_1_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            if (!curve_2_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y))
                            {
                                seq.EditPoint(2,
                                              curve_2_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.y});
                            }
                            if (!curve_2_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            if (!curve_3_optional_point_idx.has_value())
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z))
                            {
                                seq.EditPoint(3,
                                              curve_3_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.z});
                            }
                            if (!curve_3_optional_point_idx.has_value())
                            {
                                ImGui::EndDisabled();
                            }

                            break;
                        }
                        case RepresentationMeta::COLOR:
                        {
                            const bool disabled =
                                !curve_0_optional_point_idx.has_value() || !curve_1_optional_point_idx.has_value() ||
                                !curve_2_optional_point_idx.has_value() || !curve_3_optional_point_idx.has_value();

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit4(field_name, &field.r) && !disabled)
                            {
                                seq.EditPoint(0,
                                              curve_0_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.r});

                                seq.EditPoint(1,
                                              curve_1_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.g});

                                seq.EditPoint(2,
                                              curve_2_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.b});

                                seq.EditPoint(3,
                                              curve_3_optional_point_idx.value(),
                                              {(float)timeline.GetPlayerFrame(), field.a});
                            }

                            if (disabled)
                            {
                                ImGui::EndDisabled();
                            }

                            break;
                        }
                        case RepresentationMeta::VECTOR:
                        case RepresentationMeta::NONE:
                        default:
                            assert(0);  // unhandled ReresentationMeta
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".w").c_str(), &field.w))
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.w});
                    }
                    if (!curve_0_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }

                    if (!curve_1_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x))
                    {
                        seq.EditPoint(1, curve_1_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.x});
                    }
                    if (!curve_1_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }

                    if (!curve_2_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y))
                    {
                        seq.EditPoint(2, curve_2_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.y});
                    }
                    if (!curve_2_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }

                    if (!curve_3_optional_point_idx.has_value())
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z))
                    {
                        seq.EditPoint(3, curve_3_optional_point_idx.value(), {(float)timeline.GetPlayerFrame(), field.z});
                    }
                    if (!curve_3_optional_point_idx.has_value())
                    {
                        ImGui::EndDisabled();
                    }
                }
            }
        });
}

}  // namespace reflection

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

        visit_struct::context<VSContext>::for_each(
            T{},
            [&](const char* field_name, auto&& field)
            {
                registered_component.m_field_names.emplace_back(field_name);
                registered_component.m_full_names.emplace_back(registered_component.m_struct_name + "::" + field_name);
            });

        registered_component.m_add_sequence = [&registry](Timeline& timeline, const std::string& seq_name)
        { reflection::AddSequence(registry.get<T>(timeline.m_data->m_entity), timeline, seq_name); };

        registered_component.m_sample = [&registry](Timeline& timeline, float sample_time, Sequence& seq)
        { reflection::Sample(registry.get<T>(timeline.m_data->m_entity), sample_time, seq); };

        registered_component.m_inspect = [&registry](Timeline& timeline, Sequence& seq)
        { reflection::Inspect(registry.get<T>(timeline.m_data->m_entity), timeline, seq); };

        registered_component.m_entity_has = [&registry](entt::entity entity) { return registry.all_of<T>(entity); };

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
