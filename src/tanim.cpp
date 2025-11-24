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
    mySequence.mFrameMax = 1000;
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{0, 10, 30, false});
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{1, 20, 30, true});
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{3, 12, 60, false});
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{2, 61, 90, false});
    mySequence.myItems.push_back(TanimSequence::MySequenceItem{4, 90, 99, false});
}

void Tanim::Draw()
{
    mySequence.rampEdit.mMin.x = 0;
    mySequence.rampEdit.mMax.x = (float)mySequence.mFrameMax;

    ImGui::Begin("tanim_timeline");

    ImGui::PushItemWidth(130);
    ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
    ImGui::SameLine();
    ImGui::InputInt("Frame", &currentFrame);
    ImGui::SameLine();
    ImGui::InputInt("Frame Max", &mySequence.mFrameMax);
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
    ImGui::Text("mySequence.mFrameMin: %d", mySequence.mFrameMax);
    ImGui::Text("mySequence.myItems.size: %zu", mySequence.myItems.size());

    ImGui::End();

    ImGui::Begin("tanim_sequence_item");

    // add a UI to edit that particular item
    ImGui::Text("selected entry: %d", selectedEntry);
    if (selectedEntry != -1)
    {
        const TanimSequence::MySequenceItem& item = mySequence.myItems[selectedEntry];

        ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
        // switch (type) ...

        ImGui::Text("item.mType:       %d", item.mType);
        ImGui::Text("item.mExpanded:   %i", item.mExpanded);
        ImGui::Text("item.mFrameStart: %d", item.mFrameStart);
        ImGui::Text("item.mFrameEnd:   %d", item.mFrameEnd);
    }

    ImGui::End();

    ImGui::Begin("tanim_ramp_edit");

    ImGui::DragFloat2("mMin", &mySequence.rampEdit.mMin.x);
    ImGui::DragFloat2("mMax", &mySequence.rampEdit.mMax.x);

    ImGui::End();
}

}  // namespace tanim
