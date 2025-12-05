#pragma once
#include <cassert>
#include <cmath>

namespace tanim::helpers
{

inline int SecondsToFrame(float time, int samples) { return static_cast<int>(std::floorf(time * static_cast<float>(samples))); }

inline float SecondsToSampleTime(float time, int samples) { return time * static_cast<float>(samples); }

inline float FrameToSeconds(int frame, int samples)
{
    assert(samples > 0);
    return static_cast<float>(frame) * (1.0f / static_cast<float>(samples));
}

}  // namespace tanim::helpers
