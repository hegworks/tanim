#pragma once

#include "tanim/include/includes.hpp"

#include <string>
#include <vector>

#pragma once

namespace tanim
{

enum class HandleType : uint8_t
{
    SMOOTH,  // In/out handles linked: same SmoothType, mirrored direction
    BROKEN,  // In/out handles fully independent
};

enum class EnforcedType : uint8_t
{
    UNENFORCED,
    AUTO,      // Smooth
    FLAT,      // Smooth
    LINEAR,    // Broken
    CONSTANT,  // Broken
};

struct Handle
{
    enum class SmoothType : uint8_t
    {
        UNUSED,  // Currently in BROKEN mode
        AUTO,    // Clamped Catmull-Rom: auto-calculated, prevents overshoot
        FREE,    // User-adjustable direction, in/out remain mirrored
        FLAT,    // Zero slope (horizontal handle)
    };

    enum class BrokenType : uint8_t
    {
        UNUSED,    // Currently in SMOOTH mode
        FREE,      // Fully user-controlled
        LINEAR,    // Points directly toward adjacent keyframe
        CONSTANT,  // Step function: holds value until next keyframe
    };

    ImVec2 m_offset{1.0f, 0.0f};

    bool m_weighted{false};

    SmoothType m_smooth_type{SmoothType::AUTO};
    BrokenType m_broken_type{BrokenType::UNUSED};
};

struct Keyframe
{
    ImVec2 m_pos{};  // x = time/frame, y = value

    HandleType m_handle_type{HandleType::SMOOTH};

    Handle m_in;   // Incoming handle. Not editable for first keyframe.
    Handle m_out;  // Outgoing handle. Not editable for last keyframe.

    Keyframe()
    {
        m_in.m_offset = ImVec2{-1.0f, 0.0f};  // Points left (flat)
        m_out.m_offset = ImVec2{1.0f, 0.0f};  // Points right (flat)
    }

    explicit Keyframe(ImVec2 pos) : m_pos(pos)
    {
        m_in.m_offset = ImVec2{-1.0f, 0.0f};
        m_out.m_offset = ImVec2{1.0f, 0.0f};
    }

    Keyframe(float time, float value) : m_pos{time, value}
    {
        m_in.m_offset = ImVec2{-1.0f, 0.0f};
        m_out.m_offset = ImVec2{1.0f, 0.0f};
    }

    float Time() const { return m_pos.x; }
    float Value() const { return m_pos.y; }
    int Frame() const { return static_cast<int>(m_pos.x); }
};

struct Curve
{
    std::vector<Keyframe> m_keyframes{};
    EnforcedType m_enforced_type{EnforcedType::UNENFORCED};
    bool m_visibility{true};
    std::string m_name{"new_curve"};
};

}  // namespace tanim
