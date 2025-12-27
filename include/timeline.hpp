// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "user_override.hpp"
#include "tanim/include/timeline_data.hpp"
#include "tanim/include/sequence.hpp"
#include "tanim/include/timeliner.hpp"

#include <optional>
#include <vector>
#include <string>
#include <memory>

namespace tanim
{

class Timeline : public timeliner::TimelineInterface
{
public:
    //...................<<< Overrides >>>...................

    static int GetMinFrame(const TimelineData& /*data*/) { return 0; }

    static int GetMaxFrame(const TimelineData& data) { return data.m_max_frame; }

    static int GetSequenceCount(const TimelineData& data) { return static_cast<int>(data.m_sequences.size()); }

    static const char* GetSequenceLabel(const TimelineData& data, int seq_idx)
    {
        const std::string name = data.m_sequences.at(seq_idx).m_seq_id.m_entity_data.m_display +
                                 "::" + data.m_sequences.at(seq_idx).GetNameWithLessColumns();
        static char tmps[512];
        snprintf(tmps, 512, "%s", name.c_str());
        return tmps;
    }

    static void AddSequence(TimelineData& data) { data.m_sequences.emplace_back(); }

    static void DeleteSequence(TimelineData& data, int seq_idx) { data.m_sequences.erase(data.m_sequences.begin() + seq_idx); }

    static size_t GetCustomHeight(const TimelineData& data, int index)
    {
        return data.m_sequences.at(index).m_expanded ? 200 : 0;
    }

    static void DoubleClick(TimelineData& data, int seq_idx)
    {
        if (data.m_sequences.at(seq_idx).m_expanded)
        {
            data.m_sequences.at(seq_idx).m_expanded = false;
            return;
        }
        for (auto& item : data.m_sequences) item.m_expanded = false;
        data.m_sequences.at(seq_idx).m_expanded = !data.m_sequences.at(seq_idx).m_expanded;
    }

    static void CustomDraw(TimelineData& data,
                           int seq_idx,
                           ImDrawList* draw_list,
                           const ImRect& rc,
                           const ImRect& legend_rect,
                           const ImRect& clipping_rect,
                           const ImRect& legend_clipping_rect)
    {
        draw_list->PushClipRect(legend_clipping_rect.Min, legend_clipping_rect.Max, true);

        Sequence& seq = data.m_sequences.at(seq_idx);
        for (int curve_idx = 0; curve_idx < seq.GetCurveCount(); curve_idx++)
        {
            ImVec2 pta(legend_rect.Min.x + 30, legend_rect.Min.y + (float)curve_idx * 14.f);
            ImVec2 ptb(legend_rect.Max.x, legend_rect.Min.y + (float)(curve_idx + 1) * 14.f);
            draw_list->AddText(pta,
                               seq.GetCurveVisibility(curve_idx) ? 0xFFFFFFFF : 0x80FFFFFF,
                               seq.m_curves.at(curve_idx).m_name.c_str());
            if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
                seq.SetCurveVisibility(curve_idx, !seq.GetCurveVisibility(curve_idx));
        }
        draw_list->PopClipRect();

        ImGui::SetCursorScreenPos(rc.Min);
        const ImVec2 rcSize = ImVec2(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y);
        sequencer::Edit(data.m_sequences.at(seq_idx), rcSize, 137 + seq_idx, &clipping_rect);
    }

    static void CustomDrawCompact(TimelineData& data,
                                  int index,
                                  ImDrawList* draw_list,
                                  const ImRect& rc,
                                  const ImRect& clipping_rect)
    {
        draw_list->PushClipRect(clipping_rect.Min, clipping_rect.Max, true);
        Sequence& seq = data.m_sequences.at(index);
        for (int curve_index = 0; curve_index < seq.GetCurveCount(); curve_index++)
        {
            for (int point_idx = 0; point_idx < seq.GetCurvePointCount(curve_index); point_idx++)
            {
                float p = seq.m_curves.at(curve_index).m_points.at(point_idx).x;
                if (p < (float)data.m_first_frame || p > (float)seq.m_last_frame) continue;
                float r = (p - (float)data.m_min_frame) / float(data.m_max_frame - data.m_min_frame);
                float x = ImLerp(rc.Min.x, rc.Max.x, r);
                draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
            }
        }
        draw_list->PopClipRect();
    }

