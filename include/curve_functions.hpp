#pragma once

#include "tanim/include/keyframe.hpp"

namespace tanim
{

// === Curve Management ===

void EnforceCurveType(Curve& curve, EnforcedType enforced_type);

void ApplyEnforcedCurveTypeOnKeyframe(Curve& curve, int keyframe_index);

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
// Both handles get the same SmoothType, BrokenTypes set to UNUSED.
// Handle directions are mirrored.
void SetKeyframeSmoothType(Curve& curve, int keyframe_index, Handle::SmoothType type);

// Set keyframe to BROKEN mode with specified types for both handles.
// SmoothTypes set to UNUSED.
void SetKeyframeBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType in_type, Handle::BrokenType out_type);

// === Individual Handle Changes (for In Handle / Out Handle submenus) ===

// Set in-handle to BROKEN mode with specified type.
// If keyframe was SMOOTH, it becomes BROKEN.
// If type is CONSTANT, also sets previous keyframe's out-handle to CONSTANT.
void SetInHandleBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type);

// Set out-handle to BROKEN mode with specified type.
// If keyframe was SMOOTH, it becomes BROKEN.
// If type is CONSTANT, also sets next keyframe's in-handle to CONSTANT.
void SetOutHandleBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type);

// Set both handles to BROKEN mode with specified type (for "Both Handles" submenu).
// Propagates CONSTANT to adjacent keyframes if applicable.
void SetBothHandlesBrokenType(Curve& curve, int keyframe_index, Handle::BrokenType type);

// === Weight Toggle ===

void SetInHandleWeighted(Curve& curve, int keyframe_index, bool weighted);
void SetOutHandleWeighted(Curve& curve, int keyframe_index, bool weighted);
void SetBothHandlesWeighted(Curve& curve, int keyframe_index, bool weighted);

// === Handle Manipulation (for handle dragging) ===

void SetInHandleOffset(Curve& curve, int keyframe_index, ImVec2 offset);
void SetOutHandleOffset(Curve& curve, int keyframe_index, ImVec2 offset);

// === Query Functions ===

inline int GetKeyframeCount(const Curve& curve) { return static_cast<int>(curve.m_keyframes.size()); }

inline const Keyframe& GetKeyframe(const Curve& curve, int index) { return curve.m_keyframes.at(index); }

inline Keyframe& GetKeyframeMut(Curve& curve, int index) { return curve.m_keyframes.at(index); }

// Check if in-handle handle should be shown (not first keyframe, not LINEAR/CONSTANT in BROKEN mode)
bool ShouldShowInHandleHandle(const Curve& curve, int keyframe_index);

// Check if out-handle handle should be shown (not last keyframe, not LINEAR/CONSTANT in BROKEN mode)
bool ShouldShowOutHandle(const Curve& curve, int keyframe_index);

// Check if in-handle is editable
bool IsInHandleEditable(const Curve& curve, int keyframe_index);

// Check if out-handle is editable (not last keyframe)
bool IsOutHandleEditable(const Curve& curve, int keyframe_index);

}  // namespace tanim
