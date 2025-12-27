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
    std::vector<std::string> m_struct_field_names{};

    std::function<void(const entt::registry& entt_registry,
                       TimelineData& timeline_data,
                       const ComponentData& component_data,
                       SequenceId& seq_id)>
        m_add_sequence;

    std::function<void(entt::registry& entt_registry, entt::entity entity, float sample_time, Sequence& seq)> m_sample;

    std::function<void(entt::registry& entt_registry, entt::entity entity, int player_frame, Sequence& seq)> m_inspect;

    std::function<void(const entt::registry& entt_registry, entt::entity entity, int recording_frame, Sequence& seq)> m_record;

    std::function<bool(const entt::registry& entt_registry, entt::entity entity)> m_entity_has;

    bool HasStructFieldName(const std::string& struct_field_name) const
    {
        return std::find(m_struct_field_names.begin(), m_struct_field_names.end(), struct_field_name) !=
               m_struct_field_names.end();
    }
};

namespace reflection
{

template <typename T>
static void AddSequence(T& ecs_component, TimelineData& timeline_data, SequenceId& seq_id)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&timeline_data, &seq_id](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            if (field_name_str == seq_id.m_field_name)
            {
                Sequence& seq = Timeline::AddSequenceStatic(timeline_data);
                seq.m_seq_id = seq_id;
                const float last_frame = (float)Timeline::GetTimelineLastFrame(timeline_data);

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = field_name_str;
                        curve.m_points = {{0.0f, field}, {last_frame, field}};
                    }
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    seq.m_type_meta = Sequence::TypeMeta::INT;

                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = field_name_str;
                        curve.m_points = {{0.0f, static_cast<float>(field)}, {last_frame, static_cast<float>(field)}};
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
                        curve.m_points = {{0.0f, f}, {last_frame, f}};
                        curve.m_lerp_type = sequencer::LerpType::DISCRETE;
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        curve.m_points = {{0, field.x}, {last_frame, field.x}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        curve.m_points = {{0, field.y}, {last_frame, field.y}};
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    seq.m_representation_meta = RepresentationMeta::VECTOR;

                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        curve.m_points = {{0, field.x}, {last_frame, field.x}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        curve.m_points = {{0, field.y}, {last_frame, field.y}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Z";
                        curve.m_points = {{0, field.z}, {last_frame, field.z}};
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    seq.m_representation_meta = RepresentationMeta::COLOR;

                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "R";
                        curve.m_points = {{0, field.r}, {last_frame, field.r}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "G";
                        curve.m_points = {{0, field.g}, {last_frame, field.g}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "B";
                        curve.m_points = {{0, field.b}, {last_frame, field.b}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "A";
                        curve.m_points = {{0, field.a}, {last_frame, field.a}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Spins";
                        curve.m_points = {{0, 0}, {last_frame, 0}};
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    seq.m_representation_meta = RepresentationMeta::QUAT;

                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "W";
                        curve.m_points = {{0, field.w}, {last_frame, field.w}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        curve.m_points = {{0, field.x}, {last_frame, field.x}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        curve.m_points = {{0, field.y}, {last_frame, field.y}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Z";
                        curve.m_points = {{0, field.z}, {last_frame, field.z}};
                    }
                    {
                        Sequence::Curve& curve = seq.AddCurve();
                        curve.m_name = "Spins";
                        curve.m_points = {{0, 0}, {last_frame, 0}};
                        curve.m_lerp_type = sequencer::LerpType::DISCRETE;
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
            const std::string struct_field_name = helpers::MakeStructFieldName(struct_name, field_name_str);
            if (struct_field_name == seq.m_seq_id.StructFieldName())
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
                            const glm::quat q = sequencer::SampleQuatForAnimation(seq, sample_time);
                            field = {q.w, q.x, q.y, q.z};

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
                    field = sequencer::SampleQuatForAnimation(seq, sample_time);
                }
                else
                {
                    static_assert(false, "Unsupported Type");
                }
            }
        });
}

template <typename T>
static void Inspect(T& ecs_component, int player_frame, Sequence& seq)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&seq, &player_frame](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            const std::string struct_name = visit_struct::get_name<T>();
            const std::string struct_field_name = helpers::MakeStructFieldName(struct_name, field_name_str);
            if (struct_field_name == seq.m_seq_id.StructFieldName())
            {
                const float player_frame_f = static_cast<float>(player_frame);
                const auto& curve_0_optional_point_idx = seq.GetPointIdx(0, player_frame);
                const auto& curve_1_optional_point_idx = seq.GetPointIdx(1, player_frame);
                const auto& curve_2_optional_point_idx = seq.GetPointIdx(2, player_frame);
                const auto& curve_3_optional_point_idx = seq.GetPointIdx(3, player_frame);
                const auto& curve_4_optional_point_idx = seq.GetPointIdx(4, player_frame);

                const bool curve_0_disabled = !curve_0_optional_point_idx.has_value();
                const bool curve_1_disabled = !curve_1_optional_point_idx.has_value();
                const bool curve_2_disabled = !curve_2_optional_point_idx.has_value();
                const bool curve_3_disabled = !curve_3_optional_point_idx.has_value();
                const bool curve_4_disabled = !curve_4_optional_point_idx.has_value();

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                         {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER});

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputFloat(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                         {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER});

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputInt(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, static_cast<float>(field)});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Checkbox(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field == true ? 1.0f : 0.0f});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                         {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER},
                                         "X LerpType");

                    helpers::InspectEnum(seq.m_curves.at(1).m_lerp_type,
                                         {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER},
                                         "Y LerpType");

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x) && !curve_0_disabled)
                    {
                        seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field.x});
                    }
                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }

                    if (curve_1_disabled)
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y) && !curve_1_disabled)
                    {
                        seq.EditPoint(1, curve_1_optional_point_idx.value(), {player_frame_f, field.y});
                    }
                    if (curve_1_disabled)
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
                            helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                                 {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER},
                                                 "X LerpType");

                            helpers::InspectEnum(seq.m_curves.at(1).m_lerp_type,
                                                 {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER},
                                                 "Y LerpType");

                            helpers::InspectEnum(seq.m_curves.at(2).m_lerp_type,
                                                 {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER},
                                                 "Z LerpType");

                            if (curve_0_disabled)
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x) && !curve_0_disabled)
                            {
                                seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field.x});
                            }
                            if (curve_0_disabled)
                            {
                                ImGui::EndDisabled();
                            }

                            if (curve_1_disabled)
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y) && !curve_1_disabled)
                            {
                                seq.EditPoint(1, curve_1_optional_point_idx.value(), {player_frame_f, field.y});
                            }
                            if (curve_1_disabled)
                            {
                                ImGui::EndDisabled();
                            }

                            if (curve_2_disabled)
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z) && !curve_2_disabled)
                            {
                                seq.EditPoint(2, curve_2_optional_point_idx.value(), {player_frame_f, field.z});
                            }
                            if (curve_2_disabled)
                            {
                                ImGui::EndDisabled();
                            }

                            break;
                        }
                        case RepresentationMeta::COLOR:
                        {
                            const bool disabled = curve_0_disabled || curve_1_disabled || curve_2_disabled;

                            if (helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                                     {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER}))
                            {
                                seq.m_curves.at(1).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                                seq.m_curves.at(2).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                            }

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit3(field_name, &field.r) && !disabled)
                            {
                                seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field.r});
                                seq.EditPoint(1, curve_1_optional_point_idx.value(), {player_frame_f, field.g});
                                seq.EditPoint(2, curve_2_optional_point_idx.value(), {player_frame_f, field.b});
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
                    if (helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                             {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER}))
                    {
                        seq.m_curves.at(1).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                        seq.m_curves.at(2).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                        seq.m_curves.at(3).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                    }

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
                                seq.m_curves.at(4).m_name = "Spins";
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
                            ImGui::BeginDisabled();

                            ImGui::InputFloat((field_name_str + ".w").c_str(), &field.w);
                            ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x);
                            ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y);
                            ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z);

                            ImGui::EndDisabled();

                            break;
                        }
                        case RepresentationMeta::COLOR:
                        {
                            const bool disabled = curve_0_disabled || curve_1_disabled || curve_2_disabled || curve_3_disabled;

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit4(field_name, &field.r) && !disabled)
                            {
                                seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field.r});
                                seq.EditPoint(1, curve_1_optional_point_idx.value(), {player_frame_f, field.g});
                                seq.EditPoint(2, curve_2_optional_point_idx.value(), {player_frame_f, field.b});
                                seq.EditPoint(3, curve_3_optional_point_idx.value(), {player_frame_f, field.a});
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
                    if (helpers::InspectEnum(seq.m_curves.at(0).m_lerp_type,
                                             {sequencer::LerpType::NONE, sequencer::LerpType::BEZIER}))
                    {
                        seq.m_curves.at(1).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                        seq.m_curves.at(2).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                        seq.m_curves.at(3).m_lerp_type = seq.m_curves.at(0).m_lerp_type;
                    }

                    glm::quat q = field;
                    glm::vec3 euler_angles = glm::degrees(glm::eulerAngles(q));

                    ImGui::BeginDisabled();

                    ImGui::InputFloat((field_name_str + ".w").c_str(), &field.w);
                    ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x);
                    ImGui::InputFloat((field_name_str + ".y").c_str(), &field.y);
                    ImGui::InputFloat((field_name_str + ".z").c_str(), &field.z);

                    ImGui::DragFloat3("euler", &euler_angles.x);

                    ImGui::EndDisabled();

                    if (curve_4_optional_point_idx.has_value())
                    {
                        float spins = seq.m_curves.at(4).m_points.at(curve_4_optional_point_idx.value()).y;
                        int spins_int = (int)spins;
                        if (ImGui::InputInt("Spins", &spins_int))
                        {
                            spins = (float)spins_int;
                            seq.EditPoint(4, curve_4_optional_point_idx.value(), {player_frame_f, spins});

                            field = sequencer::SampleQuatForAnimation(seq, player_frame_f);

                            seq.EditPoint(0, curve_0_optional_point_idx.value(), {player_frame_f, field.w});
                            seq.EditPoint(1, curve_1_optional_point_idx.value(), {player_frame_f, field.x});
                            seq.EditPoint(2, curve_2_optional_point_idx.value(), {player_frame_f, field.y});
                            seq.EditPoint(3, curve_3_optional_point_idx.value(), {player_frame_f, field.z});
                        }
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
static void Record(T& ecs_component, int recording_frame, Sequence& seq)
{
    visit_struct::context<VSContext>::for_each(
        ecs_component,
        [&seq, &recording_frame](const char* field_name, auto& field)
        {
            using FieldType = std::decay_t<decltype(field)>;
            const std::string field_name_str = field_name;
            const std::string struct_name = visit_struct::get_name<T>();
            const std::string struct_field_name = helpers::MakeStructFieldName(struct_name, field_name_str);
            if (struct_field_name == seq.m_seq_id.StructFieldName())
            {
                const float recording_frame_f = static_cast<float>(recording_frame);

                const auto& curve_0_optional_point_idx = seq.GetPointIdx(0, recording_frame_f);
                const auto& curve_1_optional_point_idx = seq.GetPointIdx(1, recording_frame_f);
                const auto& curve_2_optional_point_idx = seq.GetPointIdx(2, recording_frame_f);
                const auto& curve_3_optional_point_idx = seq.GetPointIdx(3, recording_frame_f);

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field});
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, (float)field});
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field == true ? 1.0f : 0.0f});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditPoint(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.y});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditPoint(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.y});
                    seq.EditPoint(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.z});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::COLOR:
                            seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.r});
                            seq.EditPoint(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.g});
                            seq.EditPoint(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.b});
                            seq.EditPoint(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.a});
                            break;
                        case RepresentationMeta::QUAT:
                            seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.w});
                            seq.EditPoint(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.x});
                            seq.EditPoint(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.y});
                            seq.EditPoint(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.z});
                            break;
                        case RepresentationMeta::VECTOR:
                        case RepresentationMeta::NONE:
                        default:
                            assert(0);  // unhandled ReresentationMeta
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    seq.EditPoint(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.w});
                    seq.EditPoint(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditPoint(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.y});
                    seq.EditPoint(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.z});
                }
                else
                {
                    static_assert(false, "Unsupported Type");
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
    void RegisterComponent()
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

                registered_component.m_struct_field_names.emplace_back(helpers::MakeStructFieldName(type_name, field_name));
            });

        registered_component.m_entity_has = [](const entt::registry& entt_registry, entt::entity entity)
        { return entt_registry.all_of<T>(entity); };

        registered_component.m_add_sequence = [](const entt::registry& entt_registry,
                                                 TimelineData& timeline_data,
                                                 const ComponentData& component_data,
                                                 SequenceId& seq_id)
        {
            const auto opt_entity = Timeline::FindEntity(component_data, seq_id.m_entity_data.m_uid);
            if (opt_entity.has_value())
            {
                reflection::AddSequence(entt_registry.get<T>(opt_entity.value()), timeline_data, seq_id);
            }
        };

        registered_component.m_sample = [](entt::registry& entt_registry, entt::entity entity, float sample_time, Sequence& seq)
        {
            if (entity != entt::null)
            {
                if (entt_registry.all_of<T>(entity))
                {
                    reflection::Sample(entt_registry.get<T>(entity), sample_time, seq);
                }
                else
                {
                    LogError("entity " + std::to_string(entt::to_integral(entity)) + " does not have a component named " +
                             visit_struct::get_name<T>());
                }
            }
        };

        registered_component.m_inspect = [](entt::registry& entt_registry, entt::entity entity, int player_frame, Sequence& seq)
        {
            if (entity != entt::null)
            {
                if (entt_registry.all_of<T>(entity))
                {
                    reflection::Inspect(entt_registry.get<T>(entity), player_frame, seq);
                }
                else
                {
                    LogError("entity " + std::to_string(entt::to_integral(entity)) + " does not have a component named " +
                             visit_struct::get_name<T>());
                }
            }
        };

        registered_component.m_record =
            [](const entt::registry& entt_registry, entt::entity entity, int recording_frame, Sequence& seq)
        {
            if (entity != entt::null)
            {
                if (entt_registry.all_of<T>(entity))
                {
                    reflection::Record(entt_registry.get<T>(entity), recording_frame, seq);
                }
                else
                {
                    LogError("entity " + std::to_string(entt::to_integral(entity)) + " does not have a component named " +
                             visit_struct::get_name<T>());
                }
            }
        };

        m_components.push_back(std::move(registered_component));
    }

    const std::vector<RegisteredComponent>& GetComponents() { return m_components; }

private:
    std::vector<RegisteredComponent> m_components;

    bool IsRegisteredOnce(const std::string& name) const
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
