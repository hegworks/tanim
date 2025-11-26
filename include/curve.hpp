// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/curve_editor.hpp"

#include <algorithm>
#include <iterator>

namespace tanim
{

struct Sequence : public curve_editor::Delegate
{
    // TODO(tanim) replace raw array with vector
    // TODO(tanim) replace SoA with AoS
    ImVec2 m_curves_points[3][8];
    size_t m_curve_point_count[3];
    bool m_curve_visibility[3];
    ImVec2 m_min_point_value{0, -1.5f};
    ImVec2 m_max_point_value{500, 1.5f};
    int m_timeline_last_frame{1};

    Sequence()
    {
        m_curves_points[0][0] = ImVec2(0.f, 0.0f);
        m_curves_points[0][1] = ImVec2(10.f, 0.0f);
        m_curve_point_count[0] = 2;

        m_curves_points[1][0] = ImVec2(0.f, 0.0f);
        m_curves_points[1][1] = ImVec2(10.f, 0.0f);
        m_curve_point_count[1] = 2;

        m_curves_points[2][0] = ImVec2(0.f, 0.0f);
        m_curves_points[2][1] = ImVec2(10.f, 0.0f);
        m_curve_point_count[2] = 2;
        m_curve_visibility[0] = m_curve_visibility[1] = m_curve_visibility[2] = true;
    }

    // TODO(tanim) replace hardcoded value
    size_t GetCurveCount() override { return 3; }

    bool IsCurveVisible(size_t curve_index) override { return m_curve_visibility[curve_index]; }

    size_t GetCurvePointCount(size_t curve_index) override { return m_curve_point_count[curve_index]; }

    uint32_t GetCurveColor(size_t curve_index) override
    {
        // TODO(tanim) replace hardcoded colors
        constexpr uint32_t colors[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
        return colors[curve_index];
    }

    ImVec2* GetCurvePointsList(size_t curve_index) override { return m_curves_points[curve_index]; }

    curve_editor::LerpType GetCurveLerpType(size_t curve_index) const override
    {
        // TODO(tanim) replace hardcoded return value
        IM_UNUSED(curve_index);
        return curve_editor::LerpType::SMOOTH;
    }

    int EditPoint(size_t curve_index, int point_index, ImVec2 value) override
    {
        // TanimAddition
        // snap the time (x) of keyframes to integers
        value.x = floorf(value.x);

        // TanimAddition
        // return early if the keyframes are in the same frame
        if (point_index + 1 < (int)GetCurvePointCount(curve_index) &&
            (int)m_curves_points[curve_index][point_index + 1].x == (int)value.x)
        {
            return point_index;
        }
        if (point_index - 1 > 0 && (int)m_curves_points[curve_index][point_index - 1].x == (int)value.x)
        {
            return point_index;
        }

        m_curves_points[curve_index][point_index] = ImVec2(value.x, value.y);
        SortCurvePoints(curve_index);

        // TanimAddition
        // force the first keyframe time (x) to 0
        m_curves_points[curve_index][0].x = 0;
        // force the last keyframe time (x) to the sequence's last frame
        m_curves_points[curve_index][GetCurvePointCount(curve_index) - 1].x = (float)m_timeline_last_frame;

        for (size_t i = 0; i < GetCurvePointCount(curve_index); i++)
        {
            if (m_curves_points[curve_index][i].x == value.x) return (int)i;
        }
        return point_index;
    }

    void AddPoint(size_t curve_index, ImVec2 value) override
    {
        // TODO(tanim) remove hardcoded restriction of 8 points on a curve
        if (m_curve_point_count[curve_index] >= 8) return;
        m_curves_points[curve_index][m_curve_point_count[curve_index]++] = value;
        SortCurvePoints(curve_index);
    }

    ImVec2& GetMaxPointValue() override { return m_max_point_value; }

    ImVec2& GetMinPointValue() override { return m_min_point_value; }

    void SetMinPointValue(ImVec2 min) override { m_min_point_value = min; }

    void SetMaxPointValue(ImVec2 max) override { m_max_point_value = max; }

    void BeginEdit(int) override { /*TODO(tanim)*/ }
    void EndEdit() override { /*TODO(tanim)*/ }

    // TODO(tanim) replace hardcoded value (maybe?)
    unsigned int GetBackgroundColor() override { return 0x00000000; }

    void TimelineLastFrameEdit(int new_last_frame)
    {
        m_timeline_last_frame = new_last_frame;
        ClampLastPointsToTimelineLastFrame();
    }

private:
    void SortCurvePoints(size_t curve_index)
    {
        const auto first = std::begin(m_curves_points[curve_index]);
        const auto last = std::begin(m_curves_points[curve_index]) + GetCurvePointCount(curve_index);
        std::sort(first, last, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
    }

    void ClampLastPointsToTimelineLastFrame()
    {
        for (size_t i = 0; i < GetCurveCount(); i++)
        {
            m_curves_points[i][GetCurvePointCount(i) - 1].x = (float)m_timeline_last_frame;
            SortCurvePoints(i);
        }
    }
};

}  // namespace tanim
