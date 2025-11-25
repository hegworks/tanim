#include "../include/tanim.hpp"

#include "../include/tanimguizmo/tanimguizmo_curve_edit.h"
#include "../include/tanim_sequence.hpp"

// REF: initially based on imguizmo example
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

namespace tanim
{

void Tanim::Init()
{
    // sequence with default values
    mySequence.mFrameMin = 0;
    mySequence.mFrameMax = 500;
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{0, 0, 10, true});
}

void Tanim::Play() { playerPlaying = true; }

void Tanim::Pause() { playerPlaying = false; }

void Tanim::Update(float dt)
{
    if (playerPlaying)
    {
        playerTime += dt;
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

    tanimguizmo_sequencer::Sequencer(&mySequence,
                                     &currentFrame,
                                     &expanded,
                                     &selectedEntry,
                                     &firstFrame,
                                     tanimguizmo_sequencer::SEQUENCER_EDIT_ALL | tanimguizmo_sequencer::SEQUENCER_ADD |
                                         tanimguizmo_sequencer::SEQUENCER_DEL | tanimguizmo_sequencer::SEQUENCER_COPYPASTE |
                                         tanimguizmo_sequencer::SEQUENCER_CHANGE_FRAME);

    ImGui::End();

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

        float sampledX = tanimguizmo_curve_edit::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(0),
                                                                         mySequence.rampEdit.GetPointCount(0),
                                                                         (float)currentFrame,
                                                                         mySequence.rampEdit.GetCurveType(0));

        float sampledY = tanimguizmo_curve_edit::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(1),
                                                                         mySequence.rampEdit.GetPointCount(1),
                                                                         (float)currentFrame,
                                                                         mySequence.rampEdit.GetCurveType(1));

        float sampledZ = tanimguizmo_curve_edit::SampleCurveForAnimation(mySequence.rampEdit.GetPoints(2),
                                                                         mySequence.rampEdit.GetPointCount(2),
                                                                         (float)currentFrame,
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
