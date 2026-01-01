#pragma once

#include "tanim/include/keyframe.hpp"

namespace tanim
{

// === Keyframe Management ===

// Add a keyframe at the given time/value. Maintains sorted order by time.
// Returns the index of the new keyframe, or -1 if a keyframe already exists at that time.
int AddKeyframe(Curve& curve, float time, float value);

// Remove a keyframe by index. Cannot remove first or last keyframe.
// Returns true if removed, false if index invalid or is first/last.
bool RemoveKeyframe(Curve& curve, int keyframe_index);

// Move a keyframe to a new position. Maintains constraints.
// Cannot move past adjacent keyframes (no reordering).
// First keyframe's time is locked.
void MoveKeyframe(Curve& curve, int keyframe_index, ImVec2 new_pos);

// === Mode Changes (Context Menu Actions) ===

// Set keyframe to SMOOTH mode with specified type.
// Both tangents get the same SmoothType, BrokenTypes set to UNUSED.
// Tangent directions are mirrored.
void SetKeyframeSmoothType(Curve& curve, int keyframe_index, Tangent::SmoothType type);

// Set keyframe to BROKEN mode with specified types for both tangents.
// SmoothTypes set to UNUSED.
void SetKeyframeBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType in_type, Tangent::BrokenType out_type);

// === Individual Tangent Changes (for In Tangent / Out Tangent submenus) ===

// Set in-tangent to BROKEN mode with specified type.
// If keyframe was SMOOTH, it becomes BROKEN.
// If type is CONSTANT, also sets previous keyframe's out-tangent to CONSTANT.
void SetInTangentBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type);

// Set out-tangent to BROKEN mode with specified type.
// If keyframe was SMOOTH, it becomes BROKEN.
// If type is CONSTANT, also sets next keyframe's in-tangent to CONSTANT.
void SetOutTangentBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type);

// Set both tangents to BROKEN mode with specified type (for "Both Tangents" submenu).
// Propagates CONSTANT to adjacent keyframes if applicable.
void SetBothTangentsBrokenType(Curve& curve, int keyframe_index, Tangent::BrokenType type);

// === Weight Toggle ===

void SetInTangentWeighted(Curve& curve, int keyframe_index, bool weighted);
void SetOutTangentWeighted(Curve& curve, int keyframe_index, bool weighted);
void SetBothTangentsWeighted(Curve& curve, int keyframe_index, bool weighted);

// === Tangent Manipulation (for handle dragging) ===

void SetInTangentOffset(Curve& curve, int keyframe_index, ImVec2 offset);
void SetOutTangentOffset(Curve& curve, int keyframe_index, ImVec2 offset);

// === Query Functions ===

inline int GetKeyframeCount(const Curve& curve) { return static_cast<int>(curve.m_keyframes.size()); }

inline const Keyframe& GetKeyframe(const Curve& curve, int index) { return curve.m_keyframes.at(index); }

inline Keyframe& GetKeyframeMut(Curve& curve, int index) { return curve.m_keyframes.at(index); }

// Check if in-tangent handle should be shown (not first keyframe, not LINEAR/CONSTANT in BROKEN mode)
bool ShouldShowInTangentHandle(const Curve& curve, int keyframe_index);

// Check if out-tangent handle should be shown (not last keyframe, not LINEAR/CONSTANT in BROKEN mode)
bool ShouldShowOutTangentHandle(const Curve& curve, int keyframe_index);

// Check if in-tangent is editable (not first keyframe)
inline bool IsInTangentEditable(int keyframe_index) { return keyframe_index > 0; }

// Check if out-tangent is editable (not last keyframe)
inline bool IsOutTangentEditable(const Curve& curve, int keyframe_index)
{
    return keyframe_index < GetKeyframeCount(curve) - 1;
}

}  // namespace tanim
