#pragma once

#include "tanim/include/keyframe.hpp"
#include "tanim/include/includes.hpp"

namespace tanim
{

// === Hermite Spline Evaluation ===

// Cubic Bezier evaluation: P(t) = (1-t)^3*P0 + 3(1-t)^2*tP1 + 3(1-t)t^2*P2 + t^3*P3
ImVec2 CubicBezier(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t);

// === Curve Sampling (reads pre-computed m_dir and m_weight directly) ===

// Sample curve for animation playback (returns Y value at given time/frame)
// Handles CONSTANT segments appropriately
float SampleCurveValue(const Curve& curve, float time);

// Sample curve for drawing (returns normalized position for UI rendering)
// t: normalized parameter across entire curve [0, 1]
// min, max: view bounds for normalization
ImVec2 SampleCurveForDrawing(const Curve& curve, float t, const ImVec2& min, const ImVec2& max);

// Find which segment contains the given time
// Returns the index of the keyframe at the start of the segment, or -1 if before first keyframe
int FindSegmentIndex(const Curve& curve, float time);

// Find parameter t for a given X value using Newton-Raphson iteration.
// p0, p1, p2, p3: Bezier control points
// target_x: the X value to find
float FindTForX(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float target_x);

// === Handle Resolution (call after any keyframe/handle modification) ===

// Resolve all handle m_dir and m_weight values in the curve.
// Call this after any modification to keyframes or handle settings.
void ResolveCurveHandles(Curve& curve);

// Resolve handles for a single keyframe.
// prev_key, next_key can be nullptr for first/last keyframes.
void ResolveKeyframeHandles(Keyframe& keyframe, const Keyframe* prev_key, const Keyframe* next_key);

// === Handle Calculation Helpers ===

// Clamped Catmull-Rom handle calculation
float CalculateAutoHandleSlope(const Keyframe* prev, const Keyframe& current, const Keyframe* next);

// Calculate linear handle direction pointing at adjacent keyframe.
// Returns normalized direction vector.
float CalculateLinearHandleSlope(const Keyframe& current, const Keyframe& adjacent);

// === Handle Constraint Helpers ===

// Mirror the out-handle direction to create in-handle (or vice versa).
// Only affects m_dir, preserves individual m_weight values.
void MirrorHandlesDir(Keyframe& keyframe, bool from_out_to_in);

// Ensure handle m_dir points in valid direction and is normalized.
// In-handle: negative x (pointing left), Out-handle: positive x (pointing right)
void ValidateHandleDir(Handle& handle, bool is_in_handle);

// === Vector Utilities ===

// Get length of vector.
float Vec2Length(const ImVec2& v);

}  // namespace tanim