    static void BeginEdit(TimelineData& /*data*/, int /*seq_idx*/) { /*TODO(tanim)*/ }

    static void EndEdit(TimelineData& /*data*/) { /*TODO(tanim)*/ }

    static void Copy(TimelineData& /*data*/) { /*TODO(tanim)*/ }

    static void Paste(TimelineData& /*data*/) { /*TODO(tanim)*/ }

    static void EditFirstFrame(TimelineData& /*data*/, int /*new_start*/) { /*TODO(tanim)*/ }

    static void EditSequenceLastFrame(TimelineData& data, int seq_idx, int new_end)
    {
        data.m_sequences.at(seq_idx).EditLastFrame(new_end);
        RefreshTimelineLastFrame(data);
    }

    static void EditSequenceFirstFrame(TimelineData& data, int seq_idx, int new_end)
    {
        data.m_sequences.at(seq_idx).EditFirstFrame(new_end);
    }

    static void MoveSequence(TimelineData& data, int moving_entry, int diff)
    {
        data.m_sequences.at(moving_entry).MoveFrames(diff);
        RefreshTimelineLastFrame(data);
    }

    static void RefreshTimelineLastFrame(TimelineData& data)
    {
        int biggest_seq_last_frame = 0;
        for (const auto& seq : data.m_sequences)
        {
            if (seq.m_last_frame > biggest_seq_last_frame)
            {
                biggest_seq_last_frame = seq.m_last_frame;
            }
        }
        SetTimelineLastFrame(data, biggest_seq_last_frame);
    }

    static void SetTimelineLastFrame(TimelineData& data, int new_end) { data.m_last_frame = new_end; }

    static unsigned int GetColor(const TimelineData& /*data*/) { return 0xFFAA8080; }

    // TODO(tanim) replace this abomination with separate getters & setters
    // void MultiGet(int /*index*/, int** start, int** end, int* type, unsigned int* color) override
    //{
    //    if (color) *color = 0xFFAA8080;  // same color for everyone, return color based on type
    //    if (start) *start = nullptr;
    //     if (end) *end = &m_data->m_last_frame;
    //    if (type) *type = 0;
    //}

    // duplicate didn't make sense in my case, so I removed the functionality
    // void Duplicate(int /*index*/) override { /*m_sequence_datas.push_back(m_sequence_datas[index]);*/ }

    //...................<<< Helpers >>>...................

    //................<<< Helpers->Getters >>>...................

    static int GetTimelineFirstFrame(const TimelineData& [[maybe_unused]] data) { return 0; }

    static int GetTimelineLastFrame(const TimelineData& tdata) { return tdata.m_last_frame; }

    static int GetSequenceFirstFrame([[maybe_unused]] const TimelineData& data, [[maybe_unused]] int seq_idx)
    {
        return data.m_sequences.at(seq_idx).m_first_frame;
    }

    static int GetSequenceLastFrame(const TimelineData& data, int seq_idx) { return data.m_sequences.at(seq_idx).m_last_frame; }

    static int GetPlayImmediately(const TimelineData& tdata) { return tdata.m_play_immediately; }

    static int GetPlayerFrame(const TimelineData& tdata, const ComponentData& cdata)
    {
        return helpers::SecondsToFrame(cdata.m_player_time, tdata.m_player_samples);
    }

    static float GetPlayerRealTime(const ComponentData& cdata) { return cdata.m_player_time; }

    static float GetPlayerSampleTime(const TimelineData& tdata, const ComponentData& cdata)
    {
        return helpers::SecondsToSampleTime(cdata.m_player_time, tdata.m_player_samples);
    }

    static float GetLastFrameSampleTime(const TimelineData& data)
    {
        return helpers::FrameToSeconds(data.m_last_frame, data.m_player_samples);
    }

    static const std::string& GetName(const TimelineData& data) { return data.m_name; }

