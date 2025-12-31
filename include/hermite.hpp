#pragma once

#include "tanim/include/keyframe.hpp"
#include "tanim/include/includes.hpp"

namespace tanim
{

// === Hermite Spline Evaluation ===

// Evaluate cubic Hermite spline at parameter t in [0, 1]
// p0, p1: endpoint values
// m0: outgoing tangent at p0 (slope, already accounting for weight)
// m1: incoming tangent at p1 (slope, already accounting for weight)
float EvaluateHermite(float p0, float m0, float p1, float m1, float t);

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

// === Tangent Resolution (call after any keyframe/tangent modification) ===

// Resolve all tangent m_dir and m_weight values in the curve.
// Call this after any modification to keyframes or tangent settings.
void ResolveCurveTangents(Curve& curve);

// Resolve tangents for a single keyframe.
// prev_key, next_key can be nullptr for first/last keyframes.
void ResolveKeyframeTangents(Keyframe& keyframe, const Keyframe* prev_key, const Keyframe* next_key);

// === Tangent Calculation Helpers ===

// Calculate auto tangent direction using clamped Catmull-Rom.
// Returns normalized direction vector (positive x, pointing right).
ImVec2 CalculateAutoTangentDir(const Keyframe* prev, const Keyframe& current, const Keyframe* next);

// Calculate linear tangent direction pointing at adjacent keyframe.
// Returns normalized direction vector.
ImVec2 CalculateLinearTangentDir(const Keyframe& current, const Keyframe& adjacent, bool is_in_tangent);

// === Tangent Constraint Helpers ===

// Mirror the out-tangent direction to create in-tangent (or vice versa).
// Only affects m_dir, preserves individual m_weight values.
void MirrorTangentDir(Keyframe& keyframe, bool from_out_to_in);

// Ensure tangent m_dir points in valid direction and is normalized.
// In-tangent: negative x (pointing left), Out-tangent: positive x (pointing right)
void ValidateTangentDir(Tangent& tangent, bool is_in_tangent);

// === Vector Utilities ===

// Normalize a vector, returns default direction if near zero.
ImVec2 NormalizeVec2(const ImVec2& v, bool default_positive_x = true);

// Get length of vector.
float Vec2Length(const ImVec2& v);

// Get slope (dy/dx) from direction vector. Returns 0 if dx is near zero.
float GetSlope(const ImVec2& dir);

}  // namespace tanim
