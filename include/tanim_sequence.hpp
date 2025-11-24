#pragma once
#include "tanim_ramp_edit.hpp"
#include "tanimguizmo/tanimguizmo_sequencer.h"

#include <vector>

namespace tanim
{

static const char* SequencerItemTypeNames[] = {"Camera", "Music", "ScreenEffect", "FadeIn", "Animation"};

struct TanimSequence : public tanimguizmo_sequencer::SequenceInterface
{
    // interface with sequencer

    // my datas
    TanimSequence() : mFrameMin(0), mFrameMax(0) {}
    int mFrameMin, mFrameMax;

    struct MySequenceItem
    {
        int mType;
        int mFrameStart, mFrameEnd;
        bool mExpanded;
    };

    std::vector<MySequenceItem> myItems;

    TanimRampEdit rampEdit;

    int GetFrameMin() const override { return mFrameMin; }

    int GetFrameMax() const override { return mFrameMax; }

    int GetItemCount() const override { return (int)myItems.size(); }

    int GetItemTypeCount() const override { return sizeof(SequencerItemTypeNames) / sizeof(char*); }

    const char* GetItemTypeName(int typeIndex) const override { return SequencerItemTypeNames[typeIndex]; }

    const char* GetItemLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
        return tmps;
    }

    void Get(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        MySequenceItem& item = myItems[index];
        if (color) *color = 0xFFAA8080;  // same color for everyone, return color based on type
        if (start) *start = &item.mFrameStart;
        if (end) *end = &item.mFrameEnd;
        if (type) *type = item.mType;
    }

    void Add(int type) override { myItems.push_back(MySequenceItem{type, 0, 10, false}); }

    void Del(int index) override { myItems.erase(myItems.begin() + index); }

    void Duplicate(int index) override { myItems.push_back(myItems[index]); }

    size_t GetCustomHeight(int index) override { return myItems[index].mExpanded ? 300 : 0; }

    void DoubleClick(int index) override
    {
        if (myItems[index].mExpanded)
        {
            myItems[index].mExpanded = false;
            return;
        }
        for (auto& item : myItems) item.mExpanded = false;
        myItems[index].mExpanded = !myItems[index].mExpanded;
    }

    void CustomDraw(int index,
                    ImDrawList* draw_list,
                    const ImRect& rc,
                    const ImRect& legendRect,
                    const ImRect& clippingRect,
                    const ImRect& legendClippingRect) override
    {
        static const char* labels[] = {"Translation", "Rotation", "Scale"};

        rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
        for (int i = 0; i < 3; i++)
        {
            ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
            ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
            draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
            if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
                rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
        }
        draw_list->PopClipRect();

        ImGui::SetCursorScreenPos(rc.Min);
        const ImVec2 rcSize = ImVec2(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);
        tanimguizmo_curve_edit::Edit(rampEdit, rcSize, 137 + index, &clippingRect);
    }

    void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override
    {
        rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        for (int i = 0; i < 3; i++)
        {
            for (unsigned int j = 0; j < rampEdit.mPointCount[i]; j++)
            {
                float p = rampEdit.mPts[i][j].x;
                if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd) continue;
                float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
                float x = ImLerp(rc.Min.x, rc.Max.x, r);
                draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
            }
        }
        draw_list->PopClipRect();
    }
};

}  // namespace tanim
