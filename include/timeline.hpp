// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/timeline_data.hpp"
#include "tanim/include/sequence.hpp"
#include "tanim/include/timeliner.hpp"

#include <optional>
#include <vector>
#include <string>
#include <memory>

namespace tanim
{

static const char* SequenceTypeNames[] = {"Position"};

struct Timeline : public timeliner::TimelineInterface
{
    TimelineData* m_data{nullptr};

    bool HasData() const { return m_data != nullptr; }

    int GetMinFrame() const override { return 0; }

    int GetMaxFrame() const override { return m_data->m_max_frame; }

    int GetFirstFrame() const { return 0; }

    int GetLastFrame() const { return m_data->m_last_frame; }

    void SetMaxFrame(int max_frame) { m_data->m_max_frame = max_frame; }

    const std::string& GetName() const { return m_data->m_name; }

    void SetName(const std::string& name) { m_data->m_name = name; }

    const std::string& GetName() { return m_data->m_name; }

    bool HasSequenceWithName(const std::string& seq_name) const
    {
        for (const auto& sequence : m_data->m_sequences)
        {
            if (seq_name == sequence.m_name)
            {
                return true;
            }
        }
        return false;
    }

    int GetSequenceCount() const override { return (int)m_data->m_sequences.size(); }

    void Play() { m_data->m_player_playing = true; }

    void Pause() { m_data->m_player_playing = false; }

    void Stop()
    {
        m_data->m_player_playing = false;
        m_data->SetPlayerTimeFromSeconds(0);
    }

    bool GetPlayerPlaying() const { return m_data->m_player_playing; }

    void TickTime(float dt) { m_data->m_player_time += dt; }

    bool IsPassedLastFrame() const { return m_data->m_player_time > m_data->LastFrameTime(); }

    // TODO(tanim) change after reflection
    int GetSequenceTypeCount() const override { return sizeof(SequenceTypeNames) / sizeof(char*); }

    const char* GetSequenceTypeName(int typeIndex) const override { return SequenceTypeNames[typeIndex]; }

    // TODO(tanim) change after reflection
    const char* GetSequenceLabel(int index) const override
    {
        static char tmps[512];
        snprintf(tmps, 512, "%s", m_data->m_sequences.at(index).GetNameWithLessColumns().c_str());
        return tmps;
    }

    // TODO(tanim) replace this abomination with separate getters & setters
    void MultiGet(int index, int** start, int** end, int* type, unsigned int* color) override
    {
        Sequence& item = m_data->m_sequences[index];
        if (color) *color = 0xFFAA8080;  // same color for everyone, return color based on type
        if (start) *start = &m_data->m_first_frame;
        if (end) *end = &m_data->m_last_frame;
        if (type) *type = item.m_type;
    }

    void AddSequence(int type) override
    {
        auto& seq = m_data->m_sequences.emplace_back(&m_data->m_last_frame);
        seq.m_type = type;
    }

    void DeleteSequence(int index) override { m_data->m_sequences.erase(m_data->m_sequences.begin() + index); }

    // TanimAddition
    // duplicate didn't make sense in my case, so I removed the functionality
    void Duplicate(int /*index*/) override { /*m_sequence_datas.push_back(m_sequence_datas[index]);*/ }

    size_t GetCustomHeight(int index) override { return m_data->m_sequences[index].m_expanded ? 200 : 0; }

    void DoubleClick(int index) override
    {
        if (m_data->m_sequences[index].m_expanded)
        {
            m_data->m_sequences[index].m_expanded = false;
            return;
        }
        for (auto& item : m_data->m_sequences) item.m_expanded = false;
        m_data->m_sequences[index].m_expanded = !m_data->m_sequences[index].m_expanded;
    }

    void CustomDraw(int seq_idx,
                    ImDrawList* draw_list,
                    const ImRect& rc,
                    const ImRect& legendRect,
                    const ImRect& clippingRect,
                    const ImRect& legendClippingRect) override
    {
        draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

        Sequence& seq = m_data->m_sequences.at(seq_idx);
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
        sequencer::Edit(m_data->m_sequences.at(seq_idx), rcSize, 137 + seq_idx, &clippingRect);
    }

    void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override
    {
        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        Sequence& seq = m_data->m_sequences.at(index);
        for (int curve_index = 0; curve_index < seq.GetCurveCount(); curve_index++)
        {
            for (int point_idx = 0; point_idx < seq.GetCurvePointCount(curve_index); point_idx++)
            {
                float p = seq.m_curves.at(curve_index).m_points.at(point_idx).x;
                if (p < (float)m_data->m_first_frame || p > (float)m_data->m_last_frame) continue;
                float r = (p - (float)m_data->m_min_frame) / float(m_data->m_max_frame - m_data->m_min_frame);
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
            m_data->m_sequences.at(seq.value()).EditSnapY(value);
        }
    }

    std::optional<int> GetExpandedSequenceIdx() const
    {
        for (int i = 0; i < (int)m_data->m_sequences.size(); ++i)
        {
            if (m_data->m_sequences.at(i).m_expanded)
            {
                return i;
            }
        }
        return std::nullopt;
    }

    Sequence& GetSequence(int sequence_idx) { return m_data->m_sequences.at(sequence_idx); }

    void BeginEdit(int /* sequence_idx */) override { /*TODO(tanim)*/ }

    void EndEdit() override { /*TODO(tanim)*/ }

    void Copy() override { /*TODO(tanim)*/ }

    void Paste() override { /*TODO(tanim)*/ }

    void EditFirstFrame(int /* new_first_frame */) override { /*TODO(tanim)*/ }

    // TODO(tanim) call the function on the expanded sequence
    void EditLastFrame(int new_last_frame) override
    {
        m_data->m_last_frame = new_last_frame;
        for (int i = 0; i < (int)m_data->m_sequences.size(); ++i)
        {
            m_data->m_sequences.at(i).EditTimelineLastFrame();
        }
    }
};

}  // namespace tanim
