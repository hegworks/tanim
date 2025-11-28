// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#include "tanim/include/tanim.hpp"

#include "tanim/include/timeliner.hpp"
#include "tanim/include/sequence.hpp"

namespace tanim
{

void Tanim::Init() {}

void Tanim::Play() { m_player_playing = true; }

void Tanim::Pause() { m_player_playing = false; }

int Tanim::SecondsToFrame(float time) { return (int)floorf(time * (float)m_player_samples); }

float Tanim::SecondsToSampleTime(float time) { return time * (float)m_player_samples; }

float Tanim::FrameToSeconds(int frame)
{
    float tickTime = 1.0f / (float)m_player_samples;
    return (float)frame * tickTime;
}

void Tanim::Update(float dt)
{
    if (m_player_playing)
    {
        m_player_time += dt;
        if (m_player_time > FrameToSeconds(m_timeline.m_last_frame))
        {
            m_player_time = 0;
        }
    }
}

void Tanim::Draw()
{
    bool has_expanded_seq = false;
    int expanded_seq_idx = -1;
    if (const auto idx = m_timeline.GetExpandedSequenceIdx())
    {
        has_expanded_seq = true;
        expanded_seq_idx = idx.value();
    }

    if (has_expanded_seq)
    {
        m_timeline.GetSequence(expanded_seq_idx).m_draw_max.x = (float)m_timeline.m_max_frame;
    }

    //*****************************************************

    ImGui::Begin("Tanim");
    // empty parent window
    ImGui::End();

    //*****************************************************

    ImGui::Begin("controls");

    if (!m_player_playing)
    {
        if (ImGui::Button("Play", {50, 0}))
        {
            m_player_time = 0;
            Play();
        }
    }
    else
    {
        if (ImGui::Button("Pause", {50, 0}))
        {
            Pause();
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    ImGui::DragInt("Samples", &m_player_samples, 0.1f, m_timeline.GetMinFrame());

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    m_player_samples = ImMax(0, m_player_samples);
    ImGui::InputInt("Frame", &m_player_frame);
    m_player_frame = ImMax(0, m_player_frame);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::DragInt("MaxFrame", &m_timeline.m_max_frame, 0.1f, m_timeline.GetMinFrame());
    m_timeline.m_max_frame = ImMax(1, m_timeline.m_max_frame);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::SameLine();
    if (ImGui::DragFloat("SnapY", &m_snap_y_value, 0.01f))
    {
        m_snap_y_value = ImMax(0.0f, m_snap_y_value);
        m_timeline.EditSnapY(m_snap_y_value);
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (ImGui::Button("+ Timeline"))
    {
        // TODO(tanim) save the previous timeline, create the new one after getting the path in drive
        m_timeline = {};
        has_expanded_seq = false;
    }

    ImGui::PopItemWidth();

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeliner");

    static bool expanded{true};
    static int first_frame{0};
    timeliner::Timeliner(&m_timeline, &m_player_frame, &expanded, &m_selected_sequence, &first_frame, timeliner::TIMELINER_ALL);

    if (m_player_playing)
    {
        m_player_frame = SecondsToFrame(m_player_time);
    }
    else
    {
        m_player_time = FrameToSeconds(m_player_frame);
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("timeline");

    if (ImGui::Button("+ Sequence"))
    {
        // TODO(tanim) get the types from reflection and show a pop-up to choose between then, then AddSequence
        m_timeline.AddSequence(0);
    }

    char name_buf[256];
    strncpy_s(name_buf, m_timeline.m_name.c_str(), sizeof(name_buf));
    if (ImGui::InputText("Timeline Name", name_buf, sizeof(name_buf)))
    {
        m_timeline.m_name = std::string(name_buf);
    }

    ImGui::Text("focused:     %d", m_timeline.focused);
    ImGui::Text("min frame:   %d", m_timeline.m_min_frame);
    ImGui::Text("max frame:   %d", m_timeline.m_max_frame);
    ImGui::Text("first frame: %d", m_timeline.m_first_frame);
    ImGui::Text("last frame:  %d", m_timeline.m_last_frame);

    ImGui::End();

    //*****************************************************

    ImGui::Begin("expanded sequence");

    if (has_expanded_seq)
    {
        Sequence& seq = m_timeline.GetSequence(expanded_seq_idx);

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

    if (has_expanded_seq)
    {
        Sequence& seq = m_timeline.GetSequence(expanded_seq_idx);

        const float sampleTime = m_player_playing ? SecondsToSampleTime(m_player_time) : (float)m_player_frame;

        const float sampledX =
            sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(0), sampleTime, seq.GetCurveLerpType(0));

        const float sampledY =
            sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(1), sampleTime, seq.GetCurveLerpType(1));

        const float sampledZ =
            sequencer::SampleCurveForAnimation(seq.GetCurvePointsList(2), sampleTime, seq.GetCurveLerpType(2));

        ImGui::Text("X: %.4f", sampledX);
        ImGui::Text("Y: %.4f", sampledY);
        ImGui::Text("Z: %.4f", sampledZ);
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("Player");

    ImGui::Text("Time:         %.3f", m_player_time);
    ImGui::Text("Sample Time:  %.3f", SecondsToSampleTime(m_player_time));
    ImGui::Text("Frame:        %d", m_player_frame);

    ImGui::End();

    //*****************************************************
}

}  // namespace tanim
