// REF: originally based on the imguizmo's example main.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/example/main.cpp

#pragma once
#include "tanim/include/sequencer.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

namespace tanim
{

struct Sequence : public sequencer::SequenceInterface
{
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
        sequencer::LerpType m_lerp_type{sequencer::LerpType::SMOOTH};
        bool m_visibility{true};
    };

    std::vector<Curve> m_curves{};
    ImVec2 m_draw_min{0, -1.5f};
    ImVec2 m_draw_max{500, 1.5f};
    int m_timeline_last_frame{1};

    Sequence()
    {
        // TODO(tanim) replace hardcoded curves with curves based on the reflected parameter
        m_curves.emplace_back();
        m_curves.emplace_back();
        m_curves.emplace_back();
    }

    int CurveCount() override { return (int)m_curves.size(); }

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

        m_curves.at(curve_index).m_points.at(point_index) = ImVec2(value.x, value.y);
        SortCurvePoints(curve_index);

        // TanimAddition
        // force the first keyframe time (x) to 0
        m_curves.at(curve_index).m_points.at(0).x = 0;
        // force the last keyframe time (x) to the sequence's last frame
        m_curves.at(curve_index).m_points.at(GetCurvePointCount(curve_index) - 1).x = (float)m_timeline_last_frame;

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

    void RemovePoint(int /*curve_index*/, int /*point_index*/) override { /*TODO(tanim)*/ }

    ImVec2 GetMaxPointValue() override { return m_draw_max; }

    ImVec2 GetMinPointValue() override { return m_draw_min; }

    void SetMinPointValue(ImVec2 min) override { m_draw_min = min; }

    void SetMaxPointValue(ImVec2 max) override { m_draw_max = max; }

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
    void SortCurvePoints(int curve_index)
    {
        const auto begin = m_curves.at(curve_index).m_points.begin();
        const auto end = m_curves.at(curve_index).m_points.end();
        std::sort(begin, end, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
    }

    void ClampLastPointsToTimelineLastFrame()
    {
        for (int i = 0; i < CurveCount(); i++)
        {
            m_curves.at(i).m_points.at(GetCurvePointCount(i) - 1).x = (float)m_timeline_last_frame;
            SortCurvePoints(i);
        }
    }
};

}  // namespace tanim
