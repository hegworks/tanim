#include "tanim/include/hermite.hpp"

#include <algorithm>
#include <cmath>

namespace tanim
{

// === Vector Utilities ===

float Vec2Length(const ImVec2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

// === Hermite Spline Evaluation ===

float EvaluateHermite(float p0, float m0, float p1, float m1, float t)
{
    // Cubic Hermite basis functions:
    // H00(t) = 2t³ - 3t² + 1
    // H10(t) = t³ - 2t² + t
    // H01(t) = -2t³ + 3t²
    // H11(t) = t³ - t²
    // P(t) = H00*p0 + H10*m0 + H01*p1 + H11*m1

    float t2 = t * t;
    float t3 = t2 * t;

    float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
    float h10 = t3 - 2.0f * t2 + t;
    float h01 = -2.0f * t3 + 3.0f * t2;
    float h11 = t3 - t2;

    return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
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

float FindTForX(float p0_x, float m0_x, float p1_x, float m1_x, float target_x)
{
    // Initial guess using linear interpolation
    float t = (target_x - p0_x) / (p1_x - p0_x);
    t = std::clamp(t, 0.0f, 1.0f);

    // Newton-Raphson iterations
    for (int i = 0; i < 8; i++)
    {
        float current_x = EvaluateHermite(p0_x, m0_x, p1_x, m1_x, t);
        float error = current_x - target_x;

        if (std::abs(error) < 1e-6f) break;

        // Derivative of Hermite X with respect to t
        float t2 = t * t;
        float dx_dt =
            m0_x * (3.0f * t2 - 4.0f * t + 1.0f) + m1_x * (3.0f * t2 - 2.0f * t) + (p1_x - p0_x) * (6.0f * t - 6.0f * t2);

        if (std::abs(dx_dt) < 1e-6f) break;

        t -= error / dx_dt;
        t = std::clamp(t, 0.0f, 1.0f);
    }

    return t;
}

float SampleCurveValue(const Curve& curve, float time)
{
    const auto& keyframes = curve.m_keyframes;
    int count = static_cast<int>(keyframes.size());

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
    int seg = FindSegmentIndex(curve, time);
    if (seg < 0) return keyframes.at(0).Value();

    const Keyframe& k0 = keyframes.at(seg);
    const Keyframe& k1 = keyframes.at(seg + 1);

    // Check for CONSTANT out-tangent (step function)
    if (k0.m_tangent_type == TangentType::BROKEN && k0.m_out.m_broken_type == Tangent::BrokenType::CONSTANT)
    {
        return k0.Value();
    }

    // 2D Hermite with proper tangent vectors
    // Tangent vectors in curve space (multiply by 3 for Bezier-to-Hermite)
    ImVec2 m0 = ImVec2(k0.m_out.m_offset.x * 3.0f, k0.m_out.m_offset.y * 3.0f);
    ImVec2 m1 = ImVec2(-k1.m_in.m_offset.x * 3.0f, -k1.m_in.m_offset.y * 3.0f);

    float t = FindTForX(k0.Time(), m0.x, k1.Time(), m1.x, time);

    return EvaluateHermite(k0.Value(), m0.y, k1.Value(), m1.y, t);
}

ImVec2 SampleCurveForDrawing(const Curve& curve, float t_param, const ImVec2& min, const ImVec2& max)
{
    const auto& keyframes = curve.m_keyframes;
    int count = static_cast<int>(keyframes.size());

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

    float local_t = segment_float - static_cast<float>(seg);

    const Keyframe& k0 = keyframes.at(seg);
    const Keyframe& k1 = keyframes.at(seg + 1);

    // Check for CONSTANT
    if (k0.m_tangent_type == TangentType::BROKEN && k0.m_out.m_broken_type == Tangent::BrokenType::CONSTANT)
    {
        float step_x = k0.m_pos.x + local_t * (k1.m_pos.x - k0.m_pos.x);
        return normalize_point(ImVec2(step_x, k0.Value()));
    }

    // 2D Hermite evaluation
    ImVec2 m0 = ImVec2(k0.m_out.m_offset.x * 3.0f, k0.m_out.m_offset.y * 3.0f);
    ImVec2 m1 = ImVec2(-k1.m_in.m_offset.x * 3.0f, -k1.m_in.m_offset.y * 3.0f);

    float x = EvaluateHermite(k0.Time(), m0.x, k1.Time(), m1.x, local_t);
    float y = EvaluateHermite(k0.Value(), m0.y, k1.Value(), m1.y, local_t);

    return normalize_point(ImVec2(x, y));
}

// === Tangent Calculation Helpers ===

float CalculateAutoTangentSlope(const Keyframe* prev, const Keyframe& current, const Keyframe* next)
{
    // Clamped Catmull-Rom tangent calculation

    if (!prev && !next)
    {
        return 0.0f;  // Flat
    }

    if (!prev)
    {
        // First keyframe: tangent points toward next
        float dx = next->m_pos.x - current.m_pos.x;
        float dy = next->m_pos.y - current.m_pos.y;
        if (std::abs(dx) < 1e-6f) return 0.0f;
        return dy / dx;
    }

    if (!next)
    {
        // Last keyframe: tangent points from previous
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
        return 0.0f;  // Flat tangent to prevent overshoot
    }

    // Standard Catmull-Rom: tangent parallel to line between neighbors
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

float CalculateLinearTangentSlope(const Keyframe& current, const Keyframe& adjacent)
{
    float dx = adjacent.m_pos.x - current.m_pos.x;
    float dy = adjacent.m_pos.y - current.m_pos.y;
    if (std::abs(dx) < 1e-6f) return 0.0f;
    return dy / dx;
}

// === Tangent Resolution ===

void ResolveKeyframeTangents(Keyframe& keyframe, const Keyframe* prev_key, const Keyframe* next_key)
{
    // Resolve in-tangent
    if (prev_key)
    {
        Tangent& in_tan = keyframe.m_in;
        float segment_duration = keyframe.Time() - prev_key->Time();
        float default_x = -segment_duration / 3.0f;  // Points left

        if (keyframe.m_tangent_type == TangentType::SMOOTH)
        {
            switch (in_tan.m_smooth_type)
            {
                case Tangent::SmoothType::AUTO:
                {
                    float slope = CalculateAutoTangentSlope(prev_key, keyframe, next_key);
                    in_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Tangent::SmoothType::FLAT:
                    in_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Tangent::SmoothType::FREE:
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
                case Tangent::SmoothType::UNUSED:
                    break;
            }
        }
        else  // BROKEN
        {
            switch (in_tan.m_broken_type)
            {
                case Tangent::BrokenType::LINEAR:
                {
                    float slope = CalculateLinearTangentSlope(keyframe, *prev_key);
                    in_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Tangent::BrokenType::CONSTANT:
                    in_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Tangent::BrokenType::FREE:
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
                case Tangent::BrokenType::UNUSED:
                    break;
            }
        }
    }

    // Resolve out-tangent
    if (next_key)
    {
        Tangent& out_tan = keyframe.m_out;
        float segment_duration = next_key->Time() - keyframe.Time();
        float default_x = segment_duration / 3.0f;  // Points right

        if (keyframe.m_tangent_type == TangentType::SMOOTH)
        {
            switch (out_tan.m_smooth_type)
            {
                case Tangent::SmoothType::AUTO:
                {
                    float slope = CalculateAutoTangentSlope(prev_key, keyframe, next_key);
                    out_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Tangent::SmoothType::FLAT:
                    out_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Tangent::SmoothType::FREE:
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
                case Tangent::SmoothType::UNUSED:
                    break;
            }
        }
        else  // BROKEN
        {
            switch (out_tan.m_broken_type)
            {
                case Tangent::BrokenType::LINEAR:
                {
                    float slope = CalculateLinearTangentSlope(keyframe, *next_key);
                    out_tan.m_offset = ImVec2(default_x, default_x * slope);
                    break;
                }
                case Tangent::BrokenType::CONSTANT:
                    out_tan.m_offset = ImVec2(default_x, 0.0f);
                    break;
                case Tangent::BrokenType::FREE:
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
                case Tangent::BrokenType::UNUSED:
                    break;
            }
        }
    }
}

void ResolveCurveTangents(Curve& curve)
{
    int count = (int)curve.m_keyframes.size();

    for (int i = 0; i < count; i++)
    {
        const Keyframe* prev = (i > 0) ? &curve.m_keyframes.at(i - 1) : nullptr;
        const Keyframe* next = (i < count - 1) ? &curve.m_keyframes.at(i + 1) : nullptr;

        ResolveKeyframeTangents(curve.m_keyframes.at(i), prev, next);
    }
}

// === Tangent Constraint Helpers ===

void MirrorTangentDir(Keyframe& keyframe, bool from_out_to_in)
{
    if (from_out_to_in)
    {
        // Mirror out-tangent to in-tangent (negate both components)
        keyframe.m_in.m_offset = ImVec2(-keyframe.m_out.m_offset.x, -keyframe.m_out.m_offset.y);
    }
    else
    {
        // Mirror in-tangent to out-tangent (negate both components)
        keyframe.m_out.m_offset = ImVec2(-keyframe.m_in.m_offset.x, -keyframe.m_in.m_offset.y);
    }
}

void ValidateTangentDir(Tangent& tangent, bool is_in_tangent)
{
    if (is_in_tangent)
    {
        // In-tangent must point left (negative x)
        if (tangent.m_offset.x > 0)
        {
            tangent.m_offset.x = -tangent.m_offset.x;
        }
    }
    else
    {
        // Out-tangent must point right (positive x)
        if (tangent.m_offset.x < 0)
        {
            tangent.m_offset.x = -tangent.m_offset.x;
        }
    }
}

}  // namespace tanim
