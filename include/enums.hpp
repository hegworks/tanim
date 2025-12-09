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

}
