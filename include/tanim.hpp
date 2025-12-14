#pragma once
#include "tanim/include/timeline.hpp"

namespace tanim
{

class Tanim
{
public:
    Tanim() = default;

    static void Init();
    static void Draw();
    static void UpdateEditor(float dt);
    static void OpenForEditing(TimelineData& timeline_data);
    static inline float m_snap_y_value = 0.1f;

    static void StartTimeline(TimelineData& timeline_data);
    static void UpdateTimeline(TimelineData& timeline_data, float delta_time);
    static void StopTimeline(TimelineData& timeline_data);

    static bool IsPlaying(const TimelineData& timeline_data);
    static void Play(TimelineData& timeline_data);
    static void Pause(TimelineData& timeline_data);
    static void Stop(TimelineData& timeline_data);

    [[nodiscard]] static std::string Serialize(TimelineData& data);
    static void Deserialize(TimelineData& data, const std::string& serialized_string);

    static void EnterPlayMode() { m_is_engine_in_play_mode = true; }
    static void ExitPlayMode() { m_is_engine_in_play_mode = false; }

private:
    static inline TimelineData* m_editing_timeline_data{};
    static inline bool m_is_engine_in_play_mode{};
    static inline bool m_preview{true};

    static void Sample(TimelineData& timeline_data);
};

}  // namespace tanim
