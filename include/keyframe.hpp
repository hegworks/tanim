#pragma once

#include "tanim/include/includes.hpp"

#include <string>
#include <vector>

#pragma once

namespace tanim
{

enum class TangentType : uint8_t
{
    SMOOTH,  // In/out tangents linked: same SmoothType, mirrored direction
    BROKEN,  // In/out tangents fully independent
};

struct Tangent
{
    enum class SmoothType : uint8_t
    {
        UNUSED,  // Currently in BROKEN mode
        AUTO,    // Clamped Catmull-Rom: auto-calculated, prevents overshoot
        FREE,    // User-adjustable direction, in/out remain mirrored
        FLAT,    // Zero slope (horizontal tangent)
    };

    enum class BrokenType : uint8_t
    {
        UNUSED,    // Currently in SMOOTH mode
        FREE,      // Fully user-controlled
        LINEAR,    // Points directly toward adjacent keyframe
        CONSTANT,  // Step function: holds value until next keyframe
    };

    // Tangent offset vector from keyframe position (like Bezier control point offset).
    // In-tangent points left (negative x), out-tangent points right (positive x).
    // This is the actual offset in curve space, used directly for Hermite evaluation (* 3).
    ImVec2 m_offset{1.0f, 0.0f};

    bool m_weighted{false};

    SmoothType m_smooth_type{SmoothType::AUTO};
    BrokenType m_broken_type{BrokenType::UNUSED};
};

struct Keyframe
{
    ImVec2 m_pos{0.0f, 0.0f};  // x = time/frame, y = value

    TangentType m_tangent_type{TangentType::SMOOTH};

    Tangent m_in;   // Incoming tangent. Not editable for first keyframe.
    Tangent m_out;  // Outgoing tangent. Not editable for last keyframe.

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
    std::vector<Keyframe> m_keyframes{Keyframe(0.0f, 0.0f), Keyframe(10.0f, 0.0f)};
    bool m_visibility{true};
    std::string m_name{"new_curve"};
};

}  // namespace tanim
