// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/sequence.hpp"
#include "tanim/include/timeliner.hpp"

#include <optional>
#include <vector>
#include <string>

namespace tanim
{

static const char* SequenceTypeNames[] = {"Position"};

struct Timeline : public timeliner::TimelineInterface
{
    int m_first_frame{0};
    int m_last_frame{10};
    int m_min_frame{0};
    int m_max_frame{500};
    std::string m_name{"New Timeline"};
    std::vector<Sequence> m_sequences{};

    int GetMinFrame() const override { return 0; }

    int GetMaxFrame() const override { return m_max_frame; }

    int GetSequenceCount() const override { return (int)m_sequences.size(); }

    // TODO(tanim) change after reflection
    int GetSequenceTypeCount() const override { return sizeof(SequenceTypeNames) / sizeof(char*); }

    const char* GetSequenceTypeName(int typeIndex) const override { return SequenceTypeNames[typeIndex]; }

    // TODO(tanim) change after reflection
    const char* GetSequenceLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequenceTypeNames[m_sequences.at(index).m_type]);
        return tmps;
    }

    // TODO(tanim) replace this abomination with separate getters & setters
    void MultiGet(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        Sequence& item = m_sequences[index];
        if (color) *color = 0xFFAA8080;  // same color for everyone, return color based on type
        if (start) *start = &m_first_frame;
        if (end) *end = &m_last_frame;
        if (type) *type = item.m_type;
    }

    void AddSequence(int type) override
    {
        auto& seq = m_sequences.emplace_back(&m_last_frame);
        seq.m_type = type;
        seq.m_expanded = true;
    }

    void DeleteSequence(int index) override { m_sequences.erase(m_sequences.begin() + index); }

    // TanimAddition
    // duplicate didn't make sense in my case, so I removed the functionality
    void Duplicate(int /*index*/) override { /*m_sequence_datas.push_back(m_sequence_datas[index]);*/ }

    size_t GetCustomHeight(int index) override { return m_sequences[index].m_expanded ? 300 : 0; }

    void DoubleClick(int index) override
    {
        if (m_sequences[index].m_expanded)
        {
            m_sequences[index].m_expanded = false;
            return;
        }
        for (auto& item : m_sequences) item.m_expanded = false;
        m_sequences[index].m_expanded = !m_sequences[index].m_expanded;
    }

    void CustomDraw(int seq_idx,
                    ImDrawList* draw_list,
                    const ImRect& rc,
                    const ImRect& legendRect,
                    const ImRect& clippingRect,
                    const ImRect& legendClippingRect) override
    {
        draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

        Sequence& seq = m_sequences.at(seq_idx);
        for (int curve_idx = 0; curve_idx < seq.GetCurveCount(); curve_idx++)
        {
            ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + (float)curve_idx * 14.f);
            ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (float)(curve_idx + 1) * 14.f);
            draw_list->AddText(pta,
                               seq.GetCurveVisibility(curve_idx) ? 0xFFFFFFFF : 0x80FFFFFF,
                               seq.m_curves.at(curve_idx).m_name.c_str());
            if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
                seq.SetCurveVisibility(curve_idx, !seq.GetCurveVisibility(curve_idx));
        }
        draw_list->PopClipRect();

        ImGui::SetCursorScreenPos(rc.Min);
        const ImVec2 rcSize = ImVec2(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);
        sequencer::Edit(m_sequences.at(0), rcSize, 137 + seq_idx, &clippingRect);
    }

    void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override
    {
        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        Sequence& seq = m_sequences.at(index);
        for (int curve_index = 0; curve_index < seq.GetCurveCount(); curve_index++)
        {
            for (int point_idx = 0; point_idx < seq.GetCurvePointCount(curve_index); point_idx++)
            {
                float p = seq.m_curves.at(curve_index).m_points.at(point_idx).x;
                if (p < (float)m_first_frame || p > (float)m_last_frame) continue;
                float r = (p - (float)m_min_frame) / float(m_max_frame - m_min_frame);
                float x = ImLerp(rc.Min.x, rc.Max.x, r);
                draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
            }
        }
        draw_list->PopClipRect();
    }

    void EditSnapY(float value)
    {
        if (const auto seq = GetExpandedSequenceIdx())
        {
            m_sequences.at(seq.value()).EditSnapY(value);
        }
    }

    std::optional<int> GetExpandedSequenceIdx() const
    {
        for (int i = 0; i < (int)m_sequences.size(); ++i)
        {
            if (m_sequences.at(i).m_expanded)
            {
                return i;
            }
        }
        return std::nullopt;
    }

    Sequence& GetSequence(int sequence_idx) { return m_sequences.at(sequence_idx); }

    void BeginEdit(int /* sequence_idx */) override { /*TODO(tanim)*/ }

    void EndEdit() override { /*TODO(tanim)*/ }

    void Copy() override { /*TODO(tanim)*/ }

    void Paste() override { /*TODO(tanim)*/ }

    void EditFirstFrame(int /* new_first_frame */) override { /*TODO(tanim)*/ }

    // TODO(tanim) call the function on the expanded sequence
    void EditLastFrame(int new_last_frame) override
    {
        m_last_frame = new_last_frame;
        for (int i = 0; i < (int)m_sequences.size(); ++i)
        {
            m_sequences.at(i).EditTimelineLastFrame();
        }
    }
};

}  // namespace tanim
