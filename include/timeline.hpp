// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/sequence.hpp"
#include "tanim/include/timeliner.hpp"

#include <vector>

namespace tanim
{

static const char* SequenceTypeNames[] = {"Position"};

struct Timeline : public timeliner::TimelineInterface
{
    struct SequenceData
    {
        int m_type{0};
        int m_first_frame{0};
        int m_last_frame{60};
        bool m_expanded{true};
    };

    int m_first_frame{0};
    int m_last_frame{500};

    std::vector<SequenceData> m_sequence_datas;

    // TODO(tanim) change to a vector of sequences
    Sequence m_sequence;

    int GetFirstFrame() const override { return 0; }

    int GetLastFrame() const override { return m_last_frame; }

    int GetSequenceCount() const override { return (int)m_sequence_datas.size(); }

    int GetSequenceTypeCount() const override { return sizeof(SequenceTypeNames) / sizeof(char*); }

    const char* GetSequenceTypeName(int typeIndex) const override { return SequenceTypeNames[typeIndex]; }

    const char* GetSequenceLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequenceTypeNames[m_sequence_datas[index].m_type]);
        return tmps;
    }

    // TODO(tanim) replace this abomination with separate getters & setters
    void MultiGet(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        SequenceData& item = m_sequence_datas[index];
        if (color) *color = 0xFFAA8080;  // same color for everyone, return color based on type
        if (start) *start = &item.m_first_frame;
        if (end) *end = &item.m_last_frame;
        if (type) *type = item.m_type;
    }

    void AddSequence(int type) override { m_sequence_datas.push_back({type}); }

    void DeleteSequence(int index) override { m_sequence_datas.erase(m_sequence_datas.begin() + index); }

    // TanimAddition
    // duplicate didn't make sense in my case, so I removed the functionality
    void Duplicate(int /*index*/) override { /*m_sequence_datas.push_back(m_sequence_datas[index]);*/ }

    size_t GetCustomHeight(int index) override { return m_sequence_datas[index].m_expanded ? 300 : 0; }

    void DoubleClick(int index) override
    {
        if (m_sequence_datas[index].m_expanded)
        {
            m_sequence_datas[index].m_expanded = false;
            return;
        }
        for (auto& item : m_sequence_datas) item.m_expanded = false;
        m_sequence_datas[index].m_expanded = !m_sequence_datas[index].m_expanded;
    }

    void CustomDraw(int index,
                    ImDrawList* draw_list,
                    const ImRect& rc,
                    const ImRect& legendRect,
                    const ImRect& clippingRect,
                    const ImRect& legendClippingRect) override
    {
        // TODO(tanim) hardcoded labels
        static const char* labels[] = {"X", "Y", "Z"};

        draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

        // TODO(tanim) hardcoded 3
        for (int i = 0; i < 3; i++)
        {
            ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
            ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
            draw_list->AddText(pta, m_sequence.GetCurveVisibility(i) ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
            if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
                m_sequence.SetCurveVisibility(i, !m_sequence.GetCurveVisibility(i));
        }
        draw_list->PopClipRect();

        ImGui::SetCursorScreenPos(rc.Min);
        const ImVec2 rcSize = ImVec2(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);
        sequencer::Edit(m_sequence, rcSize, 137 + index, &clippingRect);
    }

    void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override
    {
        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        for (int curve_index = 0; curve_index < m_sequence.CurveCount(); curve_index++)
        {
            for (int point_idx = 0; point_idx < m_sequence.GetCurvePointCount(curve_index); point_idx++)
            {
                float p = m_sequence.m_curves.at(curve_index).m_points.at(point_idx).x;
                if (p < m_sequence_datas[index].m_first_frame || p > m_sequence_datas[index].m_last_frame) continue;
                float r = (p - m_first_frame) / float(m_last_frame - m_first_frame);
                float x = ImLerp(rc.Min.x, rc.Max.x, r);
                draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
            }
        }
        draw_list->PopClipRect();
    }

    // TODO(tanim) once we have multple sequences, call the function on the expanded sequence
    void EditSnapY(float value) { m_sequence.EditSnapY(value); }

    void BeginEdit(int /* sequence_idx */) override { /*TODO(tanim)*/ }

    void EndEdit() override { /*TODO(tanim)*/ }

    void Copy() override { /*TODO(tanim)*/ }

    void Paste() override { /*TODO(tanim)*/ }

    void EditFirstFrame(int /* new_first_frame */) override { /*TODO(tanim)*/ }

    void EditLastFrame(int new_last_frame) override { m_sequence.TimelineLastFrameEdit(new_last_frame); }
};

}  // namespace tanim