    static bool GetPlayerPlaying(const ComponentData& cdata) { return cdata.m_player_playing; }

    static Sequence& GetSequence(TimelineData& data, int seq_idx) { return data.m_sequences.at(seq_idx); }

    //................<<< Helpers->Setters >>>...................

    static void SetMaxFrame(TimelineData& data, int max_frame) { data.m_max_frame = max_frame; }

    static void SetDrawMaxX(TimelineData& data, int seq_idx, float max_x) { data.m_sequences.at(seq_idx).m_draw_max.x = max_x; }

    static void SetPlayerTimeFromFrame(const TimelineData& tdata, ComponentData& cdata, int frame_num)
    {
        cdata.m_player_time = helpers::FrameToSeconds(frame_num, tdata.m_player_samples);
    }

    static void SetPlayerTimeFromSeconds(ComponentData& cdata, float time) { cdata.m_player_time = time; }

    static void SetName(TimelineData& data, const std::string& name) { data.m_name = name; }

    //................<<< Helpers->Others >>>...................

    static std::optional<std::reference_wrapper<const Sequence>> FindSequenceWithFullName(const TimelineData& data,
                                                                                          const std::string& full_name)
    {
        for (const auto& seq : data.m_sequences)
        {
            if (full_name == seq.m_seq_id.FullName())
            {
                return std::ref(seq);
            }
        }
        return std::nullopt;
    }

    static bool HasSequenceWithFullName(const TimelineData& data, const std::string& full_name)
    {
        return FindSequenceWithFullName(data, full_name).has_value();
    }

    static void Play(ComponentData& cdata) { cdata.m_player_playing = true; }

    static void Pause(ComponentData& cdata) { cdata.m_player_playing = false; }

    static void Stop(ComponentData& cdata)
    {
        cdata.m_player_playing = false;
        ResetPlayerTime(cdata);
    }

    [[nodiscard]] static entt::entity FindEntity(const ComponentData& cdata, const Sequence& seq)
    {
        return GetNestedEntityOfUID(cdata.m_root_entity, seq.m_seq_id.m_entity_data.m_uid);
    }

    [[nodiscard]] static entt::entity FindEntity(const TimelineData& tdata, const ComponentData& cdata, int seq_idx)
    {
        return FindEntity(cdata, tdata.m_sequences.at(seq_idx));
    }

    [[nodiscard]] static entt::entity FindEntity(const ComponentData& cdata, const std::string& uid)
    {
        return GetNestedEntityOfUID(cdata.m_root_entity, uid);
    }

    static void ResetPlayerTime(ComponentData& cdata) { cdata.m_player_time = 0; }

    static void TickTime(const TimelineData& tdata, ComponentData& cdata, float dt)
    {
        cdata.m_player_time += dt;
        if (HasPassedLastFrame(tdata, cdata))
        {
            switch (tdata.m_playback_type)
            {
                case PlaybackType::HOLD:

                    SetPlayerTimeFromFrame(tdata, cdata, GetTimelineLastFrame(tdata));
                    Pause(cdata);
                    break;
                case PlaybackType::RESET:
                    Stop(cdata);
                    break;
                case PlaybackType::LOOP:
                    ResetPlayerTime(cdata);
                    break;
                default:
                    assert(0 && "unhandled PlaybackType");
            }
        }
    }

    static bool HasPassedLastFrame(const TimelineData& tdata, const ComponentData& cdata)
    {
        return GetPlayerFrame(tdata, cdata) > GetTimelineLastFrame(tdata);
    }

    static void EditSnapY(TimelineData& data, float value)
    {
        if (const auto seq = GetExpandedSequenceIdx(data))
        {
            data.m_sequences.at(seq.value()).EditSnapY(value);
        }
    }

    static std::optional<int> GetExpandedSequenceIdx(const TimelineData& data)
    {
        for (int i = 0; i < GetSequenceCount(data); ++i)
        {
            if (data.m_sequences.at(i).m_expanded)
            {
                return i;
            }
        }
        return std::nullopt;
    }

    static Sequence& AddSequenceStatic(TimelineData& data) { return data.m_sequences.emplace_back(); }
};

}  // namespace tanim
