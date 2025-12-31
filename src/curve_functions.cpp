#include "tanim/include/curve_functions.hpp"
#include "tanim/include/hermite.hpp"

#include <algorithm>
#include <cmath>

namespace tanim
{

// === Keyframe Management ===

int AddKeyframe(Curve& curve, float time, float value)
{
    auto& keyframes = curve.m_keyframes;

    // Find insertion position and check for duplicates
    int insert_idx = (int)keyframes.size();
    for (int i = 0; i < (int)keyframes.size(); i++)
    {
        if (std::abs(keyframes.at(i).Time() - time) < 1e-6f)
        {
            return -1;  // Duplicate
        }
        if (time < keyframes.at(i).Time())
        {
            insert_idx = i;
            break;
        }
    }

    // Create new keyframe with default SMOOTH AUTO
    const Keyframe new_key(time, value);

    // Insert
    keyframes.insert(keyframes.begin() + insert_idx, new_key);

    // Resolve all tangents (AUTO tangents need neighbor info)
    ResolveCurveTangents(curve);

    return insert_idx;
}

bool RemoveKeyframe(Curve& curve, int keyframe_index)
{
    const int count = GetKeyframeCount(curve);

    // Cannot remove first or last keyframe
    if (keyframe_index <= 0 || keyframe_index >= count - 1) return false;

    curve.m_keyframes.erase(curve.m_keyframes.begin() + keyframe_index);

    // Resolve tangents for affected keyframes
    ResolveCurveTangents(curve);

    return true;
}

void MoveKeyframe(Curve& curve, int keyframe_index, ImVec2 new_pos)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // Constrain time to not pass adjacent keyframes
    float min_time = (keyframe_index > 0) ? curve.m_keyframes.at(keyframe_index - 1).Time() + 1.0f : new_pos.x;
    float max_time = (keyframe_index < count - 1) ? curve.m_keyframes.at(keyframe_index + 1).Time() - 1.0f : new_pos.x;

    // First keyframe time is locked
    if (keyframe_index == 0)
    {
        new_pos.x = key.m_pos.x;
    }

    new_pos.x = std::clamp(new_pos.x, min_time, max_time);
    new_pos.x = std::floor(new_pos.x);  // Snap to integer frame

    key.m_pos = new_pos;

    // Resolve tangents (segment durations may have changed)
    ResolveCurveTangents(curve);
}

// === Mode Changes ===

void SetKeyframeSmoothType(Curve& curve, int keyframe_index, Tangent::SmoothType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    key.m_tangent_type = TangentType::SMOOTH;
    key.m_in.m_smooth_type = type;
    key.m_out.m_smooth_type = type;
    key.m_in.m_broken_type = Tangent::BrokenType::UNUSED;
    key.m_out.m_broken_type = Tangent::BrokenType::UNUSED;

    // For FREE mode, ensure directions are mirrored
    if (type == Tangent::SmoothType::FREE)
    {
        MirrorTangentDir(key, true);  // out -> in
    }

    ResolveCurveTangents(curve);
}

void SetKeyframeBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType in_type, Tangent::BrokenType out_type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    key.m_tangent_type = TangentType::BROKEN;
    key.m_in.m_broken_type = in_type;
    key.m_out.m_broken_type = out_type;
    key.m_in.m_smooth_type = Tangent::SmoothType::UNUSED;
    key.m_out.m_smooth_type = Tangent::SmoothType::UNUSED;

    ResolveCurveTangents(curve);
}

// === Individual Tangent Changes ===

void SetInTangentBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;  // First keyframe has no in-tangent

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If was SMOOTH, convert other tangent to BROKEN FREE
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        key.m_out.m_broken_type = Tangent::BrokenType::FREE;
        key.m_out.m_smooth_type = Tangent::SmoothType::UNUSED;
    }

    key.m_tangent_type = TangentType::BROKEN;
    key.m_in.m_broken_type = type;
    key.m_in.m_smooth_type = Tangent::SmoothType::UNUSED;

    // If CONSTANT, propagate to previous keyframe's out-tangent
    if (type == Tangent::BrokenType::CONSTANT && keyframe_index > 0)
    {
        Keyframe& prev = curve.m_keyframes.at(keyframe_index - 1);

        if (prev.m_tangent_type == TangentType::SMOOTH)
        {
            prev.m_in.m_broken_type = Tangent::BrokenType::FREE;
            prev.m_in.m_smooth_type = Tangent::SmoothType::UNUSED;
        }

        prev.m_tangent_type = TangentType::BROKEN;
        prev.m_out.m_broken_type = Tangent::BrokenType::CONSTANT;
        prev.m_out.m_smooth_type = Tangent::SmoothType::UNUSED;
    }

    ResolveCurveTangents(curve);
}

void SetOutTangentBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;  // Last keyframe has no out-tangent

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If was SMOOTH, convert other tangent to BROKEN FREE
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        key.m_in.m_broken_type = Tangent::BrokenType::FREE;
        key.m_in.m_smooth_type = Tangent::SmoothType::UNUSED;
    }

    key.m_tangent_type = TangentType::BROKEN;
    key.m_out.m_broken_type = type;
    key.m_out.m_smooth_type = Tangent::SmoothType::UNUSED;

    // If CONSTANT, propagate to next keyframe's in-tangent
    if (type == Tangent::BrokenType::CONSTANT && keyframe_index < count - 1)
    {
        Keyframe& next = curve.m_keyframes.at(keyframe_index + 1);

        if (next.m_tangent_type == TangentType::SMOOTH)
        {
            next.m_out.m_broken_type = Tangent::BrokenType::FREE;
            next.m_out.m_smooth_type = Tangent::SmoothType::UNUSED;
        }

        next.m_tangent_type = TangentType::BROKEN;
        next.m_in.m_broken_type = Tangent::BrokenType::CONSTANT;
        next.m_in.m_smooth_type = Tangent::SmoothType::UNUSED;
    }

    ResolveCurveTangents(curve);
}

void SetBothTangentsBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    // Set in-tangent (if not first keyframe)
    if (keyframe_index > 0)
    {
        SetInTangentBrokenType(curve, keyframe_index, type);
    }

    // Set out-tangent (if not last keyframe)
    if (keyframe_index < count - 1)
    {
        SetOutTangentBrokenType(curve, keyframe_index, type);
    }
}

// === Weight Toggle ===

void SetInTangentWeighted(Curve& curve, int keyframe_index, bool weighted)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;

    curve.m_keyframes.at(keyframe_index).m_in.m_weighted = weighted;
    ResolveCurveTangents(curve);
}

void SetOutTangentWeighted(Curve& curve, int keyframe_index, bool weighted)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;

    curve.m_keyframes.at(keyframe_index).m_out.m_weighted = weighted;
    ResolveCurveTangents(curve);
}

void SetBothTangentsWeighted(Curve& curve, int keyframe_index, bool weighted)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (keyframe_index > 0)
    {
        key.m_in.m_weighted = weighted;
    }

    if (keyframe_index < count - 1)
    {
        key.m_out.m_weighted = weighted;
    }

    ResolveCurveTangents(curve);
}

// === Tangent Manipulation ===

void SetInTangentDir(Curve& curve, int keyframe_index, ImVec2 new_dir)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If AUTO or FLAT, convert to FREE
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        if (key.m_in.m_smooth_type == Tangent::SmoothType::AUTO || key.m_in.m_smooth_type == Tangent::SmoothType::FLAT)
        {
            key.m_in.m_smooth_type = Tangent::SmoothType::FREE;
            key.m_out.m_smooth_type = Tangent::SmoothType::FREE;
        }
    }

    key.m_in.m_dir = NormalizeVec2(new_dir, false);
    ValidateTangentDir(key.m_in, true);

    // If SMOOTH, mirror to out-tangent
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        MirrorTangentDir(key, false);  // in -> out
    }

    ResolveCurveTangents(curve);
}

void SetOutTangentDir(Curve& curve, int keyframe_index, ImVec2 new_dir)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If AUTO or FLAT, convert to FREE
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        if (key.m_out.m_smooth_type == Tangent::SmoothType::AUTO || key.m_out.m_smooth_type == Tangent::SmoothType::FLAT)
        {
            key.m_in.m_smooth_type = Tangent::SmoothType::FREE;
            key.m_out.m_smooth_type = Tangent::SmoothType::FREE;
        }
    }

    key.m_out.m_dir = NormalizeVec2(new_dir, true);
    ValidateTangentDir(key.m_out, false);

    // If SMOOTH, mirror to in-tangent
    if (key.m_tangent_type == TangentType::SMOOTH)
    {
        MirrorTangentDir(key, true);  // out -> in
    }

    ResolveCurveTangents(curve);
}

void SetInTangentWeight(Curve& curve, int keyframe_index, float weight)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (!key.m_in.m_weighted) return;  // Only works when weighted

    key.m_in.m_weight = std::max(0.0f, weight);
}

void SetOutTangentWeight(Curve& curve, int keyframe_index, float weight)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (!key.m_out.m_weighted) return;  // Only works when weighted

    key.m_out.m_weight = std::max(0.0f, weight);
}

// === Query Functions ===

bool ShouldShowInTangentHandle(const Curve& curve, int keyframe_index)
{
    if (keyframe_index <= 0) return false;  // First keyframe

    const Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (key.m_tangent_type == TangentType::BROKEN)
    {
        // Don't show for LINEAR or CONSTANT
        if (key.m_in.m_broken_type == Tangent::BrokenType::LINEAR || key.m_in.m_broken_type == Tangent::BrokenType::CONSTANT)
        {
            return false;
        }
    }

    return true;
}

bool ShouldShowOutTangentHandle(const Curve& curve, int keyframe_index)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index >= count - 1) return false;  // Last keyframe

    const Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (key.m_tangent_type == TangentType::BROKEN)
    {
        // Don't show for LINEAR or CONSTANT
        if (key.m_out.m_broken_type == Tangent::BrokenType::LINEAR || key.m_out.m_broken_type == Tangent::BrokenType::CONSTANT)
        {
            return false;
        }
    }

    return true;
}

}  // namespace tanim
