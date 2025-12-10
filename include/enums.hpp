#pragma once
#include <cstdint>

namespace tanim
{

enum class RepresentationMeta : uint8_t
{
    NONE = 0,
    VECTOR,
    COLOR,
    QUAT,
};

enum class PlaybackType : uint8_t
{
    HOLD = 0,
    RESET,
    LOOP,
};

}  // namespace tanim
