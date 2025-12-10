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
        if (m_timeline.HasData() && m_timeline.GetPlayerPlaying())
        {
            m_timeline.TickTime(dt);
            Sample(m_timeline.m_data);
        }
    }
}

void Tanim::SetTimelineData(TimelineData* timeline_data) { m_timeline.m_data = timeline_data; }

void Tanim::Sample(TimelineData* timeline_data)
{
    m_timeline.m_data = timeline_data;

    const float sampleTime =
        m_timeline.GetPlayerPlaying() ? m_timeline.m_data->PlayerSampleTime() : (float)m_timeline.m_data->PlayerFrame();
    const auto& components = GetRegistry().GetComponents();

    for (int seq_idx = 0; seq_idx < m_timeline.GetSequenceCount(); ++seq_idx)
    {
        for (const auto& component : components)
        {
            Sequence& seq = m_timeline.GetSequence(seq_idx);

            if (!seq.IsRecording() && component.HasSequence(seq.m_name))
            {
                component.m_sample(m_timeline, sampleTime, seq);
            }
        }
    }
}

void Tanim::StartTimeline(TimelineData* timeline_data)
{
    timeline_data->SetPlayerTimeFromSeconds(0);
    if (timeline_data->m_play_immediately)
    {
        Play(timeline_data);
    }
}

void Tanim::UpdateTimeline(TimelineData* timeline_data, float delta_time)
{
    m_timeline.m_data = timeline_data;
    if (m_timeline.GetPlayerPlaying())
    {
        m_timeline.TickTime(delta_time);
        Sample(timeline_data);
    }
}

void Tanim::StopTimeline(TimelineData* timeline_data)
{
    m_timeline.m_data = timeline_data;
    m_timeline.Stop();
}

bool Tanim::IsPlaying(const TimelineData* timeline_data) { return timeline_data->m_player_playing; }

void Tanim::Play(TimelineData* timeline_data)
{
    m_timeline.m_data = timeline_data;
    m_timeline.Play();
}

void Tanim::Pause(TimelineData* timeline_data)
{
    m_timeline.m_data = timeline_data;
    m_timeline.Pause();
}

void Tanim::Stop(TimelineData* timeline_data)
{
    m_timeline.m_data = timeline_data;
    m_timeline.Stop();
}

