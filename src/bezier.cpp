#include "tanim/include/bezier.hpp"
#include "tanim/include/helpers.hpp"

#include <algorithm>
#include <cmath>

namespace tanim
{

// === Vector Utilities ===

float Vec2Length(const ImVec2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

// === Cubic Bezier Curve Evaluation ===

ImVec2 CubicBezier(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
{
    // P(t) = (1-t)^3*P0 + 3(1-t)^2*tP1 + 3(1-t)t^2*P2 + t^3*P3
    const float u = 1.0f - t;
    const float u2 = u * u;
    const float u3 = u2 * u;
    const float t2 = t * t;
    const float t3 = t2 * t;

    return {(u3 * p0.x) + (3.0f * u2 * t * p1.x) + (3.0f * u * t2 * p2.x) + t3 * p3.x,
            (u3 * p0.y) + (3.0f * u2 * t * p1.y) + (3.0f * u * t2 * p2.y) + t3 * p3.y};
}

// === Curve Sampling ===

int FindSegmentIndex(const Curve& curve, float time)
{
    const auto& keyframes = curve.m_keyframes;
    int count = (int)keyframes.size();

    if (count == 0) return -1;
    if (time < keyframes.at(0).Time()) return -1;
    if (count == 1) return 0;

    for (int i = 0; i < count - 1; i++)
    {
        if (time >= keyframes.at(i).Time() && time <= keyframes.at(i + 1).Time())
        {
            return i;
        }
    }

    return count - 2;  // Last segment
}

float FindTForX(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float target_x)
{
    // Initial guess using linear interpolation
    float t = (target_x - p0.x) / (p3.x - p0.x);
    t = std::clamp(t, 0.0f, 1.0f);

    // Newton-Raphson iterations
    for (int i = 0; i < 8; i++)
    {
        ImVec2 pos = CubicBezier(p0, p1, p2, p3, t);
        float error = pos.x - target_x;

        if (std::abs(error) < 1e-6f) break;

        // Derivative of Bezier X with respect to t
        float u = 1.0f - t;
        float dx_dt = 3.0f * u * u * (p1.x - p0.x) + 6.0f * u * t * (p2.x - p1.x) + 3.0f * t * t * (p3.x - p2.x);

        if (std::abs(dx_dt) < 1e-6f) break;

        t -= error / dx_dt;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    return t;
}

float SampleCurveValue(const Curve& curve, float time)
{
    const auto& keyframes = curve.m_keyframes;
    const int count = static_cast<int>(keyframes.size());

    if (count == 0) return 0.0f;
    if (count == 1) return keyframes.at(0).Value();

    // Before first keyframe
    if (time <= keyframes.at(0).Time())
    {
        return keyframes.at(0).Value();
    }

    // After last keyframe
    if (time >= keyframes.at(count - 1).Time())
    {
        return keyframes.at(count - 1).Value();
    }

    // Find segment
    const int seg = FindSegmentIndex(curve, time);
    if (seg < 0) return keyframes.at(0).Value();

    const Keyframe& k0 = keyframes.at(seg);
    const Keyframe& k1 = keyframes.at(seg + 1);

    // Check for CONSTANT out-handle (step function)
    if (k0.m_handle_type == HandleType::BROKEN && k0.m_out.m_broken_type == Handle::BrokenType::CONSTANT)
    {
        return k0.Value();
    }

    // Bezier control points
    const ImVec2 p0 = k0.m_pos;
    const ImVec2 p1 = k0.m_pos + k0.m_out.m_offset;
    const ImVec2 p2 = k1.m_pos + k1.m_in.m_offset;
    const ImVec2 p3 = k1.m_pos;

    const float t = FindTForX(p0, p1, p2, p3, time);

    return CubicBezier(p0, p1, p2, p3, t).y;
}

ImVec2 SampleCurveForDrawing(const Curve& curve, float t_param, const ImVec2& min, const ImVec2& max)
{
    const auto& keyframes = curve.m_keyframes;
    const int count = static_cast<int>(keyframes.size());

    auto normalize_point = [&](const ImVec2& p) -> ImVec2
    { return ImVec2((p.x - min.x) / (max.x - min.x + 1.0f), (p.y - min.y) / (max.y - min.y)); };

    if (count == 0) return ImVec2(0.0f, 0.0f);
    if (count == 1) return normalize_point(keyframes.at(0).m_pos);

    t_param = std::clamp(t_param, 0.0f, 1.0f);

    // Map t to segment
    float segment_float = t_param * (count - 1);
    int seg = static_cast<int>(segment_float);
    if (seg >= count - 1)
    {
        seg = count - 2;
        segment_float = static_cast<float>(count - 1);
    }

    const float local_t = segment_float - static_cast<float>(seg);

    const Keyframe& k0 = keyframes.at(seg);
    const Keyframe& k1 = keyframes.at(seg + 1);

    // Check for CONSTANT
    if (k0.m_handle_type == HandleType::BROKEN && k0.m_out.m_broken_type == Handle::BrokenType::CONSTANT)
    {
        const float step_x = k0.m_pos.x + local_t * (k1.m_pos.x - k0.m_pos.x);
        return normalize_point(ImVec2(step_x, k0.Value()));
    }

    // Bezier control points
    const ImVec2 p0 = k0.m_pos;
    const ImVec2 p1 = k0.m_pos + k0.m_out.m_offset;
    const ImVec2 p2 = k1.m_pos + k1.m_in.m_offset;
    const ImVec2 p3 = k1.m_pos;

    const ImVec2 result = CubicBezier(p0, p1, p2, p3, local_t);

    return normalize_point(result);
}

// === Handle Calculation Helpers ===

float CalculateAutoHandleSlope(const Keyframe* prev, const Keyframe& current, const Keyframe* next)
{
    // Clamped Catmull-Rom handle calculation

    if (!prev && !next)
    {
        return 0.0f;  // Flat
    }

    if (!prev)
    {
        // First keyframe: handle points toward next
        float dx = next->m_pos.x - current.m_pos.x;
        float dy = next->m_pos.y - current.m_pos.y;
        if (std::abs(dx) < 1e-6f) return 0.0f;
        return dy / dx;
    }

    if (!next)
    {
        // Last keyframe: handle points from previous
        float dx = current.m_pos.x - prev->m_pos.x;
        float dy = current.m_pos.y - prev->m_pos.y;
        if (std::abs(dx) < 1e-6f) return 0.0f;
        return dy / dx;
    }

    // Interior keyframe: Catmull-Rom with clamping

    // Check for local extremum (clamping)
    bool is_local_max = (current.m_pos.y >= prev->m_pos.y && current.m_pos.y >= next->m_pos.y);
    bool is_local_min = (current.m_pos.y <= prev->m_pos.y && current.m_pos.y <= next->m_pos.y);

    if (is_local_max || is_local_min)
    {
        return 0.0f;  // Flat handle to prevent overshoot
    }

    // Standard Catmull-Rom: handle parallel to line between neighbors
    float time_span = next->m_pos.x - prev->m_pos.x;
    if (time_span < 1e-6f) return 0.0f;

    float dy = next->m_pos.y - prev->m_pos.y;
    float slope = dy / time_span;

    // Clamp slope to prevent overshoot
    float slope_to_prev = (current.m_pos.y - prev->m_pos.y) / (current.m_pos.x - prev->m_pos.x);
    float slope_to_next = (next->m_pos.y - current.m_pos.y) / (next->m_pos.x - current.m_pos.x);

    // If slopes have different signs, we're at an inflection point
    if (slope_to_prev * slope_to_next < 0)
    {
        slope = 0.0f;
    }
    else
    {
        // Clamp to the smaller of the two slopes (prevents overshoot)
        float min_slope = std::min(std::abs(slope_to_prev), std::abs(slope_to_next));
        if (std::abs(slope) > min_slope)
        {
            slope = (slope > 0) ? min_slope : -min_slope;
        }
    }

    return slope;
}

float CalculateLinearHandleSlope(const Keyframe& current, const Keyframe& adjacent)
{
    float dx = adjacent.m_pos.x - current.m_pos.x;
    float dy = adjacent.m_pos.y - current.m_pos.y;
    if (std::abs(dx) < 1e-6f) return 0.0f;
    return dy / dx;
}

// === Handle Resolution ===

void ResolveKeyframeHandles(Keyframe& keyframe, const Keyframe* prev_key, const Keyframe* next_key)
{
    // Resolve in-handle
    if (prev_key)
    {
        Handle& in_tan = keyframe.m_in;
        float segment_duration = keyframe.Time() - prev_key->Time();
        float default_x = -segment_duration / 3.0f;  // Points left

        if (keyframe.m_handle_type == HandleType::SMOOTH)
        {
            switch (in_tan.m_smooth_type)
            {
                case Handle::SmoothType::AUTO:
                {
                    float slope = CalculateAutoHandleSlope(prev_key, keyframe, next_key);
                    in_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Handle::SmoothType::FLAT:
                    in_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Handle::SmoothType::FREE:
                    if (!in_tan.m_weighted)
                    {
                        // Preserve direction, reset length
                        float len = Vec2Length(in_tan.m_offset);
                        if (len > 1e-6f)
                        {
                            float scale = std::abs(default_x) / std::abs(in_tan.m_offset.x + 1e-6f);
                            in_tan.m_offset.x = default_x;
                            in_tan.m_offset.y *= scale;
                        }
                        else
                        {
                            in_tan.m_offset = ImVec2(default_x, 0.0f);
                        }
                    }
                    break;
                case Handle::SmoothType::UNUSED:
                    break;
            }
        }
        else  // BROKEN
        {
            switch (in_tan.m_broken_type)
            {
                case Handle::BrokenType::LINEAR:
                {
                    float slope = CalculateLinearHandleSlope(keyframe, *prev_key);
                    in_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Handle::BrokenType::CONSTANT:
                    in_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Handle::BrokenType::FREE:
                    if (!in_tan.m_weighted)
                    {
                        float len = Vec2Length(in_tan.m_offset);
                        if (len > 1e-6f)
                        {
                            float scale = std::abs(default_x) / std::abs(in_tan.m_offset.x + 1e-6f);
                            in_tan.m_offset.x = default_x;
                            in_tan.m_offset.y *= scale;
                        }
                        else
                        {
                            in_tan.m_offset = ImVec2(default_x, 0.0f);
                        }
                    }
                    break;
                case Handle::BrokenType::UNUSED:
                    break;
            }
        }
    }

    // Resolve out-handle
    if (next_key)
    {
        Handle& out_tan = keyframe.m_out;
        float segment_duration = next_key->Time() - keyframe.Time();
        float default_x = segment_duration / 3.0f;  // Points right

        if (keyframe.m_handle_type == HandleType::SMOOTH)
        {
            switch (out_tan.m_smooth_type)
            {
                case Handle::SmoothType::AUTO:
                {
                    float slope = CalculateAutoHandleSlope(prev_key, keyframe, next_key);
                    out_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Handle::SmoothType::FLAT:
                    out_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Handle::SmoothType::FREE:
                    if (!out_tan.m_weighted)
                    {
                        float len = Vec2Length(out_tan.m_offset);
                        if (len > 1e-6f)
                        {
                            float scale = default_x / std::abs(out_tan.m_offset.x + 1e-6f);
                            out_tan.m_offset.x = default_x;
                            out_tan.m_offset.y *= scale;
                        }
                        else
                        {
                            out_tan.m_offset = ImVec2(default_x, 0.0f);
                        }
                    }
                    break;
                case Handle::SmoothType::UNUSED:
                    break;
            }
        }
        else  // BROKEN
        {
            switch (out_tan.m_broken_type)
            {
                case Handle::BrokenType::LINEAR:
                {
                    float slope = CalculateLinearHandleSlope(keyframe, *next_key);
                    out_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Handle::BrokenType::CONSTANT:
                    out_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Handle::BrokenType::FREE:
                    if (!out_tan.m_weighted)
                    {
                        float len = Vec2Length(out_tan.m_offset);
                        if (len > 1e-6f)
                        {
                            float scale = default_x / std::abs(out_tan.m_offset.x + 1e-6f);
                            out_tan.m_offset.x = default_x;
                            out_tan.m_offset.y *= scale;
                        }
                        else
                        {
                            out_tan.m_offset = ImVec2(default_x, 0.0f);
                        }
                    }
                    break;
                case Handle::BrokenType::UNUSED:
                    break;
            }
        }
    }
}

void ResolveCurveHandles(Curve& curve)
{
    int count = (int)curve.m_keyframes.size();

    for (int i = 0; i < count; i++)
    {
        const Keyframe* prev = (i > 0) ? &curve.m_keyframes.at(i - 1) : nullptr;
        const Keyframe* next = (i < count - 1) ? &curve.m_keyframes.at(i + 1) : nullptr;

        ResolveKeyframeHandles(curve.m_keyframes.at(i), prev, next);
    }
}

// === Handle Constraint Helpers ===

void MirrorHandlesDir(Keyframe& keyframe, bool from_out_to_in)
{
    if (from_out_to_in)
    {
        // Mirror out-handle to in-handle (negate both components)
        keyframe.m_in.m_offset = ImVec2(-keyframe.m_out.m_offset.x, -keyframe.m_out.m_offset.y);
    }
    else
    {
        // Mirror in-handle to out-handle (negate both components)
        keyframe.m_out.m_offset = ImVec2(-keyframe.m_in.m_offset.x, -keyframe.m_in.m_offset.y);
    }
}

void ValidateHandleDir(Handle& handle, bool is_in_handle)
{
    if (is_in_handle)
    {
        // In-handle must point left (negative x)
        if (handle.m_offset.x > 0)
        {
            handle.m_offset.x = -handle.m_offset.x;
        }
    }
    else
    {
        // Out-handle must point right (positive x)
        if (handle.m_offset.x < 0)
        {
            handle.m_offset.x = -handle.m_offset.x;
        }
    }
}

}  // namespace tanim
