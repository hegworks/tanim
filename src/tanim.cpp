// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#include "tanim/include/tanim.hpp"

#include "tanim/include/timeliner.hpp"
#include "tanim/include/sequence.hpp"

namespace tanim
{

void Tanim::Init()
{
    // sequence with default values
    m_timeline.m_first_frame = 0;
    m_timeline.m_last_frame = 500;
    m_timeline.m_sequence_datas.push_back(Timeline::SequenceData{0, 0, 10, true});
    m_timeline.m_sequence.TimelineLastFrameEdit(10);
}

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
        if (m_player_time > FrameToSeconds(m_timeline.m_sequence.m_timeline_last_frame))
        {
            m_player_time = 0;
        }
    }
}

void Tanim::Draw()
{
    m_timeline.m_sequence.m_draw_min.x = 0;
    m_timeline.m_sequence.m_draw_max.x = (float)m_timeline.m_last_frame;

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
    ImGui::DragInt("Samples", &m_player_samples, 0.1f, m_timeline.GetFirstFrame());

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    m_player_samples = ImMax(0, m_player_samples);
    ImGui::InputInt("Frame", &m_player_frame);
    m_player_frame = ImMax(0, m_player_frame);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::DragInt("MaxFrame", &m_timeline.m_last_frame, 0.1f, m_timeline.GetFirstFrame());
    m_timeline.m_last_frame = ImMax(1, m_timeline.m_last_frame);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    ImGui::SameLine();
    if (ImGui::DragFloat("Snap Y", &m_snap_y_value, 0.01f))
    {
        m_snap_y_value = ImMax(0.0f, m_snap_y_value);
        m_timeline.EditSnapY(m_snap_y_value);
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

    ImGui::Text("focused:     %d", m_timeline.focused);
    ImGui::Text("first frame: %d", m_timeline.m_first_frame);
    ImGui::Text("last frame:  %d", m_timeline.m_last_frame);

    ImGui::End();

    //*****************************************************

    ImGui::Begin("selected sequence");

    ImGui::Text("index:       %d", m_selected_sequence);

    ImGui::PushItemWidth(100);
    ImGui::BeginDisabled();
    ImGui::DragFloat2("draw min", &m_timeline.m_sequence.m_draw_min.x);
    ImGui::DragFloat2("draw max", &m_timeline.m_sequence.m_draw_max.x);
    ImGui::EndDisabled();
    ImGui::PopItemWidth();

    if (m_selected_sequence != -1)
    {
        const Timeline::SequenceData& sd = m_timeline.m_sequence_datas[m_selected_sequence];
        // switch (type) ...

        ImGui::Text("type:        %d", sd.m_type);
        ImGui::Text("type name:   %s", m_timeline.GetSequenceTypeName(sd.m_type));
        ImGui::Text("expanded:    %i", sd.m_expanded);
        ImGui::Text("first frame: %d", sd.m_first_frame);
        ImGui::Text("last frame:  %d", sd.m_last_frame);
    }

    ImGui::End();

    //*****************************************************

    ImGui::Begin("curves");

    float sampleTime = m_player_playing ? SecondsToSampleTime(m_player_time) : (float)m_player_frame;

    float sampledX = sequencer::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(0),
                                                        sampleTime,
                                                        m_timeline.m_sequence.GetCurveLerpType(0));

    float sampledY = sequencer::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(1),
                                                        sampleTime,
                                                        m_timeline.m_sequence.GetCurveLerpType(1));

    float sampledZ = sequencer::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(2),
                                                        sampleTime,
                                                        m_timeline.m_sequence.GetCurveLerpType(2));

    ImGui::Text("X: %.4f", sampledX);
    ImGui::Text("Y: %.4f", sampledY);
    ImGui::Text("Z: %.4f", sampledZ);

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