void Tanim::Draw()
{
    if (!m_timeline.HasData())
    {
        return;
    }

    bool has_expanded_seq = false;
    int expanded_seq_idx = -1;
    if (const auto idx = m_timeline.GetExpandedSequenceIdx())
    {
        has_expanded_seq = true;
        expanded_seq_idx = idx.value();
    }

    if (has_expanded_seq)
    {
        m_timeline.GetSequence(expanded_seq_idx).m_draw_max.x = (float)m_timeline.GetMaxFrame();
    }

    //*****************************************************

    ImGui::Begin("Tanim");
    // empty parent window
    ImGui::End();

    //*****************************************************

    ImGui::Begin("controls");

    if (!m_timeline.GetPlayerPlaying())
    {
        if (ImGui::Button("Play", {50, 0}))
        {
            m_timeline.m_data->SetPlayerTimeFromSeconds(0.0f);
            m_timeline.Play();
        }
    }
    else
    {
        if (ImGui::Button("Pause", {50, 0}))
        {
            m_timeline.Pause();
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::PushItemWidth(100);

    const bool disabled = m_timeline.GetPlayerPlaying();
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
            const int frame_before = m_timeline.GetPlayerFrame();
            m_timeline.m_data->SetPlayerTimeFromSeconds(0);
            Sample(m_timeline.m_data);
            m_preview = false;
            m_timeline.m_data->SetPlayerTimeFromFrame(frame_before);
        }
    }
    if (disabled)
    {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::DragInt("Samples", &m_timeline.m_data->m_player_samples, 0.1f, m_timeline.GetMinFrame());

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    m_timeline.m_data->m_player_samples = ImMax(0, m_timeline.m_data->m_player_samples);
    int player_frame = m_timeline.m_data->PlayerFrame();
    if (ImGui::InputInt("Frame", &player_frame))
    {
        player_frame = ImMax(0, player_frame);
        m_timeline.m_data->SetPlayerTimeFromFrame(player_frame);
        if (m_preview)
        {
            Sample(m_timeline.m_data);
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    int max_frame = m_timeline.GetMaxFrame();
    ImGui::DragInt("MaxFrame", &max_frame, 0.1f, m_timeline.GetMinFrame());
    m_timeline.SetMaxFrame(ImMax(1, max_frame));

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::SameLine();
    if (ImGui::DragFloat("SnapY", &m_snap_y_value, 0.01f))
    {
        m_snap_y_value = ImMax(0.0f, m_snap_y_value);
        m_timeline.EditSnapY(m_snap_y_value);
    }

    /*ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();*/

    // if (ImGui::Button("+ Timeline"))
    //{
    //     // TODO(tanim) save the previous timeline, create the new one after getting the path in drive
    //     m_timeline = {};
    //     has_expanded_seq = false;
    // }

    ImGui::PopItemWidth();

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeliner");

    static bool expanded{true};
    static int first_frame{0};
    static int m_selected_sequence{-1};
    const int player_frame_before = player_frame;
    timeliner::Timeliner(&m_timeline, &player_frame, &expanded, &m_selected_sequence, &first_frame, timeliner::TIMELINER_ALL);
    const int player_frame_after = player_frame;

    if ((!m_is_engine_in_play_mode && !m_timeline.GetPlayerPlaying()) || (player_frame_before != player_frame_after))
    {
        m_timeline.m_data->SetPlayerTimeFromFrame(player_frame_after);
        if (m_preview)
        {
            Sample(m_timeline.m_data);
        }
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeline");

    helpers::InspectEnum(m_timeline.m_data->m_playback_type);

    if (ImGui::Button("+ Sequence"))
    {
        ImGui::OpenPopup("AddSequencePopup");
    }

    if (ImGui::BeginPopup("AddSequencePopup"))
    {
        const auto& components = GetRegistry().GetComponents();
        for (const auto& component : components)
        {
            if (component.m_entity_has(m_timeline.m_data->m_entity))
            {
                for (const auto& field_name : component.m_field_names)
                {
                    const std::string full_name = component.m_struct_name + "::" + field_name;
                    if (!m_timeline.HasSequenceWithName(full_name))
                    {
                        if (ImGui::MenuItem(full_name.c_str()))
                        {
                            component.m_add_sequence(m_timeline, field_name);
                        }
                    }
                }
            }
        }

        ImGui::EndPopup();
    }

    char name_buf[256];
    strncpy_s(name_buf, m_timeline.GetName().c_str(), sizeof(name_buf));
    if (ImGui::InputText("Timeline Name", name_buf, sizeof(name_buf)))
    {
        m_timeline.SetName(std::string(name_buf));
    }

    ImGui::Checkbox("Play Immediately", &m_timeline.m_data->m_play_immediately);

    // ImGui::Text("focused:     %d", m_timeline.focused);
    // ImGui::Text("min frame:   %d", m_timeline.GetMinFrame());
    ImGui::Text("max frame:   %d", m_timeline.GetMaxFrame());
    // ImGui::Text("first frame: %d", m_timeline.GetFirstFrame());
    ImGui::Text("last frame:  %d", m_timeline.GetLastFrame());
    ImGui::Text("entity:      %llu", static_cast<uint64_t>(m_timeline.m_data->m_entity));
    ImGui::Text("id:          %llu", m_timeline.m_data->m_id);

    ImGui::End();

    //*****************************************************

    ImGui::Begin("expanded sequence");

    if (has_expanded_seq)
    {
        Sequence& seq = m_timeline.GetSequence(expanded_seq_idx);

        const bool is_keyframe_in_all_curves = seq.IsKeyframeInAllCurves(m_timeline.GetPlayerFrame());
        if (is_keyframe_in_all_curves)
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("+Keyframe"))
        {
            seq.AddNewKeyframe(m_timeline.GetPlayerFrame());
            seq.StartRecording(m_timeline.GetPlayerFrame());

            const auto& components = GetRegistry().GetComponents();
            for (const auto& component : components)
            {
                if (component.HasSequence(seq.m_name))
                {
                    component.m_record(m_timeline, seq);
                }
            }

            seq.StopRecording();
        }
        if (is_keyframe_in_all_curves)
        {
            ImGui::EndDisabled();
        }

        if (seq.IsRecording())
        {
            const bool clicked_on_stop_recording = ImGui::Button("Stop Recording");
            const bool has_moved_player_frame = m_timeline.GetPlayerFrame() != seq.GetRecordingFrame();
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
                        component.m_record(m_timeline, seq);
                    }
                }
            }
        }
        else
        {
            if (ImGui::Button("Record"))
            {
                seq.AddNewKeyframe(m_timeline.GetPlayerFrame());
                seq.StartRecording(m_timeline.GetPlayerFrame());
            }
        }

        ImGui::PushItemWidth(100);
        ImGui::Text("index:       %d", expanded_seq_idx);
        ImGui::BeginDisabled();
        ImGui::DragFloat2("draw min", &seq.m_draw_min.x);
        ImGui::DragFloat2("draw max", &seq.m_draw_max.x);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();

        // switch (type) ...

        ImGui::Text("type:        %d", seq.m_type);
        ImGui::Text("type name:   %s", m_timeline.GetSequenceTypeName(seq.m_type));
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

    for (int seq_idx = 0; seq_idx < m_timeline.GetSequenceCount(); ++seq_idx)
    {
        for (const auto& component : components)
        {
            Sequence& seq = m_timeline.GetSequence(seq_idx);
            const bool is_recording = seq.IsRecording();

            if (component.HasSequence(seq.m_name))
            {
                if (is_recording)
                {
                    ImGui::Text("RECORDING");
                    ImGui::BeginDisabled();
                }

                ImGui::Text("%s", component.m_struct_name.c_str());
                component.m_inspect(m_timeline, seq);
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

    ImGui::Text("Time:         %.3f", m_timeline.m_data->m_player_time);
    ImGui::Text("Sample Time:  %.3f", m_timeline.m_data->PlayerSampleTime());
    ImGui::Text("Frame:        %d", m_timeline.m_data->PlayerFrame());

    ImGui::End();

    //*****************************************************
}

}  // namespace tanim
