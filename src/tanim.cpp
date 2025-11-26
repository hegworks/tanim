// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#include "tanim/include/tanim.hpp"

#include "tanim/include/curve_editor.hpp"
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
    m_timeline.m_sequence.m_min_point_value.x = 0;
    m_timeline.m_sequence.m_max_point_value.x = (float)m_timeline.m_last_frame;

    ImGui::Begin("tanim_timeline");

    ImGui::PushItemWidth(100);

    ImGui::InputInt("Frame", &m_player_frame);
    m_player_frame = ImMax(0, m_player_frame);
    ImGui::SameLine();
    ImGui::DragInt("MaxFrame", &m_timeline.m_last_frame, 0.1f, m_timeline.GetFirstFrame());
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::DragInt("Samples", &m_player_samples, 0.1f, m_timeline.GetFirstFrame());
    ImGui::SameLine();
    if (!m_player_playing)
    {
        if (ImGui::SmallButton("Play"))
        {
            m_player_time = 0;
            Play();
        }
    }
    else
    {
        if (ImGui::SmallButton("Pause"))
        {
            Pause();
        }
    }
    ImGui::SameLine();
    ImGui::Text("Time %.2f", m_player_time);

    ImGui::PopItemWidth();

    static bool expanded{true};
    static int first_frame{0};
    sequence_editor::Sequencer(&m_timeline,
                               &m_player_frame,
                               &expanded,
                               &m_selected_sequence,
                               &first_frame,
                               sequence_editor::SEQUENCER_EDIT_ALL | sequence_editor::SEQUENCER_ADD |
                                   sequence_editor::SEQUENCER_DEL | sequence_editor::SEQUENCER_COPYPASTE |
                                   sequence_editor::SEQUENCER_CHANGE_FRAME);

    ImGui::End();

    if (m_player_playing)
    {
        m_player_frame = SecondsToFrame(m_player_time);
    }
    else
    {
        m_player_time = FrameToSeconds(m_player_frame);
    }

    ImGui::Begin("tanim_sequence");

    ImGui::Text("mySequence.focused:   %d", m_timeline.focused);
    ImGui::Text("mySequence.mFrameMin: %d", m_timeline.m_first_frame);
    ImGui::Text("mySequence.mFrameMax: %d", m_timeline.m_last_frame);
    ImGui::Text("mySequence.myItems.size: %zu", m_timeline.m_sequence_datas.size());

    ImGui::End();

    ImGui::Begin("tanim_sequence_item");

    // add a UI to edit that particular item
    ImGui::Text("selected entry: %d", m_selected_sequence);
    if (m_selected_sequence != -1)
    {
        const Timeline::SequenceData& item = m_timeline.m_sequence_datas[m_selected_sequence];

        ImGui::Text("SequencerItemTypeName: %s", SequenceTypeNames[item.m_type]);
        // switch (type) ...

        ImGui::Text("item.mType:       %d", item.m_type);
        ImGui::Text("item.mExpanded:   %i", item.m_expanded);
        ImGui::Text("item.mFrameStart: %d", item.m_first_frame);
        ImGui::Text("item.mFrameEnd:   %d", item.m_last_frame);

        float sampleTime = m_player_playing ? SecondsToSampleTime(m_player_time) : (float)m_player_frame;

        float sampledX = curve_editor::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(0),
                                                               m_timeline.m_sequence.GetCurvePointCount(0),
                                                               sampleTime,
                                                               m_timeline.m_sequence.GetCurveLerpType(0));

        float sampledY = curve_editor::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(1),
                                                               m_timeline.m_sequence.GetCurvePointCount(1),
                                                               sampleTime,
                                                               m_timeline.m_sequence.GetCurveLerpType(1));

        float sampledZ = curve_editor::SampleCurveForAnimation(m_timeline.m_sequence.GetCurvePointsList(2),
                                                               m_timeline.m_sequence.GetCurvePointCount(2),
                                                               sampleTime,
                                                               m_timeline.m_sequence.GetCurveLerpType(2));
        ImGui::Text("X: %.4f", sampledX);
        ImGui::Text("Y: %.4f", sampledY);
        ImGui::Text("Z: %.4f", sampledZ);
    }

    ImGui::End();

    ImGui::Begin("tanim_ramp_edit");

    ImGui::DragFloat2("mMin", &m_timeline.m_sequence.m_min_point_value.x);
    ImGui::DragFloat2("mMax", &m_timeline.m_sequence.m_max_point_value.x);

    ImGui::End();
}

}  // namespace tanim
