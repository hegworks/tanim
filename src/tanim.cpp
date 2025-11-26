#include "tanim/include/tanim.hpp"

#include "tanim/include/curve_editor.hpp"
#include "tanim/include/sequence.hpp"

// REF: initially based on the imguizmo example, but changed a lot over time
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

namespace tanim
{

void Tanim::Init()
{
    // sequence with default values
    mySequence.mFrameMin = 0;
    mySequence.mFrameMax = 500;
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{0, 0, 10, true});
    mySequence.rampEdit.SequenceFrameEndEdit(10);
}

void Tanim::Play() { playerPlaying = true; }

void Tanim::Pause() { playerPlaying = false; }

int Tanim::SecondsToFrame(float time) { return (int)floorf(time * (float)samplesPS); }

float Tanim::SecondsToSampleTime(float time) { return time * (float)samplesPS; }

float Tanim::FrameToSeconds(int frame)
{
    float tickTime = 1.0f / (float)samplesPS;
    return (float)frame * tickTime;
}

void Tanim::Update(float dt)
{
    if (playerPlaying)
    {
        playerTime += dt;
        if (playerTime > FrameToSeconds(mySequence.rampEdit.mSequenceFrameEnd))
        {
            playerTime = 0;
        }
    }
}

void Tanim::Draw()
{
    mySequence.rampEdit.mMin.x = 0;
    mySequence.rampEdit.mMax.x = (float)mySequence.mFrameMax;

    ImGui::Begin("tanim_timeline");

    ImGui::PushItemWidth(100);

    ImGui::InputInt("Frame", &currentFrame);
    currentFrame = ImMax(0, currentFrame);
    ImGui::SameLine();
    ImGui::DragInt("MaxFrame", &mySequence.mFrameMax, 0.1f, mySequence.GetFrameMin());
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::DragInt("Samples", &samplesPS, 0.1f, mySequence.GetFrameMin());
    ImGui::SameLine();
    if (!playerPlaying)
    {
        if (ImGui::SmallButton("Play"))
        {
            playerTime = 0;
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
    ImGui::Text("Time %.2f", playerTime);

    ImGui::PopItemWidth();

    sequence_editor::Sequencer(&mySequence,
                                     &currentFrame,
                                     &expanded,
                                     &selectedEntry,
                                     &firstFrame,
                                     sequence_editor::SEQUENCER_EDIT_ALL | sequence_editor::SEQUENCER_ADD |
                                         sequence_editor::SEQUENCER_DEL | sequence_editor::SEQUENCER_COPYPASTE |
                                         sequence_editor::SEQUENCER_CHANGE_FRAME);

    ImGui::End();

    if (playerPlaying)
    {
        currentFrame = SecondsToFrame(playerTime);
    }
    else
    {
        playerTime = FrameToSeconds(currentFrame);
    }

    ImGui::Begin("tanim_sequence");

    ImGui::Text("mySequence.focused:   %d", mySequence.focused);
    ImGui::Text("mySequence.mFrameMin: %d", mySequence.mFrameMin);
    ImGui::Text("mySequence.mFrameMax: %d", mySequence.mFrameMax);
    ImGui::Text("mySequence.myItems.size: %zu", mySequence.myItems.size());

    ImGui::End();

    ImGui::Begin("tanim_sequence_item");

    // add a UI to edit that particular item
    ImGui::Text("selected entry: %d", selectedEntry);
    if (selectedEntry != -1)
    {
        const TanimSequence::MySequenceItem& item = mySequence.myItems[selectedEntry];

        ImGui::Text("SequencerItemTypeName: %s", SequencerItemTypeNames[item.mType]);
        // switch (type) ...

        ImGui::Text("item.mType:       %d", item.mType);
        ImGui::Text("item.mExpanded:   %i", item.mExpanded);
        ImGui::Text("item.mFrameStart: %d", item.mFrameStart);
        ImGui::Text("item.mFrameEnd:   %d", item.mFrameEnd);

        float sampleTime = playerPlaying ? SecondsToSampleTime(playerTime) : (float)currentFrame;

        float sampledX = curve_editor::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(0),
                                                                         mySequence.rampEdit.GetPointCount(0),
                                                                         sampleTime,
                                                                         mySequence.rampEdit.GetCurveType(0));

        float sampledY = curve_editor::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(1),
                                                                         mySequence.rampEdit.GetPointCount(1),
                                                                         sampleTime,
                                                                         mySequence.rampEdit.GetCurveType(1));

        float sampledZ = curve_editor::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(2),
                                                                         mySequence.rampEdit.GetPointCount(2),
                                                                         sampleTime,
                                                                         mySequence.rampEdit.GetCurveType(2));
        ImGui::Text("X: %.4f", sampledX);
        ImGui::Text("Y: %.4f", sampledY);
        ImGui::Text("Z: %.4f", sampledZ);
    }

    ImGui::End();

    ImGui::Begin("tanim_ramp_edit");

    ImGui::DragFloat2("mMin", &mySequence.rampEdit.mMin.x);
    ImGui::DragFloat2("mMax", &mySequence.rampEdit.mMax.x);

    ImGui::End();
}

}  // namespace tanim
