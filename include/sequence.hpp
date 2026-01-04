// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/entity_data.hpp"
#include "tanim/include/enums.hpp"
#include "tanim/include/keyframe.hpp"
#include "tanim/include/curve_functions.hpp"
#include "tanim/include/bezier.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace tanim
{

struct Sequence
{
    enum class TypeMeta : uint8_t
    {
        NONE = 0,
        INT,
        BOOL,
    };

    std::vector<Curve> m_curves{};
    TypeMeta m_type_meta{TypeMeta::NONE};
    RepresentationMeta m_representation_meta{RepresentationMeta::NONE};
    ImVec2 m_draw_min{0, -1.5f};
    ImVec2 m_draw_max{500, 1.5f};
    int m_first_frame{0};
    int m_last_frame{10};
    bool m_expanded{false};
    SequenceId m_seq_id{};
    bool m_recording{false};
    int m_recording_frame{-1};
    float m_snap_y_value = 0.1f;
    bool m_focused{true};

    Curve& AddCurve() { return m_curves.emplace_back(); }

    bool IsKeyframeInAllCurves(int frame_num) const
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (!GetKeyframeIdx(curve_idx, frame_num).has_value())
            {
                return false;
            }
        }
        return true;
    }

    bool IsKeyframeInAnyCurve(int frame_num) const
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (GetKeyframeIdx(curve_idx, frame_num).has_value())
            {
                return true;
            }
        }
        return false;
    }

    bool IsKeyframeXMoveable(int curve_idx, int frame_idx) const
    {
        const bool is_quat = m_representation_meta == RepresentationMeta::QUAT;
        if (is_quat) return false;

        const bool is_first_keyframe = frame_idx <= 0;
        if (is_first_keyframe) return false;

        const Curve& curve = m_curves.at(curve_idx);

        const bool is_last_keyframe = frame_idx >= GetKeyframeCount(curve) - 1;
        if (is_last_keyframe) return false;

        return true;
    }

    bool IsKeyframeYMoveable(int /*curve_idx*/, int /*frame_idx*/) const
    {
        const bool is_quat = m_representation_meta == RepresentationMeta::QUAT;
        if (is_quat) return false;

        return true;
    }

    void AddNewKeyframe(int frame_num)
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (!GetKeyframeIdx(curve_idx, frame_num).has_value())
            {
                AddKeyframe(m_curves.at(curve_idx), (float)frame_num, 0.0f);
            }
        }
    }

    void DeleteKeyframe(int frame_num)
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (auto opt_idx = GetKeyframeIdx(curve_idx, frame_num); opt_idx.has_value())
            {
                RemoveKeyframe(m_curves.at(curve_idx), opt_idx.value());
            }
        }
    }

    /// turns "a::b::c::d" into "c::d"
    std::string GetNameWithLessColumns() const
    {
        std::string name = m_seq_id.StructFieldName();
        if (name.empty()) return name;

        const size_t last_column_pos = name.find_last_of("::");
        if (last_column_pos == std::string::npos) return name;  // No :: found

        if (last_column_pos < 2) return name;  // Not enough characters before ::

        const std::string before_last_column_str = name.substr(0, last_column_pos - 1);
        if (before_last_column_str.empty()) return name;

        const size_t second_last_column_pos = before_last_column_str.find_last_of("::");
        if (second_last_column_pos == std::string::npos) return name;  // Only one :: pair found

        if (second_last_column_pos + 1 >= name.length()) return name;  // Not enough characters after second ::

        std::string after_second_last_column_str = name.substr(second_last_column_pos + 1);
        return after_second_last_column_str;
    }

    int GetCurveCount() const { return static_cast<int>(m_curves.size()); }

    bool GetCurveVisibility(int curve_idx) const { return m_curves.at(curve_idx).m_visibility; }

    void SetCurveVisibility(int curve_idx, bool visibility) { m_curves.at(curve_idx).m_visibility = visibility; }

    int GetCurveKeyframeCount(int curve_idx) const { return GetKeyframeCount(m_curves.at(curve_idx)); }

    static uint32_t GetCurveColor(int curve_index)
    {
        // TODO(tanim) replace hardcoded colors
        constexpr uint32_t colors[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFF00, 0xFFFF00FF};
        return colors[curve_index % 5];
    }

    // const std::vector<ImVec2>& GetCurvePointsList(int curve_index)
    // {
    //     return reinterpret_cast<const std::vector<ImVec2>&>(m_curves.at(curve_index).m_points);
    // }

    bool IsRecording() const { return m_recording; }

    bool IsBetweenFirstAndLastFrame(int frame_num) const { return frame_num >= m_first_frame && frame_num <= m_last_frame; }

    int GetRecordingFrame() const { return m_recording_frame; }

    void StartRecording(int frame_num)
    {
        m_recording = true;
        m_recording_frame = frame_num;
    }

    void StopRecording()
    {
        m_recording = false;
        m_recording_frame = -1;
    }

    void EditKeyframe(int curve_idx, int keyframe_idx, ImVec2 new_pos)
    {
        Curve& curve = m_curves.at(curve_idx);
        const int keyframe_count = GetKeyframeCount(curve);
        if (keyframe_idx < 0 || keyframe_idx >= keyframe_count) return;

        // Snap X to integer frame
        new_pos.x = std::floor(new_pos.x);

        // Prevent duplicate frames
        if (keyframe_idx + 1 < keyframe_count)
        {
            if (curve.m_keyframes.at(keyframe_idx + 1).Frame() == static_cast<int>(new_pos.x)) return;
        }
        if (keyframe_idx > 0)
        {
            if (curve.m_keyframes.at(keyframe_idx - 1).Frame() == static_cast<int>(new_pos.x)) return;
        }

        // Y-axis snapping
        if (ImGui::GetIO().KeyCtrl && m_snap_y_value > 0.0f)
        {
            new_pos.y = std::round(new_pos.y / m_snap_y_value) * m_snap_y_value;
        }

        // TypeMeta constraints
        if (m_type_meta == TypeMeta::INT)
        {
            new_pos.y = std::floor(new_pos.y);
        }
        else if (m_type_meta == TypeMeta::BOOL)
        {
            new_pos.y = new_pos.y > 0.5f ? 1.0f : 0.0f;
        }

        // RepresentationMeta constraints
        if (m_representation_meta == RepresentationMeta::COLOR)
        {
            new_pos.y = ImClamp(new_pos.y, 0.0f, 1.0f);
        }

        // Clamp first keyframe to first frame
        if (keyframe_idx == 0)
        {
            new_pos.x = static_cast<float>(m_first_frame);
        }

        // Clamp last keyframe to last frame
        if (keyframe_idx == keyframe_count - 1)
        {
            new_pos.x = static_cast<float>(m_last_frame);
        }

        MoveKeyframe(curve, keyframe_idx, new_pos);
    }

    // Returns the index of the new keyframe, or -1 if a keyframe already exists at that time.
    int AddKeyframeAtPos(int curve_idx, ImVec2 pos)
    {
        pos.x = std::floor(pos.x);
        return AddKeyframe(m_curves.at(curve_idx), pos.x, pos.y);
    }

    void RemoveKeyframeAtIdx(int curve_idx, int keyframe_idx) { RemoveKeyframe(m_curves.at(curve_idx), keyframe_idx); }

    std::optional<int> GetKeyframeIdx(int curve_idx, int frame_num) const
    {
        if (curve_idx < 0 || curve_idx >= GetCurveCount()) return std::nullopt;

        const Curve& curve = m_curves.at(curve_idx);
        const int count = GetKeyframeCount(curve);

        for (int i = 0; i < count; ++i)
        {
            if (curve.m_keyframes.at(i).Frame() == frame_num) return i;
        }

        return std::nullopt;
    }

    ImVec2 GetDrawMin() const { return m_draw_min; }

    ImVec2 GetDrawMax() const { return m_draw_max; }

    void SetDrawMin(ImVec2 min)
    {
        m_draw_min = min;
        m_draw_min.x = 0;
    }

    void SetDrawMax(ImVec2 max) { m_draw_max = max; }

    static void BeginEdit(int) { /*TODO(tanim)*/ }
    static void EndEdit() { /*TODO(tanim)*/ }

    // TODO(tanim) replace hardcoded value (maybe?)
    static unsigned int GetBackgroundColor() { return 0x00000000; }

    void ResetHandlesForKeyframe(int curve_idx, int keyframe_idx)
    {
        Curve& curve = m_curves.at(curve_idx);
        SetKeyframeSmoothType(curve, keyframe_idx, Handle::SmoothType::AUTO);
    }

    void EditFirstFrame(int new_first_frame)
    {
        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            const int keyframe_count = GetKeyframeCount(curve);

            for (int k = 0; k < keyframe_count; k++)
            {
                Keyframe& keyframe = curve.m_keyframes.at(k);

                keyframe.m_pos.x = helpers::Remap(static_cast<float>(m_first_frame),
                                                  static_cast<float>(m_last_frame),
                                                  static_cast<float>(new_first_frame),
                                                  static_cast<float>(m_last_frame),
                                                  keyframe.m_pos.x);
            }
        }

        m_first_frame = new_first_frame;
        ClampFirstKeyframesToFirstFrame();
    }

    void EditLastFrame(int new_last_frame)
    {
        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            const int keyframe_count = GetKeyframeCount(curve);

            for (int k = 0; k < keyframe_count; k++)
            {
                Keyframe& keyframe = curve.m_keyframes.at(k);

                keyframe.m_pos.x = helpers::Remap(static_cast<float>(m_first_frame),
                                                  static_cast<float>(m_last_frame),
                                                  static_cast<float>(m_first_frame),
                                                  static_cast<float>(new_last_frame),
                                                  keyframe.m_pos.x);
            }

            ResolveCurveHandles(curve);
        }

        m_last_frame = new_last_frame;
        ClampLastKeyframesToLastFrame();
    }

    void MoveFrames(int frame_diff)
    {
        m_last_frame += frame_diff;
        m_first_frame += frame_diff;

        const float frame_diff_f = static_cast<float>(frame_diff);
        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            for (auto& key : curve.m_keyframes)
            {
                key.m_pos.x += frame_diff_f;
            }
            ResolveCurveHandles(curve);
        }
    }

    void EditSnapY(float value) { m_snap_y_value = value; }

    // Finds extrema of a cubic Bezier curve (Y component only)
    // Updates min_y and max_y if extrema are found within t in [0,1]
    static void FindBezierExtremaY(float p0, float p1, float p2, float p3, float& min_y, float& max_y)
    {
        // Derivative of cubic Bezier: B'(t) = 3(1-t)^2*(p1-p0) + 6(1-t)t(p2-p1) + 3t^2*(p3-p2)
        // This simplifies to a quadratic: at^2 + bt + c = 0
        // where:
        const float a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
        const float b = 2.0f * p0 - 4.0f * p1 + 2.0f * p2;
        const float c = -p0 + p1;

        // Solve quadratic equation
        std::array<float, 2> roots;
        int root_count = 0;

        if (std::abs(a) < 1e-6f)
        {
            // Linear case: bt + c = 0
            if (std::abs(b) > 1e-6f)
            {
                roots.at(root_count++) = -c / b;
            }
        }
        else
        {
            // Quadratic case
            const float discriminant = b * b - 4.0f * a * c;
            if (discriminant >= 0.0f)
            {
                const float sqrt_d = std::sqrt(discriminant);
                const float inv_2a = 1.0f / (2.0f * a);
                roots.at(root_count++) = (-b + sqrt_d) * inv_2a;
                roots.at(root_count++) = (-b - sqrt_d) * inv_2a;
            }
        }

        // Evaluate Bezier at valid roots
        for (int i = 0; i < root_count; i++)
        {
            const float t = roots.at(i);
            if (t > 0.0f && t < 1.0f)
            {
                // Evaluate cubic Bezier at t
                const float one_minus_t = 1.0f - t;
                const float y = one_minus_t * one_minus_t * one_minus_t * p0 + 3.0f * one_minus_t * one_minus_t * t * p1 +
                                3.0f * one_minus_t * t * t * p2 + t * t * t * p3;

                min_y = std::min(min_y, y);
                max_y = std::max(max_y, y);
            }
        }
    }

    void Fit()
    {
        m_draw_min.x = 0;

        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();

        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            const int keyframe_count = GetKeyframeCount(curve);

            for (int k = 0; k < keyframe_count; k++)
            {
                const Keyframe& kf = curve.m_keyframes.at(k);

                // Always include keyframe value
                max_y = std::max(max_y, kf.Value());
                min_y = std::min(min_y, kf.Value());

                // For segments between keyframes, find Bezier extrema
                if (k < keyframe_count - 1)
                {
                    const Keyframe& kf_next = curve.m_keyframes.at(k + 1);

                    // Get Bezier control points (Y values only)
                    const float p0 = kf.Value();
                    const float p1 = kf.Value() + kf.m_out.m_offset.y;
                    const float p2 = kf_next.Value() + kf_next.m_in.m_offset.y;
                    const float p3 = kf_next.Value();

                    // Find extrema of the cubic Bezier in Y
                    FindBezierExtremaY(p0, p1, p2, p3, min_y, max_y);
                }
            }
        }

        // Handle empty or single-value case
        if (min_y > max_y)
        {
            min_y = -1.0f;
            max_y = 1.0f;
        }

        const float range = max_y - min_y;
        const float padding = (range > 0.1f) ? range * 0.15f : 0.2f;
        m_draw_min.y = min_y - padding;
        m_draw_max.y = max_y + padding;
    }

private:
    void ClampFirstKeyframesToFirstFrame()
    {
        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            const int keyframe_count = GetKeyframeCount(curve);

            if (keyframe_count > 0)
            {
                curve.m_keyframes.at(0).m_pos.x = static_cast<float>(m_first_frame);
                ResolveCurveHandles(curve);
            }
        }
    }

    void ClampLastKeyframesToLastFrame()
    {
        const int curve_count = GetCurveCount();
        for (int c = 0; c < curve_count; c++)
        {
            Curve& curve = m_curves.at(c);
            const int keyframe_count = GetKeyframeCount(curve);
            if (keyframe_count > 0)
            {
                curve.m_keyframes.at(keyframe_count - 1).m_pos.x = static_cast<float>(m_last_frame);
                ResolveCurveHandles(curve);
            }
        }
    }
};

}  // namespace tanim
