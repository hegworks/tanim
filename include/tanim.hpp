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
    static void Update(float dt);
    static void SetTimelineData(TimelineData* timeline_data);
    static inline float m_snap_y_value = 0.1f;
    static void Sample(TimelineData* timeline_data);

private:
    static inline Timeline m_timeline{};

    static void Play();
    static void Pause();
};

}  // namespace tanim
