#pragma once

#include "sequencer.hpp"
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
                        Curve& curve = seq.AddCurve();
                        curve.m_name = field_name_str;
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0.0f, field});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    seq.m_type_meta = Sequence::TypeMeta::INT;

                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = field_name_str;
                        SetCurveHandleType(curve, CurveHandleType::LINEAR);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0.0f, static_cast<float>(field)});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, static_cast<float>(field)});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    seq.m_type_meta = Sequence::TypeMeta::BOOL;

                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = field_name_str;
                        SetCurveHandleType(curve, CurveHandleType::CONSTANT);
                        LockCurveHandleType(curve);
                        float f = field == true ? 1.0f : 0.0f;
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0.0f, f});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, f});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.x});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.x});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.y});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.y});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    seq.m_representation_meta = RepresentationMeta::VECTOR;

                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.x});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.x});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.y});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.y});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Z";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.z});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.z});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    seq.m_representation_meta = RepresentationMeta::COLOR;

                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "R";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.r});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.r});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "G";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.g});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.g});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "B";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.b});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.b});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "A";
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.a});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.a});
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    seq.m_representation_meta = RepresentationMeta::QUAT;

                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "W";
                        SetCurveHandleType(curve, CurveHandleType::LINEAR);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.w});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.w});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "X";
                        SetCurveHandleType(curve, CurveHandleType::LINEAR);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.x});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.x});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Y";
                        SetCurveHandleType(curve, CurveHandleType::LINEAR);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.y});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.y});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Z";
                        SetCurveHandleType(curve, CurveHandleType::LINEAR);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, field.z});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, field.z});
                    }
                    {
                        Curve& curve = seq.AddCurve();
                        curve.m_name = "Spins";
                        SetCurveHandleType(curve, CurveHandleType::CONSTANT);
                        LockCurveHandleType(curve);
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {0, 0});
                        seq.AddKeyframeAtPos(seq.GetCurveCount() - 1, {last_frame, 0});
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
                    field = SampleCurveValue(seq.m_curves.at(0), sample_time);
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    field = SampleCurveValue(seq.m_curves.at(0), sample_time);
                    field = static_cast<int>(std::floorf(field));
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    field = SampleCurveValue(seq.m_curves.at(0), sample_time);
                    field = std::round(field) >= 0.5f ? true : false;
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    field.x = SampleCurveValue(seq.m_curves.at(0), sample_time);
                    field.y = SampleCurveValue(seq.m_curves.at(1), sample_time);
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    field.x = SampleCurveValue(seq.m_curves.at(0), sample_time);
                    field.y = SampleCurveValue(seq.m_curves.at(1), sample_time);
                    field.z = SampleCurveValue(seq.m_curves.at(2), sample_time);
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::COLOR:
                        {
                            field.r = SampleCurveValue(seq.m_curves.at(0), sample_time);
                            field.g = SampleCurveValue(seq.m_curves.at(1), sample_time);
                            field.b = SampleCurveValue(seq.m_curves.at(2), sample_time);
                            field.a = SampleCurveValue(seq.m_curves.at(3), sample_time);

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

inline void SyncAllHandleTypesInCurve(Sequence& seq, CurveHandleType curve_handle_type, int curves_count)
{
    for (int c = 0; c < curves_count; ++c)
    {
        SetCurveHandleType(seq.m_curves.at(c), curve_handle_type);
        ApplyCurveHandleTypeOnCurve(seq.m_curves.at(c));
    }
}

inline void SyncAllHandleTypeLocksInCurve(Sequence& seq, bool lock_state, int curves_count)
{
    for (int c = 0; c < curves_count; ++c)
    {
        if (lock_state)
        {
            LockCurveHandleType(seq.m_curves.at(c));
        }
        else
        {
            UnlockCurveHandleType(seq.m_curves.at(c));
        }
    }
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
                const auto& curve_0_optional_point_idx = seq.GetKeyframeIdx(0, player_frame);
                const auto& curve_1_optional_point_idx = seq.GetKeyframeIdx(1, player_frame);
                const auto& curve_2_optional_point_idx = seq.GetKeyframeIdx(2, player_frame);
                const auto& curve_3_optional_point_idx = seq.GetKeyframeIdx(3, player_frame);
                const auto& curve_4_optional_point_idx = seq.GetKeyframeIdx(4, player_frame);

                const bool curve_0_disabled = !curve_0_optional_point_idx.has_value();
                const bool curve_1_disabled = !curve_1_optional_point_idx.has_value();
                const bool curve_2_disabled = !curve_2_optional_point_idx.has_value();
                const bool curve_3_disabled = !curve_3_optional_point_idx.has_value();
                const bool curve_4_disabled = !curve_4_optional_point_idx.has_value();

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    {
                        Curve& curve = seq.m_curves.at(0);
                        ImGui::Checkbox("Lock All Handles", &curve.m_handle_type_locked);
                        if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "All Handles' Type"))
                        {
                            ApplyCurveHandleTypeOnCurve(curve);
                        }
                    }

                    ImGui::Separator();

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputFloat(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    {
                        Curve& curve = seq.m_curves.at(0);
                        ImGui::Checkbox("Lock All Handles", &curve.m_handle_type_locked);
                        if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "All Handles' Type"))
                        {
                            ApplyCurveHandleTypeOnCurve(curve);
                        }
                    }

                    ImGui::Separator();

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::InputInt(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, static_cast<float>(field)});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    {
                        ImGui::BeginDisabled();
                        Curve& curve = seq.m_curves.at(0);
                        ImGui::Checkbox("Lock All Handles", &curve.m_handle_type_locked);
                        if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "All Handles' Type"))
                        {
                            ApplyCurveHandleTypeOnCurve(curve);
                        }
                        ImGui::EndDisabled();
                    }

                    ImGui::Separator();

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Checkbox(field_name, &field) && !curve_0_disabled)
                    {
                        seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field == true ? 1.0f : 0.0f});
                    }

                    if (curve_0_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    {
                        ImGui::PushID(0);
                        Curve& curve = seq.m_curves.at(0);
                        ImGui::Checkbox("X Lock All Handles", &curve.m_handle_type_locked);
                        if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "X All Handles' Type"))
                        {
                            ApplyCurveHandleTypeOnCurve(curve);
                        }
                        ImGui::PopID();
                    }
                    {
                        ImGui::PushID(1);
                        Curve& curve = seq.m_curves.at(1);
                        ImGui::Checkbox("Y Lock All Handles", &curve.m_handle_type_locked);
                        if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "Y All Handles' Type"))
                        {
                            ApplyCurveHandleTypeOnCurve(curve);
                        }
                        ImGui::PopID();
                    }

                    ImGui::Separator();

                    if (curve_0_disabled)
                    {
                        ImGui::BeginDisabled();
                    }
                    if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x) && !curve_0_disabled)
                    {
                        seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field.x});
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
                        seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {player_frame_f, field.y});
                    }
                    if (curve_1_disabled)
                    {
                        ImGui::EndDisabled();
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    if (helpers::InspectEnum(seq.m_representation_meta, {RepresentationMeta::NONE, RepresentationMeta::QUAT}))
                    {
                        if (seq.m_representation_meta == RepresentationMeta::COLOR)
                        {
                            SyncAllHandleTypesInCurve(seq, seq.m_curves.at(0).m_curve_handle_type, 3);
                        }
                    }

                    ImGui::Separator();

                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::VECTOR:
                        {
                            {
                                ImGui::PushID(0);
                                Curve& curve = seq.m_curves.at(0);
                                ImGui::Checkbox("X Lock All Handles", &curve.m_handle_type_locked);
                                if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "X All Handles' Type"))
                                {
                                    ApplyCurveHandleTypeOnCurve(curve);
                                }
                                ImGui::PopID();
                            }
                            {
                                ImGui::PushID(1);
                                Curve& curve = seq.m_curves.at(1);
                                ImGui::Checkbox("Y Lock All Handles", &curve.m_handle_type_locked);
                                if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "Y All Handles' Type"))
                                {
                                    ApplyCurveHandleTypeOnCurve(curve);
                                }
                                ImGui::PopID();
                            }
                            {
                                ImGui::PushID(2);
                                Curve& curve = seq.m_curves.at(2);
                                ImGui::Checkbox("Z Lock All Handles", &curve.m_handle_type_locked);
                                if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "Z All Handles' Type"))
                                {
                                    ApplyCurveHandleTypeOnCurve(curve);
                                }
                                ImGui::PopID();
                            }

                            ImGui::Separator();

                            if (curve_0_disabled)
                            {
                                ImGui::BeginDisabled();
                            }
                            if (ImGui::InputFloat((field_name_str + ".x").c_str(), &field.x) && !curve_0_disabled)
                            {
                                seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field.x});
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
                                seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {player_frame_f, field.y});
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
                                seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {player_frame_f, field.z});
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

                            {
                                Curve& curve = seq.m_curves.at(0);
                                ImGui::Checkbox("Lock All Curves' Handles", &curve.m_handle_type_locked);
                                if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "All Curves' Handles' Type"))
                                {
                                    SyncAllHandleTypesInCurve(seq, curve.m_curve_handle_type, 3);
                                }
                            }

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit3(field_name, &field.r) && !disabled)
                            {
                                seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field.r});
                                seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {player_frame_f, field.g});
                                seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {player_frame_f, field.b});
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
                                SyncAllHandleTypesInCurve(seq, CurveHandleType::LINEAR, 4);
                                SyncAllHandleTypeLocksInCurve(seq, false, 4);

                                break;
                            case RepresentationMeta::QUAT:
                                seq.m_curves.at(0).m_name = "W";
                                seq.m_curves.at(1).m_name = "X";
                                seq.m_curves.at(2).m_name = "Y";
                                seq.m_curves.at(3).m_name = "Z";
                                seq.m_curves.at(4).m_name = "Spins";

                                SyncAllHandleTypesInCurve(seq, CurveHandleType::LINEAR, 4);
                                SetCurveHandleType(seq.m_curves.at(4), CurveHandleType::CONSTANT);
                                SyncAllHandleTypeLocksInCurve(seq, true, 5);
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
                            {
                                Curve& curve = seq.m_curves.at(0);
                                ImGui::BeginDisabled();
                                ImGui::Checkbox("Lock All Curves' Handles", &curve.m_handle_type_locked);
                                ImGui::EndDisabled();
                                if (helpers::InspectEnum(curve.m_curve_handle_type,
                                                         {CurveHandleType::UNCONSTRAINED, CurveHandleType::AUTO},
                                                         "All Curves' Handles' Type"))
                                {
                                    SyncAllHandleTypesInCurve(seq, curve.m_curve_handle_type, 4);
                                }
                            }

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
                            {
                                Curve& curve = seq.m_curves.at(0);
                                ImGui::Checkbox("Lock All Curves' Handles", &curve.m_handle_type_locked);
                                if (helpers::InspectEnum(curve.m_curve_handle_type, {}, "All Curves' Handles' Type"))
                                {
                                    SyncAllHandleTypesInCurve(seq, curve.m_curve_handle_type, 4);
                                }
                            }

                            const bool disabled = curve_0_disabled || curve_1_disabled || curve_2_disabled || curve_3_disabled;

                            if (disabled)
                            {
                                ImGui::BeginDisabled();
                            }

                            if (ImGui::ColorEdit4(field_name, &field.r) && !disabled)
                            {
                                seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field.r});
                                seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {player_frame_f, field.g});
                                seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {player_frame_f, field.b});
                                seq.EditKeyframe(3, curve_3_optional_point_idx.value(), {player_frame_f, field.a});
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
                    {
                        Curve& curve = seq.m_curves.at(0);
                        ImGui::BeginDisabled();
                        ImGui::Checkbox("Lock All Curves' Handles", &curve.m_handle_type_locked);
                        ImGui::EndDisabled();
                        if (helpers::InspectEnum(curve.m_curve_handle_type,
                                                 {CurveHandleType::UNCONSTRAINED, CurveHandleType::AUTO},
                                                 "All Curves' Handles' Type"))
                        {
                            SyncAllHandleTypesInCurve(seq, curve.m_curve_handle_type, 4);
                        }
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
                        float spins = seq.m_curves.at(4).m_keyframes.at(curve_4_optional_point_idx.value()).Value();
                        int spins_int = (int)spins;
                        if (ImGui::InputInt("Spins", &spins_int))
                        {
                            spins = (float)spins_int;
                            seq.EditKeyframe(4, curve_4_optional_point_idx.value(), {player_frame_f, spins});

                            field = sequencer::SampleQuatForAnimation(seq, player_frame_f);

                            seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {player_frame_f, field.w});
                            seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {player_frame_f, field.x});
                            seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {player_frame_f, field.y});
                            seq.EditKeyframe(3, curve_3_optional_point_idx.value(), {player_frame_f, field.z});
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

                const auto& curve_0_optional_point_idx = seq.GetKeyframeIdx(0, recording_frame_f);
                const auto& curve_1_optional_point_idx = seq.GetKeyframeIdx(1, recording_frame_f);
                const auto& curve_2_optional_point_idx = seq.GetKeyframeIdx(2, recording_frame_f);
                const auto& curve_3_optional_point_idx = seq.GetKeyframeIdx(3, recording_frame_f);

                if constexpr (std::is_same_v<FieldType, float>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field});
                }
                else if constexpr (std::is_same_v<FieldType, int>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, (float)field});
                }
                else if constexpr (std::is_same_v<FieldType, bool>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field == true ? 1.0f : 0.0f});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec2>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.y});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec3>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.y});
                    seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.z});
                }
                else if constexpr (std::is_same_v<FieldType, glm::vec4>)
                {
                    switch (seq.m_representation_meta)
                    {
                        case RepresentationMeta::COLOR:
                            seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.r});
                            seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.g});
                            seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.b});
                            seq.EditKeyframe(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.a});
                            break;
                        case RepresentationMeta::QUAT:
                            seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.w});
                            seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.x});
                            seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.y});
                            seq.EditKeyframe(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.z});
                            break;
                        case RepresentationMeta::VECTOR:
                        case RepresentationMeta::NONE:
                        default:
                            assert(0);  // unhandled ReresentationMeta
                    }
                }
                else if constexpr (std::is_same_v<FieldType, glm::quat>)
                {
                    seq.EditKeyframe(0, curve_0_optional_point_idx.value(), {recording_frame_f, field.w});
                    seq.EditKeyframe(1, curve_1_optional_point_idx.value(), {recording_frame_f, field.x});
                    seq.EditKeyframe(2, curve_2_optional_point_idx.value(), {recording_frame_f, field.y});
                    seq.EditKeyframe(3, curve_3_optional_point_idx.value(), {recording_frame_f, field.z});
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
