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
        sequencer::LerpType m_lerp_type{sequencer::LerpType::LINEAR};
        bool m_visibility{true};
        std::string m_name{"new_curve"};
    };

    TypeMeta m_type_meta{TypeMeta::NONE};
    RepresentationMeta m_representation_meta{RepresentationMeta::NONE};
    std::vector<Curve> m_curves{};
    ImVec2 m_draw_min{0, -1.5f};
    ImVec2 m_draw_max{500, 1.5f};
    bool m_expanded{false};
    SequenceId m_seq_id{};
    bool m_recording{false};
    int m_recording_frame{-1};

    Curve& AddCurve() { return m_curves.emplace_back(); }

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

    // TODO(tanim) this has to be removed after we use ECS maybe
    const int* m_timeline_last_frame{nullptr};

    float m_snap_y_value = 0.1f;

    // TODO(tanim) the int* has to be removed after we use ECS maybe
    Sequence(const int* timeline_last_frame) { m_timeline_last_frame = timeline_last_frame; }

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

    int GetCurvePointCount(int curve_index) override { return (int)m_curves.at(curve_index).m_points.size(); }

    uint32_t GetCurveColor(int curve_index) override
    {
        // TODO(tanim) replace hardcoded colors
        constexpr uint32_t colors[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
        return colors[curve_index];
    }

    const std::vector<ImVec2>& GetCurvePointsList(int curve_index) override
    {
        return reinterpret_cast<const std::vector<ImVec2>&>(m_curves.at(curve_index).m_points);
    }

    sequencer::LerpType GetCurveLerpType(int curve_idx) override { return m_curves.at(curve_idx).m_lerp_type; }

    bool IsRecording() const { return m_recording; }

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
        // TanimAddition
        // snap the time (x) of keyframes to integers
        value.x = floorf(value.x);

        // TanimAddition
        // return early if the keyframes are in the same frame
        if (point_index + 1 < (int)GetCurvePointCount(curve_index) &&
            (int)m_curves.at(curve_index).m_points.at(point_index + 1).x == (int)value.x)
        {
            return point_index;
        }
        if (point_index - 1 > 0 && (int)m_curves.at(curve_index).m_points.at(point_index - 1).x == (int)value.x)
        {
            return point_index;
        }

        if (ImGui::GetIO().KeyCtrl)
        {
            if (m_snap_y_value > 0.0f)
            {
                value.y = std::round(value.y / m_snap_y_value) * m_snap_y_value;
            }
        }

        if (m_type_meta == TypeMeta::INT)
        {
            value.y = std::floorf(value.y);
        }
        else if (m_type_meta == TypeMeta::BOOL)
        {
            value.y = value.y > 0.5f ? 1.0f : 0.0f;
        }

        if (m_representation_meta == RepresentationMeta::COLOR)
        {
            value.y = ImClamp(value.y, 0.0f, 1.0f);
        }

        m_curves.at(curve_index).m_points.at(point_index) = ImVec2(value.x, value.y);
        SortCurvePoints(curve_index);

        // TanimAddition
        // force the first keyframe time (x) to 0
        m_curves.at(curve_index).m_points.at(0).x = 0;
        // force the last keyframe time (x) to the sequence's last frame
        m_curves.at(curve_index).m_points.at(GetCurvePointCount(curve_index) - 1).x = (float)*m_timeline_last_frame;

        for (int i = 0; i < GetCurvePointCount(curve_index); i++)
        {
            if (m_curves.at(curve_index).m_points.at(i).Frame() == (int)value.x) return i;
        }
        return point_index;
    }

    void AddPoint(int curve_index, ImVec2 value) override
    {
        m_curves.at(curve_index).m_points.emplace_back(value);
        SortCurvePoints(curve_index);
    }

    void RemovePoint(int curve_index, int point_index) override
    {
        if (point_index > 0 && point_index < GetCurvePointCount(curve_index) - 1)
        {
            std::vector<Point>* points = &m_curves.at(curve_index).m_points;
            points->erase(points->begin() + point_index);
        }
    }

    std::optional<int> GetPointIdx(int curve_idx, int frame_num) const
    {
        if (curve_idx < 0 || curve_idx >= GetCurveCount())
        {
            return std::nullopt;
        }

        const Curve& curve = m_curves.at(curve_idx);
        for (int point_idx = 0; point_idx < (int)curve.m_points.size(); ++point_idx)
        {
            if (curve.m_points.at(point_idx).Frame() == frame_num)
            {
                return point_idx;
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

    void EditTimelineLastFrame() { ClampLastPointsToTimelineLastFrame(); }

    void EditSnapY(float value) { m_snap_y_value = value; }

    void Fit() override
    {
        m_draw_min.x = 0;

        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::min();

        for (const auto& curve : m_curves)
        {
            for (const auto& point : curve.m_points)
            {
                if (point.y > max_y)
                {
                    max_y = point.y;
                }
                if (point.y < min_y)
                {
                    min_y = point.y;
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

    void ClampLastPointsToTimelineLastFrame()
    {
        for (int i = 0; i < GetCurveCount(); i++)
        {
            m_curves.at(i).m_points.at(GetCurvePointCount(i) - 1).x = (float)*m_timeline_last_frame;
            SortCurvePoints(i);
        }
    }
};

}  // namespace tanim
