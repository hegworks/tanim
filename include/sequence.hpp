// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "entity_data.hpp"
#include "tanim/include/enums.hpp"
#include "tanim/include/sequencer.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace tanim
{

struct Sequence : public sequencer::SequenceInterface
{
    enum class TypeMeta : uint8_t
    {
        NONE = 0,
        INT,
        BOOL,
    };

    struct Point : ImVec2
    {
        Point(float px = 0, float py = 0) : ImVec2{px, py} {}
        Point(ImVec2 im_vec2) : ImVec2(im_vec2) {}
        int Frame() const { return (int)x; }
        float Value() const { return y; }
        Point& operator=(const ImVec2& im_vec2)
        {
            x = im_vec2.x;
            y = im_vec2.y;
            return *this;
        }
    };

    struct Curve
    {
        std::vector<Point> m_points{{0, 0}, {10, 0}};
        sequencer::LerpType m_lerp_type{sequencer::LerpType::BEZIER};
        bool m_visibility{true};
        std::string m_name{"new_curve"};
    };

    TypeMeta m_type_meta{TypeMeta::NONE};
    RepresentationMeta m_representation_meta{RepresentationMeta::NONE};
    std::vector<Curve> m_curves{};
    ImVec2 m_draw_min{0, -1.5f};
    ImVec2 m_draw_max{500, 1.5f};
    int m_first_frame{0};
    int m_last_frame{10};
    bool m_expanded{false};
    SequenceId m_seq_id{};
    bool m_recording{false};
    int m_recording_frame{-1};
    float m_snap_y_value = 0.1f;

    Curve& AddCurve() { return m_curves.emplace_back(); }

    bool IsBezierCurve(int curve_index) const { return m_curves.at(curve_index).m_lerp_type == sequencer::LerpType::BEZIER; }

    bool IsKeyframeInAllCurves(int frame_num) const
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (!GetPointIdx(curve_idx, frame_num).has_value())
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
            if (GetPointIdx(curve_idx, frame_num).has_value())
            {
                return true;
            }
        }
        return false;
    }

    void AddNewKeyframe(int frame_num)
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (!GetPointIdx(curve_idx, frame_num).has_value())
            {
                AddPoint(curve_idx, {(float)frame_num, 0.0f});
            }
        }
    }

    void DeleteKeyframe(int frame_num)
    {
        for (int curve_idx = 0; curve_idx < GetCurveCount(); ++curve_idx)
        {
            if (auto opt_pt_idx = GetPointIdx(curve_idx, frame_num); opt_pt_idx.has_value())
            {
                RemovePoint(curve_idx, opt_pt_idx.value());
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

    int GetCurveCount() const override { return (int)m_curves.size(); }

    bool GetCurveVisibility(int curve_idx) override { return m_curves.at(curve_idx).m_visibility; }

    void SetCurveVisibility(int curve_idx, bool visibility) override { m_curves.at(curve_idx).m_visibility = visibility; }

    /// @return keyframe count
    int GetCurvePointCount(int curve_index) override
    {
        if (IsBezierCurve(curve_index)) return (int)m_curves.at(curve_index).m_points.size() / 3;
        return (int)m_curves.at(curve_index).m_points.size();
    }

    /// @return keyframe + handles(for bezier) count
    int GetCurveRawPointCount(int curve_index) const { return (int)m_curves.at(curve_index).m_points.size(); }

    /// @return keyframe index from raw point index (for Bezier)
    int GetKeyframeIndex(int curve_index, int raw_point_index) const
    {
        if (IsBezierCurve(curve_index)) return raw_point_index / 3;
        return raw_point_index;
    }

    /// @return Get raw index of keyframe (for Bezier)
    int GetKeyframeRawIndex(int curve_index, int keyframe_index) const
    {
        if (IsBezierCurve(curve_index)) return keyframe_index * 3 + 1;
        return keyframe_index;
    }

    static float ConstrainHandleX(float handleX, float keyframeX, float adjacentKeyX, bool isOutHandle)
    {
        if (isOutHandle)
            return ImClamp(handleX, keyframeX, adjacentKeyX);
        else
            return ImClamp(handleX, adjacentKeyX, keyframeX);
    }

    // Generate default handles (flat tangents, 1/3 segment width) for a keyframe
    void GenerateDefaultHandles(int curve_index,
                                int keyframe_index,
                                ImVec2 keyframePos,
                                Point& outInHandle,
                                Point& outOutHandle) const
    {  // REF: by claude.ai
        const auto& pts = m_curves.at(curve_index).m_points;
        int keyframeCount = (int)pts.size() / 3;
        const float handleFraction = 1.0f / 3.0f;

        // In-handle
        if (keyframe_index > 0)
        {
            const Point& prevKey = pts.at((keyframe_index - 1) * 3 + 1);
            float segmentWidth = keyframePos.x - prevKey.x;
            outInHandle = Point(keyframePos.x - segmentWidth * handleFraction, keyframePos.y);
        }
        else
        {
            outInHandle = keyframePos;
        }

        // Out-handle
        if (keyframe_index < keyframeCount - 1)
        {
            const Point& nextKey = pts.at((keyframe_index + 1) * 3 + 1);
            float segmentWidth = nextKey.x - keyframePos.x;
            outOutHandle = Point(keyframePos.x + segmentWidth * handleFraction, keyframePos.y);
        }
        else
        {
            outOutHandle = keyframePos;
        }
    }

    void ResetTangentsForKeyframe(int curve_index, int keyframe_index) override
    {
        if (!IsBezierCurve(curve_index)) return;

        auto& pts = m_curves.at(curve_index).m_points;
        ImVec2 keyframePos = pts.at(keyframe_index * 3 + 1);

        Point newInHandle, newOutHandle;
        GenerateDefaultHandles(curve_index, keyframe_index, keyframePos, newInHandle, newOutHandle);

        pts.at(keyframe_index * 3) = newInHandle;
        pts.at(keyframe_index * 3 + 2) = newOutHandle;
    }

    void ConstrainHandlesForKeyframe(int curve_index, int keyframe_index)
    {  // REF: by claude.ai
        auto& pts = m_curves.at(curve_index).m_points;
        int keyframeCount = (int)pts.size() / 3;

        int inIdx = keyframe_index * 3;
        int keyIdx = keyframe_index * 3 + 1;
        int outIdx = keyframe_index * 3 + 2;

        Point& inHandle = pts.at(inIdx);
        Point& keyframe = pts.at(keyIdx);
        Point& outHandle = pts.at(outIdx);

        if (keyframe_index > 0)
        {
            float prevKeyX = pts.at((keyframe_index - 1) * 3 + 1).x;
            inHandle.x = ConstrainHandleX(inHandle.x, keyframe.x, prevKeyX, false);
        }
        else
        {
            inHandle = keyframe;
        }

        if (keyframe_index < keyframeCount - 1)
        {
            float nextKeyX = pts.at((keyframe_index + 1) * 3 + 1).x;
            outHandle.x = ConstrainHandleX(outHandle.x, keyframe.x, nextKeyX, true);
        }
        else
        {
            outHandle = keyframe;
        }
    }

    uint32_t GetCurveColor(int curve_index) override
    {
        // TODO(tanim) replace hardcoded colors
        constexpr uint32_t colors[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFF00, 0xFFFF00FF};
        return colors[curve_index];
    }

    const std::vector<ImVec2>& GetCurvePointsList(int curve_index) override
    {
        return reinterpret_cast<const std::vector<ImVec2>&>(m_curves.at(curve_index).m_points);
    }

    sequencer::LerpType GetCurveLerpType(int curve_idx) override { return m_curves.at(curve_idx).m_lerp_type; }

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

    int EditPoint(int curve_index, int point_index, ImVec2 value) override
    {
        auto& pts = m_curves.at(curve_index).m_points;
        if (IsBezierCurve(curve_index))
        {
            int pointType = point_index % 3;  // 0=in-handle, 1=keyframe, 2=out-handle
            int keyframeIdx = point_index / 3;
            int keyframeCount = GetCurvePointCount(curve_index);

            if (pointType == 1)  // === EDITING A KEYFRAME ===
            {
                // Snap X to integer
                value.x = floorf(value.x);

                // Prevent duplicate frames
                if (keyframeIdx + 1 < keyframeCount)
                {
                    if ((int)pts.at((keyframeIdx + 1) * 3 + 1).x == (int)value.x) return point_index;
                }
                if (keyframeIdx > 0)
                {
                    if ((int)pts.at((keyframeIdx - 1) * 3 + 1).x == (int)value.x) return point_index;
                }

                // Y-axis snapping
                if (ImGui::GetIO().KeyCtrl && m_snap_y_value > 0.0f)
                    value.y = std::round(value.y / m_snap_y_value) * m_snap_y_value;

                // TypeMeta constraints
                if (m_type_meta == TypeMeta::INT)
                    value.y = std::floorf(value.y);
                else if (m_type_meta == TypeMeta::BOOL)
                    value.y = value.y > 0.5f ? 1.0f : 0.0f;

                // RepresentationMeta constraints
                if (m_representation_meta == RepresentationMeta::COLOR) value.y = ImClamp(value.y, 0.0f, 1.0f);

                // Calculate delta from old position
                Point oldKeyframe = pts.at(point_index);
                ImVec2 delta(value.x - oldKeyframe.x, value.y - oldKeyframe.y);

                // Update keyframe
                pts.at(point_index) = value;

                // Move handles with keyframe
                int inIdx = keyframeIdx * 3;
                int outIdx = keyframeIdx * 3 + 2;
                pts.at(inIdx).x += delta.x;
                pts.at(inIdx).y += delta.y;
                pts.at(outIdx).x += delta.x;
                pts.at(outIdx).y += delta.y;

                // Clamp first keyframe to first frame
                if (keyframeIdx == 0)
                {
                    float targetX = static_cast<float>(m_first_frame);
                    float xDiff = targetX - pts.at(point_index).x;
                    pts.at(point_index).x = targetX;
                    pts.at(inIdx).x += xDiff;
                    pts.at(outIdx).x += xDiff;
                }

                // Clamp last keyframe to last frame
                if (keyframeIdx == keyframeCount - 1)
                {
                    float targetX = static_cast<float>(m_last_frame);
                    float xDiff = targetX - pts.at(point_index).x;
                    pts.at(point_index).x = targetX;
                    pts.at(inIdx).x += xDiff;
                    pts.at(outIdx).x += xDiff;
                }

                // Constrain handles to valid ranges
                ConstrainHandlesForKeyframe(curve_index, keyframeIdx);

                // Also constrain neighbor handles that might be affected
                if (keyframeIdx > 0) ConstrainHandlesForKeyframe(curve_index, keyframeIdx - 1);
                if (keyframeIdx < keyframeCount - 1) ConstrainHandlesForKeyframe(curve_index, keyframeIdx + 1);

                return point_index;
            }
            else  // === EDITING A HANDLE ===
            {
                Point& keyframe = pts.at(keyframeIdx * 3 + 1);

                if (pointType == 0)  // In-handle
                {
                    if (keyframeIdx > 0)
                    {
                        float prevKeyX = pts.at((keyframeIdx - 1) * 3 + 1).x;
                        value.x = ConstrainHandleX(value.x, keyframe.x, prevKeyX, false);
                    }
                    else
                    {
                        value = keyframe;
                    }
                }
                else  // Out-handle
                {
                    if (keyframeIdx < keyframeCount - 1)
                    {
                        float nextKeyX = pts.at((keyframeIdx + 1) * 3 + 1).x;
                        value.x = ConstrainHandleX(value.x, keyframe.x, nextKeyX, true);
                    }
                    else
                    {
                        value = keyframe;
                    }
                }

                pts.at(point_index) = value;
                return point_index;
            }
        }
        else  // === NON-BEZIER CURVES ===
        {
            value.x = floorf(value.x);

            if (point_index + 1 < (int)GetCurvePointCount(curve_index) && (int)pts.at(point_index + 1).x == (int)value.x)
                return point_index;

            if (point_index - 1 > 0 && (int)pts.at(point_index - 1).x == (int)value.x) return point_index;

            if (ImGui::GetIO().KeyCtrl && m_snap_y_value > 0.0f)
                value.y = std::round(value.y / m_snap_y_value) * m_snap_y_value;

            if (m_type_meta == TypeMeta::INT)
                value.y = std::floorf(value.y);
            else if (m_type_meta == TypeMeta::BOOL)
                value.y = value.y > 0.5f ? 1.0f : 0.0f;

            if (m_representation_meta == RepresentationMeta::COLOR) value.y = ImClamp(value.y, 0.0f, 1.0f);

            pts.at(point_index) = ImVec2(value.x, value.y);
            SortCurvePoints(curve_index);

            pts.at(0).x = static_cast<float>(m_first_frame);
            pts.at(GetCurvePointCount(curve_index) - 1).x = static_cast<float>(m_last_frame);

            for (int i = 0; i < GetCurvePointCount(curve_index); i++)
            {
                if (m_curves.at(curve_index).m_points.at(i).Frame() == (int)value.x) return i;
            }
            return point_index;
        }
    }

    void AddPoint(int curve_index, ImVec2 value) override
    {
        auto& pts = m_curves.at(curve_index).m_points;

        if (IsBezierCurve(curve_index))
        {  // REF: by claude.ai
            value.x = floorf(value.x);
            int keyframeCount = GetCurvePointCount(curve_index);

            // Find insertion position & check for duplicates
            int insertKeyIdx = keyframeCount;
            for (int k = 0; k < keyframeCount; k++)
            {
                float keyX = pts.at(k * 3 + 1).x;
                if ((int)value.x == (int)keyX) return;  // Duplicate
                if (value.x < keyX)
                {
                    insertKeyIdx = k;
                    break;
                }
            }

            // Generate handles based on position AFTER insertion
            // We need to calculate what the neighbors will be
            Point inHandle, outHandle;
            const float handleFraction = 1.0f / 3.0f;

            // In-handle
            if (insertKeyIdx > 0)
            {
                float prevKeyX = pts.at((insertKeyIdx - 1) * 3 + 1).x;
                float segmentWidth = value.x - prevKeyX;
                inHandle = Point(value.x - segmentWidth * handleFraction, value.y);
            }
            else
            {
                inHandle = value;
            }

            // Out-handle
            if (insertKeyIdx < keyframeCount)
            {
                float nextKeyX = pts.at(insertKeyIdx * 3 + 1).x;
                float segmentWidth = nextKeyX - value.x;
                outHandle = Point(value.x + segmentWidth * handleFraction, value.y);
            }
            else if (keyframeCount > 0)
            {
                float prevKeyX = pts.at((keyframeCount - 1) * 3 + 1).x;
                float segmentWidth = value.x - prevKeyX;
                outHandle = Point(value.x + segmentWidth * handleFraction, value.y);
            }
            else
            {
                outHandle = value;
            }

            // Insert the 3 points
            int insertRawIdx = insertKeyIdx * 3;
            pts.insert(pts.begin() + insertRawIdx, Point(inHandle));
            pts.insert(pts.begin() + insertRawIdx + 1, Point(value));
            pts.insert(pts.begin() + insertRawIdx + 2, Point(outHandle));

            // Constrain neighbor handles
            int newKeyframeCount = keyframeCount + 1;
            if (insertKeyIdx > 0) ConstrainHandlesForKeyframe(curve_index, insertKeyIdx - 1);
            if (insertKeyIdx < newKeyframeCount - 1) ConstrainHandlesForKeyframe(curve_index, insertKeyIdx + 1);
        }
        else
        {
            pts.emplace_back(value);
            SortCurvePoints(curve_index);
        }
    }

    void RemovePoint(int curve_index, int point_index) override
    {
        auto& pts = m_curves.at(curve_index).m_points;

        if (IsBezierCurve(curve_index))
        {  // REF: by claude.ai
            int keyframeCount = GetCurvePointCount(curve_index);

            if (point_index <= 0 || point_index >= keyframeCount - 1) return;

            int rawIdx = point_index * 3;
            pts.erase(pts.begin() + rawIdx, pts.begin() + rawIdx + 3);
        }
        else
        {
            if (point_index > 0 && point_index < GetCurvePointCount(curve_index) - 1) pts.erase(pts.begin() + point_index);
        }
    }

    std::optional<int> GetPointIdx(int curve_idx, int frame_num) const
    {
        if (curve_idx < 0 || curve_idx >= GetCurveCount())
        {
            return std::nullopt;
        }

        const Curve& curve = m_curves.at(curve_idx);

        if (IsBezierCurve(curve_idx))
        {  // REF: by claude.ai
            int keyframeCount = (int)curve.m_points.size() / 3;
            for (int k = 0; k < keyframeCount; k++)
            {
                if (curve.m_points.at(k * 3 + 1).Frame() == frame_num) return k;
            }
        }
        else
        {
            for (int i = 0; i < (int)curve.m_points.size(); ++i)
            {
                if (curve.m_points.at(i).Frame() == frame_num) return i;
            }
        }

        return std::nullopt;
    }

    ImVec2 GetDrawMax() override { return m_draw_max; }

    ImVec2 GetDrawMin() override { return m_draw_min; }

    void SetDrawMin(ImVec2 min) override
    {
        m_draw_min = min;
        m_draw_min.x = 0;
    }

    void SetDrawMax(ImVec2 max) override { m_draw_max = max; }

    void BeginEdit(int) override { /*TODO(tanim)*/ }
    void EndEdit() override { /*TODO(tanim)*/ }

    // TODO(tanim) replace hardcoded value (maybe?)
    unsigned int GetBackgroundColor() override { return 0x00000000; }

    void EditLastFrame(int new_last_frame)
    {
        for (int c = 0; c < (int)m_curves.size(); c++)
        {
            auto& curve = m_curves[c];

            if (IsBezierCurve(c))
            {  // REF: by claude.ai
                int keyframeCount = (int)curve.m_points.size() / 3;
                for (int k = 0; k < keyframeCount; k++)
                {
                    int inIdx = k * 3;
                    int keyIdx = k * 3 + 1;
                    int outIdx = k * 3 + 2;

                    float oldX = curve.m_points[keyIdx].x;
                    float newX = helpers::Remap((float)m_first_frame,
                                                (float)m_last_frame,
                                                (float)m_first_frame,
                                                (float)new_last_frame,
                                                oldX);

                    float xDiff = newX - oldX;
                    curve.m_points[keyIdx].x = newX;
                    curve.m_points[inIdx].x += xDiff;
                    curve.m_points[outIdx].x += xDiff;
                }

                for (int k = 0; k < keyframeCount; k++) ConstrainHandlesForKeyframe(c, k);
            }
            else
            {
                for (auto& point : curve.m_points)
                {
                    point.x = helpers::Remap((float)m_first_frame,
                                             (float)m_last_frame,
                                             (float)m_first_frame,
                                             (float)new_last_frame,
                                             point.x);
                }
            }
        }

        m_last_frame = new_last_frame;
        ClampLastPointsToLastFrame();
    }

    void EditFirstFrame(int new_first_frame)
    {
        for (int c = 0; c < (int)m_curves.size(); c++)
        {
            auto& curve = m_curves[c];

            if (IsBezierCurve(c))
            {  // REF: by claude.ai
                int keyframeCount = (int)curve.m_points.size() / 3;
                for (int k = 0; k < keyframeCount; k++)
                {
                    int inIdx = k * 3;
                    int keyIdx = k * 3 + 1;
                    int outIdx = k * 3 + 2;

                    float oldX = curve.m_points[keyIdx].x;
                    float newX = helpers::Remap((float)m_first_frame,
                                                (float)m_last_frame,
                                                (float)new_first_frame,
                                                (float)m_last_frame,
                                                oldX);

                    float xDiff = newX - oldX;
                    curve.m_points[keyIdx].x = newX;
                    curve.m_points[inIdx].x += xDiff;
                    curve.m_points[outIdx].x += xDiff;
                }

                for (int k = 0; k < keyframeCount; k++) ConstrainHandlesForKeyframe(c, k);
            }
            else
            {
                for (auto& point : curve.m_points)
                {
                    point.x = helpers::Remap((float)m_first_frame,
                                             (float)m_last_frame,
                                             (float)new_first_frame,
                                             (float)m_last_frame,
                                             point.x);
                }
            }
        }

        m_first_frame = new_first_frame;
        ClampFirstPointsToFirstFrame();
    }

    void MoveFrames(int frame_diff)
    {
        m_last_frame += frame_diff;
        m_first_frame += frame_diff;
        for (auto& curve : m_curves)
        {
            for (auto& point : curve.m_points)
            {
                point.x += frame_diff;
            }
        }
    }

    void EditSnapY(float value) { m_snap_y_value = value; }

    void Fit() override
    {
        m_draw_min.x = 0;

        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();

        for (int c = 0; c < (int)m_curves.size(); c++)
        {
            const auto& curve = m_curves[c];

            if (IsBezierCurve(c))
            {  // REF: by claude.ai
                int keyframeCount = (int)curve.m_points.size() / 3;
                for (int k = 0; k < keyframeCount; k++)
                {
                    float y = curve.m_points.at(k * 3 + 1).y;
                    max_y = std::max(max_y, y);
                    min_y = std::min(min_y, y);
                }
            }
            else
            {
                for (const auto& point : curve.m_points)
                {
                    max_y = std::max(max_y, point.y);
                    min_y = std::min(min_y, point.y);
                }
            }
        }

        const float range = max_y - min_y;
        const float padding = (range > 0.1f) ? range * 0.15f : 0.2f;
        m_draw_min.y = min_y - padding;
        m_draw_max.y = max_y + padding;
    }

private:
    void SortCurvePoints(int curve_index)
    {
        const auto begin = m_curves.at(curve_index).m_points.begin();
        const auto end = m_curves.at(curve_index).m_points.end();
        std::sort(begin, end, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
    }

    void ClampLastPointsToLastFrame()
    {
        for (int c = 0; c < GetCurveCount(); c++)
        {
            auto& curve = m_curves[c];

            if (IsBezierCurve(c))
            {
                int keyframeCount = (int)curve.m_points.size() / 3;
                if (keyframeCount > 0)
                {
                    float xDiff = (float)m_first_frame - curve.m_points[1].x;
                    curve.m_points[0] = Point((float)m_first_frame, curve.m_points[1].y);
                    curve.m_points[1].x = (float)m_first_frame;
                    curve.m_points[2].x += xDiff;

                    ConstrainHandlesForKeyframe(c, 0);
                }
            }
            else
            {
                if (!curve.m_points.empty()) curve.m_points[0].x = (float)m_first_frame;
                SortCurvePoints(c);
            }
        }
    }

    void ClampFirstPointsToFirstFrame()
    {
        for (int i = 0; i < GetCurveCount(); i++)
        {
            m_curves.at(i).m_points.at(0).x = (float)m_first_frame;
            SortCurvePoints(i);
        }
    }
};

}  // namespace tanim
