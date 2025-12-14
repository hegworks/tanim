// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#include "tanim/include/tanim.hpp"

#include "tanim/include/registry.hpp"
#include "tanim/include/timeliner.hpp"
#include "tanim/include/sequence.hpp"

namespace tanim
{

void Tanim::Init() {}

void Tanim::UpdateEditor(float dt)
{
    if (!m_is_engine_in_play_mode)
    {
        if (m_editing_timeline_data != nullptr)
        {
            TimelineData& data = *m_editing_timeline_data;
            if (Timeline::GetPlayerPlaying(data))
            {
                Timeline::TickTime(data, dt);
                Sample(*m_editing_timeline_registry, m_editing_timeline_entity, data);
            }
        }
    }
}

void Tanim::OpenForEditing(entt::registry& registry, entt::entity entity, TimelineData& timeline_data)
{
    m_editing_timeline_data = &timeline_data;
    m_editing_timeline_registry = &registry;
    m_editing_timeline_entity = entity;
}

void Tanim::CloseEditor()
{
    m_editing_timeline_data = nullptr;
    m_editing_timeline_registry = nullptr;
    m_editing_timeline_entity = entt::null;
}

void Tanim::Sample(entt::registry& registry, entt::entity entity, TimelineData& data)
{
    const float sample_time = Timeline::GetPlayerPlaying(data) ? Timeline::GetPlayerSampleTime(data)
                                                               : static_cast<float>(Timeline::GetPlayerFrame(data));

    const auto& components = GetRegistry().GetComponents();

    for (int seq_idx = 0; seq_idx < Timeline::GetSequenceCount(data); ++seq_idx)
    {
        for (const auto& component : components)
        {
            Sequence& seq = Timeline::GetSequence(data, seq_idx);

            if (!seq.IsRecording() && component.HasSequence(seq.m_name))
            {
                component.m_sample(registry, entity, data, sample_time, seq);
            }
        }
    }
}

void Tanim::StartTimeline(TimelineData& data)
{
    Timeline::ResetPlayerTime(data);
    if (Timeline::GetPlayImmediately(data))
    {
        Timeline::Play(data);
    }
}

void Tanim::UpdateTimeline(entt::registry& registry, entt::entity entity, TimelineData& data, float delta_time)
{
    if (Timeline::GetPlayerPlaying(data))
    {
        Timeline::TickTime(data, delta_time);
        Sample(registry, entity, data);
    }
}

void Tanim::StopTimeline(TimelineData& data) { Timeline::Stop(data); }

bool Tanim::IsPlaying(const TimelineData& data) { return Timeline::GetPlayerPlaying(data); }

void Tanim::Play(TimelineData& data) { Timeline::Play(data); }

void Tanim::Pause(TimelineData& data) { Timeline::Pause(data); }

void Tanim::Stop(TimelineData& data) { Timeline::Stop(data); }

void Tanim::Draw()
{
    if (m_editing_timeline_data == nullptr)
    {
        return;
    }

    TimelineData& data = *m_editing_timeline_data;

    bool has_expanded_seq = false;
    int expanded_seq_idx = -1;
    if (const auto idx = Timeline::GetExpandedSequenceIdx(data))
    {
        has_expanded_seq = true;
        expanded_seq_idx = idx.value();
    }

    if (has_expanded_seq)
    {
        Timeline::SetDrawMaxX(data, expanded_seq_idx, static_cast<float>(Timeline::GetMaxFrame(data)));
    }

    //*****************************************************

    ImGui::Begin("Tanim");
    // empty parent window
    ImGui::End();

    //*****************************************************

    ImGui::Begin("controls");

    if (!Timeline::GetPlayerPlaying(data))
    {
        if (ImGui::Button("Play", {50, 0}))
        {
            Timeline::ResetPlayerTime(data);
            Timeline::Play(data);
        }
    }
    else
    {
        if (ImGui::Button("Pause", {50, 0}))
        {
            Timeline::Pause(data);
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::PushItemWidth(100);

    const bool disabled = Timeline::GetPlayerPlaying(data);
    if (disabled)
    {
        ImGui::BeginDisabled();
        m_preview = true;
    }
    if (ImGui::Checkbox("Preview", &m_preview))
    {
        if (m_preview == false)
        {
            m_preview = true;
            const int frame_before = Timeline::GetPlayerFrame(data);
            Timeline::ResetPlayerTime(data);
            Sample(*m_editing_timeline_registry, m_editing_timeline_entity, data);
            m_preview = false;
            Timeline::SetPlayerTimeFromFrame(data, frame_before);
        }
    }
    if (disabled)
    {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::DragInt("Samples", &data.m_player_samples, 0.1f, Timeline::GetMinFrame(data));
    data.m_player_samples = ImMax(1, data.m_player_samples);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    int player_frame = Timeline::GetPlayerFrame(data);
    if (ImGui::InputInt("Frame", &player_frame))
    {
        player_frame = ImMax(0, player_frame);
        Timeline::SetPlayerTimeFromFrame(data, player_frame);
        if (m_preview)
        {
            Sample(*m_editing_timeline_registry, m_editing_timeline_entity, data);
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::DragInt("MaxFrame", &data.m_max_frame, 0.1f, Timeline::GetMinFrame(data));
    data.m_max_frame = ImMax(1, data.m_max_frame);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::SameLine();
    if (ImGui::DragFloat("SnapY", &m_snap_y_value, 0.01f))
    {
        m_snap_y_value = ImMax(0.0f, m_snap_y_value);
        Timeline::EditSnapY(data, m_snap_y_value);
    }

    ImGui::PopItemWidth();

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeliner");

    constexpr int flags =
        timeliner::TIMELINER_CHANGE_FRAME | timeliner::TIMELINER_DELETE_SEQUENCE | timeliner::TIMELINER_EDIT_STARTEND;

    const int player_frame_before = player_frame;
    timeliner::Timeliner(data, &player_frame, &data.m_expanded, &data.m_selected_sequence, &data.m_first_frame, flags);
    const int player_frame_after = player_frame;

    if ((!m_is_engine_in_play_mode && !Timeline::GetPlayerPlaying(data)) || (player_frame_before != player_frame_after))
    {
        Timeline::SetPlayerTimeFromFrame(data, player_frame_after);
        if (m_preview)
        {
            Sample(*m_editing_timeline_registry, m_editing_timeline_entity, data);
        }
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeline");

    helpers::InspectEnum(data.m_playback_type);

    if (ImGui::Button("+ Sequence"))
    {
        ImGui::OpenPopup("AddSequencePopup");
    }

    if (ImGui::BeginPopup("AddSequencePopup"))
    {
        const auto& components = GetRegistry().GetComponents();
        for (const auto& component : components)
        {
            if (component.m_entity_has(*m_editing_timeline_registry, m_editing_timeline_entity))
            {
                for (const auto& field_name : component.m_field_names)
                {
                    const std::string full_name = component.m_struct_name + "::" + field_name;
                    if (!Timeline::HasSequenceWithName(data, full_name))
                    {
                        if (ImGui::MenuItem(full_name.c_str()))
                        {
                            component.m_add_sequence(*m_editing_timeline_registry, m_editing_timeline_entity, data, field_name);
                        }
                    }
                }
            }
        }

        ImGui::EndPopup();
    }

    char name_buf[256];
    strncpy_s(name_buf, Timeline::GetName(data).c_str(), sizeof(name_buf));
    if (ImGui::InputText("Timeline Name", name_buf, sizeof(name_buf)))
    {
        Timeline::SetName(data, std::string(name_buf));
    }

    ImGui::Checkbox("Play Immediately", &data.m_play_immediately);

    ImGui::Text("max frame:   %d", Timeline::GetMinFrame(data));
    ImGui::Text("last frame:  %d", Timeline::GetLastFrame(data));
    ImGui::Text("entity:      %llu", static_cast<uint64_t>(m_editing_timeline_entity));
    // ImGui::Text("id:          %llu", data.m_id);

    ImGui::End();

    //*****************************************************

    ImGui::Begin("expanded sequence");

    if (has_expanded_seq)
    {
        Sequence& seq = Timeline::GetSequence(data, expanded_seq_idx);

        const bool is_keyframe_in_all_curves = seq.IsKeyframeInAllCurves(Timeline::GetPlayerFrame(data));
        if (is_keyframe_in_all_curves || Timeline::GetPlayerPlaying(data))
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("+Keyframe"))
        {
            seq.AddNewKeyframe(Timeline::GetPlayerFrame(data));
            seq.StartRecording(Timeline::GetPlayerFrame(data));

            const auto& components = GetRegistry().GetComponents();
            for (const auto& component : components)
            {
                if (component.HasSequence(seq.m_name))
                {
                    component.m_record(*m_editing_timeline_registry, m_editing_timeline_entity, data, seq);
                }
            }

            seq.StopRecording();
        }
        if (is_keyframe_in_all_curves || Timeline::GetPlayerPlaying(data))
        {
            ImGui::EndDisabled();
        }

        if (seq.IsRecording())
        {
            const bool clicked_on_stop_recording = ImGui::Button("Stop Recording");
            const bool has_moved_player_frame = Timeline::GetPlayerFrame(data) != seq.GetRecordingFrame();
            const bool has_moved_recording_frame_x = !seq.IsKeyframeInAllCurves(seq.GetRecordingFrame());
            if (clicked_on_stop_recording || has_moved_player_frame || has_moved_recording_frame_x)
            {
                seq.StopRecording();
            }
            else
            {
                const auto& components = GetRegistry().GetComponents();
                for (const auto& component : components)
                {
                    if (component.HasSequence(seq.m_name))
                    {
                        component.m_record(*m_editing_timeline_registry, m_editing_timeline_entity, data, seq);
                    }
                }
            }
        }
        else
        {
            if (Timeline::GetPlayerPlaying(data))
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Record"))
            {
                seq.AddNewKeyframe(Timeline::GetPlayerFrame(data));
                seq.StartRecording(Timeline::GetPlayerFrame(data));
            }
            if (Timeline::GetPlayerPlaying(data))
            {
                ImGui::EndDisabled();
            }
        }

        ImGui::PushItemWidth(100);
        ImGui::Text("index:       %d", expanded_seq_idx);
        ImGui::BeginDisabled();
        ImGui::DragFloat2("draw min", &seq.m_draw_min.x);
        ImGui::DragFloat2("draw max", &seq.m_draw_max.x);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();

        ImGui::Text("expanded:    %i", seq.m_expanded);

        for (int i = 0; i < seq.GetCurveCount(); ++i)
        {
            ImGui::PushID(i);

            char curve_name_buf[256];
            strncpy_s(curve_name_buf, seq.m_curves.at(i).m_name.c_str(), sizeof(curve_name_buf));
            if (ImGui::InputText("Field", curve_name_buf, sizeof(curve_name_buf)))
            {
                seq.m_curves.at(i).m_name = std::string(curve_name_buf);
            }

            ImGui::PopID();
        }
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("curves");

    const auto& components = GetRegistry().GetComponents();

    for (int seq_idx = 0; seq_idx < Timeline::GetSequenceCount(data); ++seq_idx)
    {
        Sequence& seq = Timeline::GetSequence(data, seq_idx);
        const bool is_recording = seq.IsRecording();
        for (const auto& component : components)
        {
            if (component.HasSequence(seq.m_name))
            {
                if (is_recording)
                {
                    ImGui::Text("RECORDING");
                    ImGui::BeginDisabled();
                }

                ImGui::Text("%s", component.m_struct_name.c_str());
                component.m_inspect(*m_editing_timeline_registry, m_editing_timeline_entity, data, seq);
                ImGui::Separator();

                if (is_recording)
                {
                    ImGui::EndDisabled();
                }
            }
        }
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("Player");

    ImGui::Text("Real Time:    %.3fs", Timeline::GetPlayerRealTime(data));
    ImGui::Text("Sample Time:  %.3fs", Timeline::GetPlayerSampleTime(data));
    ImGui::Text("Frame:        %d", Timeline::GetPlayerFrame(data));

    ImGui::End();

    //*****************************************************
}

std::string Tanim::Serialize(TimelineData& data)
{
    nlohmann::ordered_json json{};

    json["about"] = {"Tanim Serialized JSON", "more info: https://github.com/hegworks/tanim"};
    json["version"] = 1;

    nlohmann::ordered_json timeline_js{};
    timeline_js["m_name"] = data.m_name;
    timeline_js["m_first_frame"] = data.m_first_frame;
    timeline_js["m_last_frame"] = data.m_last_frame;
    timeline_js["m_min_frame"] = data.m_min_frame;
    timeline_js["m_max_frame"] = data.m_max_frame;
    timeline_js["m_play_immediately"] = data.m_play_immediately;
    timeline_js["m_player_samples"] = data.m_player_samples;
    timeline_js["m_playback_type"] = std::string(magic_enum::enum_name(data.m_playback_type));

    nlohmann::ordered_json sequences_js_array = nlohmann::ordered_json::array();
    for (int seq_idx = 0; seq_idx < static_cast<int>(data.m_sequences.size()); ++seq_idx)
    {
        nlohmann::ordered_json seq_js{};

        Sequence& seq = data.m_sequences.at(seq_idx);
        seq_js["m_name"] = seq.m_name;
        seq_js["m_type_meta"] = std::string(magic_enum::enum_name(seq.m_type_meta));
        seq_js["m_representation_meta"] = std::string(magic_enum::enum_name(seq.m_representation_meta));

        nlohmann::ordered_json curves_js_array = nlohmann::ordered_json::array();
        for (int curve_idx = 0; curve_idx < seq.GetCurveCount(); ++curve_idx)
        {
            nlohmann::ordered_json curve_js{};

            Sequence::Curve& curve = seq.m_curves.at(curve_idx);
            curve_js["m_name"] = curve.m_name;
            curve_js["m_lerp_type"] = std::string(magic_enum::enum_name(curve.m_lerp_type));

            nlohmann::ordered_json pts_js_array = nlohmann::ordered_json::array();
            for (int pt_idx = 0; pt_idx < seq.GetCurvePointCount(curve_idx); ++pt_idx)
            {
                const auto& pt = curve.m_points.at(pt_idx);
                pts_js_array.push_back({pt.x, pt.y});
            }
            curve_js["m_points"] = pts_js_array;

            curves_js_array.push_back(curve_js);
        }

        seq_js["m_curves"] = curves_js_array;
        sequences_js_array.push_back(seq_js);
    }

    timeline_js["m_sequences"] = sequences_js_array;
    json["timeline_data"] = timeline_js;

    return json.dump(4);
}

void Tanim::Deserialize(TimelineData& data, const std::string& serialized_string)
{
    assert(!serialized_string.empty());
    const nlohmann::ordered_json json = nlohmann::ordered_json::parse(serialized_string);
    assert(!json.empty());

    const auto& timeline_js = json["timeline_data"];
    data.m_name = timeline_js["m_name"].get<std::string>();
    data.m_first_frame = timeline_js["m_first_frame"].get<int>();
    data.m_last_frame = timeline_js["m_last_frame"].get<int>();
    data.m_min_frame = timeline_js["m_min_frame"].get<int>();
    data.m_max_frame = timeline_js["m_max_frame"].get<int>();
    data.m_play_immediately = timeline_js["m_play_immediately"].get<bool>();
    data.m_player_samples = timeline_js["m_player_samples"].get<int>();

    const std::string playback_type_str = timeline_js["m_playback_type"].get<std::string>();
    data.m_playback_type = magic_enum::enum_cast<PlaybackType>(playback_type_str).value_or(PlaybackType::HOLD);

    for (const auto& seq_js : timeline_js["m_sequences"])
    {
        Sequence& seq = data.m_sequences.emplace_back(&data.m_last_frame);

        seq.m_name = seq_js["m_name"].get<std::string>();

        const std::string type_meta_str = seq_js["m_type_meta"].get<std::string>();
        seq.m_type_meta = magic_enum::enum_cast<Sequence::TypeMeta>(type_meta_str).value_or(Sequence::TypeMeta::NONE);

        const std::string representation_meta_str = seq_js["m_representation_meta"].get<std::string>();
        seq.m_representation_meta =
            magic_enum::enum_cast<RepresentationMeta>(representation_meta_str).value_or(RepresentationMeta::NONE);

        seq.m_curves.clear();
        for (const auto& curve_js : seq_js["m_curves"])
        {
            Sequence::Curve& curve = seq.m_curves.emplace_back();

            curve.m_name = curve_js["m_name"].get<std::string>();

            const std::string lerp_type_str = curve_js["m_lerp_type"].get<std::string>();
            curve.m_lerp_type = magic_enum::enum_cast<sequencer::LerpType>(lerp_type_str).value_or(sequencer::LerpType::SMOOTH);

            curve.m_points.clear();
            for (const auto& pt_js : curve_js["m_points"])
            {
                const float x = pt_js[0].get<float>();
                const float y = pt_js[1].get<float>();
                curve.m_points.emplace_back(x, y);
            }
        }
    }
}

}  // namespace tanim
