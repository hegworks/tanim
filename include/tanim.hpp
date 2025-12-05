#pragma once
#include "tanim/include/timeline.hpp"

namespace tanim
{

enum class EngineState
{
    EDITOR,
    PLAY,
    PAUSE,
};

class Tanim
{
public:
    Tanim() = default;

    static void Init();
    static void Draw();
    static void Update(float dt);
    static void SetTimelineData(TimelineData* timeline_data);
    static inline float m_snap_y_value = 0.1f;
    static void Sample(TimelineData* timeline_data);
    static void UpdateTimeline(TimelineData* timeline_data, float delta_time);
    static void EnterPlayMode() { m_engine_state = EngineState::PLAY; }
    static void EnterPauseMode() { m_engine_state = EngineState::PAUSE; }
    static void ExitPlayMode() { m_engine_state = EngineState::EDITOR; }

private:
    static inline Timeline m_timeline{};
    static inline EngineState m_engine_state{};

    static void Play();
    static void Pause();
};

}  // namespace tanim
