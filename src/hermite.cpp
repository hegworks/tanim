#include "tanim/include/hermite.hpp"

#include <algorithm>
#include <cmath>

namespace tanim
{

// === Vector Utilities ===

float Vec2Length(const ImVec2& v) { return std::sqrt(v.x * v.x + v.y * v.y); }

ImVec2 NormalizeVec2(const ImVec2& v, bool default_positive_x)
{
    float len = Vec2Length(v);
    if (len < 1e-6f)
    {
        return default_positive_x ? ImVec2(1.0f, 0.0f) : ImVec2(-1.0f, 0.0f);
    }
    return ImVec2(v.x / len, v.y / len);
}

float GetSlope(const ImVec2& dir)
{
    if (std::abs(dir.x) < 1e-6f) return 0.0f;
    return dir.y / std::abs(dir.x);
}

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

float SampleCurveValue(const Curve& curve, float time)
{
    const auto& keyframes = curve.m_keyframes;
    int count = (int)keyframes.size();

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

    // Hermite interpolation
    float segment_duration = k1.Time() - k0.Time();
    if (segment_duration < 1e-6f) return k0.Value();

    float t = (time - k0.Time()) / segment_duration;

    // Compute Hermite tangents: slope * segment_duration
    float slope0 = GetSlope(k0.m_out.m_dir);
    float slope1 = GetSlope(k1.m_in.m_dir);

    float m0 = slope0 * k0.m_out.m_weight;
    float m1 = slope1 * k1.m_in.m_weight;

    return EvaluateHermite(k0.Value(), m0, k1.Value(), m1, t);
}

ImVec2 SampleCurveForDrawing(const Curve& curve, float t, const ImVec2& min, const ImVec2& max)
{
    const auto& keyframes = curve.m_keyframes;
    int count = (int)keyframes.size();

    auto normalize_point = [&](const ImVec2& p) -> ImVec2
    { return ImVec2((p.x - min.x) / (max.x - min.x + 1.0f), (p.y - min.y) / (max.y - min.y)); };

    if (count == 0) return ImVec2(0.0f, 0.0f);
    if (count == 1) return normalize_point(keyframes.at(0).m_pos);

    t = std::clamp(t, 0.0f, 1.0f);

    // Map t to segment
    float segment_float = t * (count - 1);
    int seg = (int)segment_float;
    if (seg >= count - 1)
    {
        seg = count - 2;
        segment_float = (float)(count - 1);
    }

    float local_t = segment_float - (float)seg;

    const Keyframe& k0 = keyframes.at(seg);
    const Keyframe& k1 = keyframes.at(seg + 1);

    // Check for CONSTANT
    if (k0.m_tangent_type == TangentType::BROKEN && k0.m_out.m_broken_type == Tangent::BrokenType::CONSTANT)
    {
        // Draw as step: horizontal line at k0.y
        float step_x = k0.m_pos.x + local_t * (k1.m_pos.x - k0.m_pos.x);
        return normalize_point(ImVec2(step_x, k0.Value()));
    }

    // Hermite interpolation
    float segment_duration = k1.Time() - k0.Time();
    if (segment_duration < 1e-6f) return normalize_point(k0.m_pos);

    float slope0 = GetSlope(k0.m_out.m_dir);
    float slope1 = GetSlope(k1.m_in.m_dir);

    float m0 = slope0 * k0.m_out.m_weight;
    float m1 = slope1 * k1.m_in.m_weight;

    float x = k0.m_pos.x + local_t * segment_duration;
    float y = EvaluateHermite(k0.Value(), m0, k1.Value(), m1, local_t);

    return normalize_point(ImVec2(x, y));
}

// === Tangent Calculation Helpers ===

ImVec2 CalculateAutoTangentDir(const Keyframe* prev, const Keyframe& current, const Keyframe* next)
{
    // Clamped Catmull-Rom tangent calculation

    if (!prev && !next)
    {
        // Single keyframe, flat tangent
        return ImVec2(1.0f, 0.0f);
    }

    if (!prev)
    {
        // First keyframe: tangent points toward next
        float dx = next->m_pos.x - current.m_pos.x;
        float dy = next->m_pos.y - current.m_pos.y;
        return NormalizeVec2(ImVec2(dx, dy), true);
    }

    if (!next)
    {
        // Last keyframe: tangent points from previous
        float dx = current.m_pos.x - prev->m_pos.x;
        float dy = current.m_pos.y - prev->m_pos.y;
        return NormalizeVec2(ImVec2(dx, dy), true);
    }

    // Interior keyframe: Catmull-Rom with clamping

    // Check for local extremum (clamping)
    bool is_local_max = (current.m_pos.y >= prev->m_pos.y && current.m_pos.y >= next->m_pos.y);
    bool is_local_min = (current.m_pos.y <= prev->m_pos.y && current.m_pos.y <= next->m_pos.y);

    if (is_local_max || is_local_min)
    {
        // Local extremum: flat tangent to prevent overshoot
        return ImVec2(1.0f, 0.0f);
    }

    // Standard Catmull-Rom: tangent parallel to line between neighbors
    float time_span = next->m_pos.x - prev->m_pos.x;
    if (time_span < 1e-6f) return ImVec2(1.0f, 0.0f);

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

    // Return normalized direction vector (always positive x)
    float len = std::sqrt(1.0f + slope * slope);
    return ImVec2(1.0f / len, slope / len);
}

ImVec2 CalculateLinearTangentDir(const Keyframe& current, const Keyframe& adjacent, bool is_in_tangent)
{
    float dx = adjacent.m_pos.x - current.m_pos.x;
    float dy = adjacent.m_pos.y - current.m_pos.y;

    return NormalizeVec2(ImVec2(dx, dy), !is_in_tangent);
}

// === Tangent Resolution ===

void ResolveKeyframeTangents(Keyframe& keyframe, const Keyframe* prev_key, const Keyframe* next_key)
{
    // Resolve in-tangent
    if (prev_key)
    {
        Tangent& in_tan = keyframe.m_in;
        float segment_duration = keyframe.Time() - prev_key->Time();
        float default_weight = segment_duration / 3.0f;

        if (keyframe.m_tangent_type == TangentType::SMOOTH)
        {
            switch (in_tan.m_smooth_type)
            {
                case Tangent::SmoothType::AUTO:
                {
                    ImVec2 auto_dir = CalculateAutoTangentDir(prev_key, keyframe, next_key);
                    in_tan.m_dir = ImVec2(-std::abs(auto_dir.x), -auto_dir.y);  // Flip for in-tangent
                    in_tan.m_weight = default_weight;
                    break;
                }
                case Tangent::SmoothType::FLAT:
                    in_tan.m_dir = ImVec2(-1.0f, 0.0f);
                    in_tan.m_weight = default_weight;
                    break;
                case Tangent::SmoothType::FREE:
                    ValidateTangentDir(in_tan, true);
                    if (!in_tan.m_weighted) in_tan.m_weight = default_weight;
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
                    in_tan.m_dir = CalculateLinearTangentDir(keyframe, *prev_key, true);
                    in_tan.m_weight = default_weight;
                    break;
                case Tangent::BrokenType::CONSTANT:
                    in_tan.m_dir = ImVec2(-1.0f, 0.0f);
                    in_tan.m_weight = default_weight;
                    break;
                case Tangent::BrokenType::FREE:
                    ValidateTangentDir(in_tan, true);
                    if (!in_tan.m_weighted) in_tan.m_weight = default_weight;
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
        float default_weight = segment_duration / 3.0f;

        if (keyframe.m_tangent_type == TangentType::SMOOTH)
        {
            switch (out_tan.m_smooth_type)
            {
                case Tangent::SmoothType::AUTO:
                {
                    ImVec2 auto_dir = CalculateAutoTangentDir(prev_key, keyframe, next_key);
                    out_tan.m_dir = ImVec2(std::abs(auto_dir.x), auto_dir.y);
                    out_tan.m_weight = default_weight;
                    break;
                }
                case Tangent::SmoothType::FLAT:
                    out_tan.m_dir = ImVec2(1.0f, 0.0f);
                    out_tan.m_weight = default_weight;
                    break;
                case Tangent::SmoothType::FREE:
                    ValidateTangentDir(out_tan, false);
                    if (!out_tan.m_weighted) out_tan.m_weight = default_weight;
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
                    out_tan.m_dir = CalculateLinearTangentDir(keyframe, *next_key, false);
                    out_tan.m_weight = default_weight;
                    break;
                case Tangent::BrokenType::CONSTANT:
                    out_tan.m_dir = ImVec2(1.0f, 0.0f);
                    out_tan.m_weight = default_weight;
                    break;
                case Tangent::BrokenType::FREE:
                    ValidateTangentDir(out_tan, false);
                    if (!out_tan.m_weighted) out_tan.m_weight = default_weight;
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
        // Mirror out-tangent direction to in-tangent (negate)
        ImVec2 out_dir = NormalizeVec2(keyframe.m_out.m_dir, true);
        keyframe.m_in.m_dir = ImVec2(-out_dir.x, -out_dir.y);
    }
    else
    {
        // Mirror in-tangent direction to out-tangent (negate)
        ImVec2 in_dir = NormalizeVec2(keyframe.m_in.m_dir, false);
        keyframe.m_out.m_dir = ImVec2(-in_dir.x, -in_dir.y);
    }
}

void ValidateTangentDir(Tangent& tangent, bool is_in_tangent)
{
    // Ensure normalized
    tangent.m_dir = NormalizeVec2(tangent.m_dir, !is_in_tangent);

    if (is_in_tangent)
    {
        // In-tangent must point left (negative x)
        if (tangent.m_dir.x > 0)
        {
            tangent.m_dir.x = -tangent.m_dir.x;
        }
    }
    else
    {
        // Out-tangent must point right (positive x)
        if (tangent.m_dir.x < 0)
        {
            tangent.m_dir.x = -tangent.m_dir.x;
        }
    }
}

}  // namespace tanim
