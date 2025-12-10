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
    static void SetTimelineData(TimelineData* timeline_data);
    static inline float m_snap_y_value = 0.1f;

    static void StartTimeline(TimelineData* timeline_data);
    static void UpdateTimeline(TimelineData* timeline_data, float delta_time);
    static void StopTimeline(TimelineData* timeline_data);

    static bool IsPlaying(const TimelineData* timeline_data);
    static void Play(TimelineData* timeline_data);
    static void Pause(TimelineData* timeline_data);
    static void Stop(TimelineData* timeline_data);

    static void EnterPlayMode() { m_is_engine_in_play_mode = true; }
    static void ExitPlayMode() { m_is_engine_in_play_mode = false; }

private:
    static inline Timeline m_timeline{};
    static inline bool m_is_engine_in_play_mode{};
    static inline bool m_preview{true};

    static void Sample(TimelineData* timeline_data);
};

}  // namespace tanim
