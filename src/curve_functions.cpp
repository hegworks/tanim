#include "tanim/include/curve_functions.hpp"
#include "tanim/include/bezier.hpp"

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

    // Resolve all handles (AUTO handles need neighbor info)
    ResolveCurveHandles(curve);

    return insert_idx;
}

bool RemoveKeyframe(Curve& curve, int keyframe_index)
{
    const int count = GetKeyframeCount(curve);

    // Cannot remove first or last keyframe
    if (keyframe_index <= 0 || keyframe_index >= count - 1) return false;

    curve.m_keyframes.erase(curve.m_keyframes.begin() + keyframe_index);

    // Resolve handles for affected keyframes
    ResolveCurveHandles(curve);

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

    // Resolve handles (segment durations may have changed)
    ResolveCurveHandles(curve);
}

// === Mode Changes ===

void SetKeyframeSmoothType(Curve& curve, int keyframe_index, Handle::SmoothType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    key.m_handle_type = HandleType::SMOOTH;
    key.m_in.m_smooth_type = type;
    key.m_out.m_smooth_type = type;
    key.m_in.m_broken_type = Handle::BrokenType::UNUSED;
    key.m_out.m_broken_type = Handle::BrokenType::UNUSED;

    // For FREE mode, ensure directions are mirrored
    if (type == Handle::SmoothType::FREE)
    {
        MirrorHandlesDir(key, true);  // out -> in
    }

    ResolveCurveHandles(curve);
}

void SetKeyframeBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType in_type, Handle::BrokenType out_type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    key.m_handle_type = HandleType::BROKEN;
    key.m_in.m_broken_type = in_type;
    key.m_out.m_broken_type = out_type;
    key.m_in.m_smooth_type = Handle::SmoothType::UNUSED;
    key.m_out.m_smooth_type = Handle::SmoothType::UNUSED;

    ResolveCurveHandles(curve);
}

// === Individual Handle Changes ===

void SetInHandleBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;  // First keyframe has no in-handle

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If was SMOOTH, convert other handle to BROKEN FREE
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        key.m_out.m_broken_type = Handle::BrokenType::FREE;
        key.m_out.m_smooth_type = Handle::SmoothType::UNUSED;
    }

    key.m_handle_type = HandleType::BROKEN;
    key.m_in.m_broken_type = type;
    key.m_in.m_smooth_type = Handle::SmoothType::UNUSED;

    // If CONSTANT, propagate to previous keyframe's out-handle
    if (type == Handle::BrokenType::CONSTANT && keyframe_index > 0)
    {
        Keyframe& prev = curve.m_keyframes.at(keyframe_index - 1);

        if (prev.m_handle_type == HandleType::SMOOTH)
        {
            prev.m_in.m_broken_type = Handle::BrokenType::FREE;
            prev.m_in.m_smooth_type = Handle::SmoothType::UNUSED;
        }

        prev.m_handle_type = HandleType::BROKEN;
        prev.m_out.m_broken_type = Handle::BrokenType::CONSTANT;
        prev.m_out.m_smooth_type = Handle::SmoothType::UNUSED;
    }

    ResolveCurveHandles(curve);
}

void SetOutHandleBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;  // Last keyframe has no out-handle

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If was SMOOTH, convert other handle to BROKEN FREE
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        key.m_in.m_broken_type = Handle::BrokenType::FREE;
        key.m_in.m_smooth_type = Handle::SmoothType::UNUSED;
    }

    key.m_handle_type = HandleType::BROKEN;
    key.m_out.m_broken_type = type;
    key.m_out.m_smooth_type = Handle::SmoothType::UNUSED;

    // If CONSTANT, propagate to next keyframe's in-handle
    if (type == Handle::BrokenType::CONSTANT && keyframe_index < count - 1)
    {
        Keyframe& next = curve.m_keyframes.at(keyframe_index + 1);

        if (next.m_handle_type == HandleType::SMOOTH)
        {
            next.m_out.m_broken_type = Handle::BrokenType::FREE;
            next.m_out.m_smooth_type = Handle::SmoothType::UNUSED;
        }

        next.m_handle_type = HandleType::BROKEN;
        next.m_in.m_broken_type = Handle::BrokenType::CONSTANT;
        next.m_in.m_smooth_type = Handle::SmoothType::UNUSED;
    }

    ResolveCurveHandles(curve);
}

void SetBothHandlesBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count) return;

    // Set in-handle (if not first keyframe)
    if (keyframe_index > 0)
    {
        SetInHandleBrokenType(curve, keyframe_index, type);
    }

    // Set out-handle (if not last keyframe)
    if (keyframe_index < count - 1)
    {
        SetOutHandleBrokenType(curve, keyframe_index, type);
    }
}

// === Weight Toggle ===

void SetInHandleWeighted(Curve& curve, int keyframe_index, bool weighted)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;

    curve.m_keyframes.at(keyframe_index).m_in.m_weighted = weighted;
    ResolveCurveHandles(curve);
}

void SetOutHandleWeighted(Curve& curve, int keyframe_index, bool weighted)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;

    curve.m_keyframes.at(keyframe_index).m_out.m_weighted = weighted;
    ResolveCurveHandles(curve);
}

void SetBothHandlesWeighted(Curve& curve, int keyframe_index, bool weighted)
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

    ResolveCurveHandles(curve);
}

// === Handle Manipulation ===

void SetInHandleOffset(Curve& curve, int keyframe_index, ImVec2 offset)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index <= 0 || keyframe_index >= count) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If AUTO or FLAT, convert to FREE
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        if (key.m_in.m_smooth_type == Handle::SmoothType::AUTO || key.m_in.m_smooth_type == Handle::SmoothType::FLAT)
        {
            key.m_in.m_smooth_type = Handle::SmoothType::FREE;
            key.m_out.m_smooth_type = Handle::SmoothType::FREE;
        }
    }

    // Ensure it points left
    if (offset.x > 0) offset.x = -offset.x;
    key.m_in.m_offset = offset;

    // Resolve before mirroring to apply clamping handles
    ResolveCurveHandles(curve);

    // If SMOOTH, mirror to out-handle
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        MirrorHandlesDir(key, false);  // in -> out
    }

    ResolveCurveHandles(curve);
}

void SetOutHandleOffset(Curve& curve, int keyframe_index, ImVec2 offset)
{
    int count = GetKeyframeCount(curve);
    if (keyframe_index < 0 || keyframe_index >= count - 1) return;

    Keyframe& key = curve.m_keyframes.at(keyframe_index);

    // If AUTO or FLAT, convert to FREE
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        if (key.m_out.m_smooth_type == Handle::SmoothType::AUTO || key.m_out.m_smooth_type == Handle::SmoothType::FLAT)
        {
            key.m_in.m_smooth_type = Handle::SmoothType::FREE;
            key.m_out.m_smooth_type = Handle::SmoothType::FREE;
        }
    }

    // Ensure it points right
    if (offset.x < 0) offset.x = -offset.x;
    key.m_out.m_offset = offset;

    // Resolve before mirroring to apply clamping handles
    ResolveCurveHandles(curve);

    // If SMOOTH, mirror to in-handle
    if (key.m_handle_type == HandleType::SMOOTH)
    {
        MirrorHandlesDir(key, true);  // out -> in
    }

    ResolveCurveHandles(curve);
}

// === Query Functions ===

bool ShouldShowInHandleHandle(const Curve& curve, int keyframe_index)
{
    if (!IsInHandleEditable(curve, keyframe_index)) return false;

    const Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (key.m_handle_type == HandleType::BROKEN)
    {
        // Don't show for LINEAR or CONSTANT
        if (key.m_in.m_broken_type == Handle::BrokenType::LINEAR || key.m_in.m_broken_type == Handle::BrokenType::CONSTANT)
        {
            return false;
        }
    }

    return true;
}

bool ShouldShowOutHandle(const Curve& curve, int keyframe_index)
{
    if (!IsOutHandleEditable(curve, keyframe_index)) return false;

    const Keyframe& key = curve.m_keyframes.at(keyframe_index);

    if (key.m_handle_type == HandleType::BROKEN)
    {
        // Don't show for LINEAR or CONSTANT
        if (key.m_out.m_broken_type == Handle::BrokenType::LINEAR || key.m_out.m_broken_type == Handle::BrokenType::CONSTANT)
        {
            return false;
        }
    }

    return true;
}

bool IsInHandleEditable(const Curve& curve, int keyframe_index)
{
    const bool is_first_frame = keyframe_index <= 0;
    const bool is_prev_keyframe_out_constant =
        !is_first_frame && GetKeyframe(curve, keyframe_index - 1).m_out.m_broken_type == Handle::BrokenType::CONSTANT;
    return !is_first_frame && !is_prev_keyframe_out_constant;
}

}  // namespace tanim
