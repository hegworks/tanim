// REF (tanim): originally based on the imguizmo's ImCurveEdit.cpp:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/ImCurveEdit.cpp

// https://github.com/CedricGuillemet/ImGuizmo
// v1.92.5 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2016-2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "tanim/include/sequencer.hpp"

#include "tanim/include/curve_functions.hpp"
#include "tanim/include/bezier.hpp"
#include "tanim/include/includes.hpp"
#include "tanim/include/sequence.hpp"

#include <cstdint>
#include <set>
#include <vector>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>
#endif
#if !defined(_MSC_VER) && !defined(__MINGW64_VERSION_MAJOR)
#define _malloca(x) alloca(x)
#define _freea(x)
#endif

namespace tanim::sequencer
{

static float Distance(float x, float y, float x1, float y1, float x2, float y2)
{
    const float a = x - x1;
    const float b = y - y1;
    const float c = x2 - x1;
    const float d = y2 - y1;

    const float dot = a * c + b * d;
    const float len_sq = c * c + d * d;
    float param = -1.0f;
    if (len_sq > FLT_EPSILON) param = dot / len_sq;

    float xx, yy;

    if (param < 0.0f)
    {
        xx = x1;
        yy = y1;
    }
    else if (param > 1.0f)
    {
        xx = x2;
        yy = y2;
    }
    else
    {
        xx = x1 + param * c;
        yy = y1 + param * d;
    }

    const float dx = x - xx;
    const float dy = y - yy;
    return sqrtf(dx * dx + dy * dy);
}

static HandleState DrawHandle(ImDrawList* draw_list,
                              const ImVec2& handle_screen_pos,
                              const ImVec2& keyframe_screen_pos,
                              bool is_in_handle,
                              bool is_selected)
{
    HandleState ret = HandleState::NONE;
    ImGuiIO& io = ImGui::GetIO();

    // Draw line from keyframe to handle
    draw_list->AddLine(keyframe_screen_pos, handle_screen_pos, 0xFF808080, 1.0f);

    // Handle colors
    ImU32 border_color = is_in_handle ? 0xFF0000FF : 0xFFFF0000;  // Red for in, Blue for out (BGR)
    ImU32 fill_color = 0x00000000;                                // Transparent by default

    const float radius = 4.0f;
    const ImRect handle_rect(handle_screen_pos - ImVec2(radius, radius), handle_screen_pos + ImVec2(radius, radius));

    bool hovered = handle_rect.Contains(io.MousePos);
    if (hovered)
    {
        ret = HandleState::HOVERED;
        fill_color = 0xFFFFFFFF;  // White fill on hover
        if (io.MouseDown[0]) ret = HandleState::CLICKED;
    }

    if (is_selected)
    {
        fill_color = 0xFFFFFFFF;
    }

    // Draw handle circle
    if (fill_color != 0x00000000) draw_list->AddCircleFilled(handle_screen_pos, radius, fill_color);
    draw_list->AddCircle(handle_screen_pos, radius, border_color, 0, 2.0f);

    return ret;
}

static int DrawKeyframe(ImDrawList* draw_list,
                        const ImVec2& pos,
                        const ImVec2& size,
                        const ImVec2& offset,
                        bool edited,
                        bool x_moveable,
                        bool y_moveable)
{
    int ret = 0;
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 center = pos * size + offset;

    // Shape sizes
    static constexpr float k_square_half_size = 4.5f;
    static constexpr float k_rect_long = 6.0f;
    static constexpr float k_rect_short = 3.0f;
    static constexpr float k_diamond_radius = 4.5f;
    static constexpr float k_anchor_half_size = 5.0f;

    // Outline thickness
    static constexpr float k_outline_thickness_edited = 3.0f;
    static constexpr float k_outline_thickness_normal = 2.0f;

    // Colors
    static constexpr ImU32 k_fill_color = 0xFF000000;
    static constexpr ImU32 k_outline_color_edited = 0xFFFFFFFF;
    static constexpr ImU32 k_outline_color_hovered = 0xFF80B0FF;
    static constexpr ImU32 k_outline_color_normal = 0xFF0080FF;

    ImVec2 half_size;
    bool draw_diamond = false;

    if (x_moveable && y_moveable)
    {
        half_size = ImVec2(k_square_half_size, k_square_half_size);
    }
    else if (x_moveable)
    {
        half_size = ImVec2(k_rect_long, k_rect_short);
    }
    else if (y_moveable)
    {
        half_size = ImVec2(k_rect_short, k_rect_long);
    }
    else
    {
        draw_diamond = true;
    }

    const ImRect anchor(center - ImVec2(k_anchor_half_size, k_anchor_half_size),
                        center + ImVec2(k_anchor_half_size, k_anchor_half_size));
    if (anchor.Contains(io.MousePos))
    {
        ret = 1;
        if (io.MouseDown[0]) ret = 2;
    }

    ImU32 outline_color;
    float outline_thickness;

    if (edited)
    {
        outline_color = k_outline_color_edited;
        outline_thickness = k_outline_thickness_edited;
    }
    else if (ret)
    {
        outline_color = k_outline_color_hovered;
        outline_thickness = k_outline_thickness_normal;
    }
    else
    {
        outline_color = k_outline_color_normal;
        outline_thickness = k_outline_thickness_normal;
    }

    if (draw_diamond)
    {
        static constexpr ImVec2 LOCAL_OFFSETS[4] = {ImVec2(1, 0), ImVec2(0, 1), ImVec2(-1, 0), ImVec2(0, -1)};
        ImVec2 offsets[4];
        for (int i = 0; i < 4; i++)
        {
            offsets[i] = center + LOCAL_OFFSETS[i] * k_diamond_radius;
        }
        draw_list->AddConvexPolyFilled(offsets, 4, k_fill_color);
        draw_list->AddPolyline(offsets, 4, outline_color, true, outline_thickness);
    }
    else
    {
        ImVec2 rect_min = center - half_size;
        ImVec2 rect_max = center + half_size;
        draw_list->AddRectFilled(rect_min, rect_max, k_fill_color);
        draw_list->AddRect(rect_min, rect_max, outline_color, 0.0f, 0, outline_thickness);
    }

    return ret;
}

static ImVec2 GetHandleScreenPos(const Keyframe& keyframe,
                                 bool is_in_handle,
                                 const ImVec2& keyframe_screen_pos,
                                 const ImVec2& view_size,
                                 const ImVec2& range)
{
    const Handle& handle = is_in_handle ? keyframe.m_in : keyframe.m_out;

    if (handle.m_weighted)
    {
        // Weighted: use actual offset converted to screen space
        const ImVec2 screen_offset =
            ImVec2(handle.m_offset.x * view_size.x / range.x, handle.m_offset.y * view_size.y / range.y);
        return keyframe_screen_pos + screen_offset;
    }
    else
    {
        // Non-weighted: fixed screen-space radius, direction from offset
        const float radius = std::abs(view_size.x) * 0.0075f;  // 0.75% of view width

        // Get direction in screen space
        const ImVec2 screen_dir = ImVec2(handle.m_offset.x * view_size.x / range.x, handle.m_offset.y * view_size.y / range.y);

        const float len = std::sqrt(screen_dir.x * screen_dir.x + screen_dir.y * screen_dir.y);
        if (len > 1e-6f)
        {
            return keyframe_screen_pos + ImVec2(screen_dir.x / len * radius, screen_dir.y / len * radius);
        }
        else
        {
            const float sign = is_in_handle ? -1.0f : 1.0f;
            return keyframe_screen_pos + ImVec2(sign * radius, 0.0f);
        }
    }
}

int Edit(Sequence& seq, const ImVec2& size, unsigned int id, const ImRect* clipping_rect, ImVector<EditPoint>* selected_points)
{
    static bool selecting_quad = false;
    static ImVec2 quad_selection;
    static int over_curve = -1;
    static int moving_curve = -1;
    static bool scrolling_v = false;
    static std::set<EditPoint> selection;
    static bool over_selected_point = false;

    // For handles
    static std::set<EditPoint> handle_selection;  // m_keyframe_index encodes: keyframe_idx * 2 + (is_out ? 1 : 0)
    static bool over_handle = false;

    int ret = 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::BeginChild(id, size, ImGuiChildFlags_FrameStyle);
    seq.m_focused = ImGui::IsWindowFocused();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (clipping_rect) draw_list->PushClipRect(clipping_rect->Min, clipping_rect->Max, true);

    const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(0.0f, size.y);
    const ImVec2 ssize(size.x, -size.y);
    const ImRect container(offset + ImVec2(0.0f, ssize.y), offset + ImVec2(ssize.x, 0.0f));
    ImVec2 min = seq.GetDrawMin();
    ImVec2 max = seq.GetDrawMax();

    // Handle zoom and VScroll
    if (container.Contains(io.MousePos))
    {
        if (fabsf(io.MouseWheel) > FLT_EPSILON)
        {
            const float r = (io.MousePos.y - offset.y) / ssize.y;
            float ratio_y = ImLerp(min.y, max.y, r);
            auto scale_value = [&](float v)
            {
                v -= ratio_y;
                v *= (1.0f - io.MouseWheel * 0.05f);
                v += ratio_y;
                return v;
            };
            min.y = scale_value(min.y);
            max.y = scale_value(max.y);
            seq.SetDrawMin(min);
            seq.SetDrawMax(max);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_F))
        {
            seq.Fit();
        }
        if (!scrolling_v && ImGui::IsMouseDown(2))
        {
            scrolling_v = true;
        }
    }
    ImVec2 range = max - min + ImVec2(1.0f, 0.0f);  // +1 because of inclusive last frame
    const ImVec2 view_size(size.x, -size.y);
    const ImVec2 size_of_pixel = ImVec2(1.0f, 1.0f) / view_size;
    const int curve_count = seq.GetCurveCount();
    if (scrolling_v)
    {
        float delta_h = io.MouseDelta.y * range.y * size_of_pixel.y;
        min.y -= delta_h;
        max.y -= delta_h;
        seq.SetDrawMin(min);
        seq.SetDrawMax(max);
        if (!ImGui::IsMouseDown(2))
        {
            scrolling_v = false;
        }
    }

    draw_list->AddRectFilled(offset, offset + ssize, Sequence::GetBackgroundColor());

    auto point_to_range = [&](const ImVec2& pt) { return (pt - min) / range; };
    auto range_to_point = [&](const ImVec2& pt) { return (pt * range) + min; };

    draw_list->AddLine(ImVec2(-1.0f, -min.y / range.y) * view_size + offset,
                       ImVec2(1.0f, -min.y / range.y) * view_size + offset,
                       0xFF000000,
                       1.5f);

    bool over_curve_or_point = false;

    int local_over_curve = -1;
    // Make sure highlighted curve is rendered last
    int* curves_index = static_cast<int*>(_malloca(sizeof(int) * curve_count));
    for (int c = 0; c < curve_count; c++) curves_index[c] = c;
    int high_lighted_curve_index = -1;
    if (over_curve != -1 && curve_count)
    {
        ImSwap(curves_index[over_curve], curves_index[curve_count - 1]);
        high_lighted_curve_index = over_curve;
    }

    for (int cur = 0; cur < curve_count; cur++)
    {
        int c = curves_index[cur];
        if (!seq.GetCurveVisibility(c)) continue;

        const Curve& curve = seq.m_curves.at(c);
        const int keyframe_count = seq.GetCurveKeyframeCount(c);
        if (keyframe_count < 1) continue;

        uint32_t curve_color = Sequence::GetCurveColor(c);
        if ((c == high_lighted_curve_index && selection.empty() && !selecting_quad) || moving_curve == c)
        {
            curve_color = 0xFFFFFFFF;
        }

        // Draw curve segments
        for (int k = 0; k < keyframe_count - 1; k++)
        {
            const Keyframe& k0 = curve.m_keyframes.at(k);
            const Keyframe& k1 = curve.m_keyframes.at(k + 1);

            // Check for CONSTANT segment
            bool is_constant =
                (k0.m_handle_type == HandleType::BROKEN && k0.m_out.m_broken_type == Handle::BrokenType::CONSTANT);

            if (is_constant)
            {
                // Draw step: horizontal then vertical
                ImVec2 p1 = point_to_range(k0.m_pos) * view_size + offset;
                ImVec2 p2 = ImVec2(point_to_range(k1.m_pos).x, point_to_range(k0.m_pos).y) * view_size + offset;
                ImVec2 p3 = point_to_range(k1.m_pos) * view_size + offset;

                draw_list->AddLine(p1, p2, curve_color, 1.3f);
                draw_list->AddLine(p2, p3, curve_color, 1.3f);

                if (Distance(io.MousePos.x, io.MousePos.y, p1.x, p1.y, p2.x, p2.y) < 8.0f ||
                    Distance(io.MousePos.x, io.MousePos.y, p2.x, p2.y, p3.x, p3.y) < 8.0f)
                {
                    if (!scrolling_v)
                    {
                        local_over_curve = c;
                        over_curve = c;
                        over_curve_or_point = true;
                    }
                }
            }
            else
            {
                // Draw Hermite curve with substeps
                constexpr int sub_steps = 32;
                for (int step = 0; step < sub_steps; step++)
                {
                    float t1 = static_cast<float>(step) / static_cast<float>(sub_steps);
                    float t2 = static_cast<float>(step + 1) / static_cast<float>(sub_steps);

                    // Sample using normalized t within this segment
                    float global_t1 = (static_cast<float>(k) + t1) / static_cast<float>(keyframe_count - 1);
                    float global_t2 = (static_cast<float>(k) + t2) / static_cast<float>(keyframe_count - 1);

                    ImVec2 pos1 = SampleCurveForDrawing(curve, global_t1, min, max) * view_size + offset;
                    ImVec2 pos2 = SampleCurveForDrawing(curve, global_t2, min, max) * view_size + offset;

                    if (Distance(io.MousePos.x, io.MousePos.y, pos1.x, pos1.y, pos2.x, pos2.y) < 8.0f && !scrolling_v)
                    {
                        local_over_curve = c;
                        over_curve = c;
                        over_curve_or_point = true;
                    }

                    draw_list->AddLine(pos1, pos2, curve_color, 1.3f);
                }
            }
        }

        // Draw handles
        for (int k = 0; k < keyframe_count; k++)
        {
            const Keyframe& keyframe = curve.m_keyframes.at(k);
            ImVec2 keyframe_range = point_to_range(keyframe.m_pos);
            ImVec2 keyframe_screen = keyframe_range * view_size + offset;

            // Draw in-handle
            if (ShouldShowInHandleHandle(curve, k))
            {
                ImVec2 handle_screen = GetHandleScreenPos(keyframe, true, keyframe_screen, view_size, range);
                bool is_handle_selected = handle_selection.find({c, k * 2}) != handle_selection.end();

                HandleState handle_state = DrawHandle(draw_list, handle_screen, keyframe_screen, true, is_handle_selected);
                if (handle_state != HandleState::NONE && moving_curve == -1 && !selecting_quad)
                {
                    over_curve_or_point = true;
                    over_handle = true;
                    over_curve = -1;

                    if (handle_state == HandleState::CLICKED)
                    {
                        if (!io.KeyShift) handle_selection.clear();
                        handle_selection.insert({c, k * 2});
                    }
                }
            }

            // Draw out-handle
            if (ShouldShowOutHandle(curve, k))
            {
                ImVec2 handle_screen = GetHandleScreenPos(keyframe, false, keyframe_screen, view_size, range);
                bool is_handle_selected = handle_selection.find({c, k * 2 + 1}) != handle_selection.end();

                HandleState handle_state = DrawHandle(draw_list, handle_screen, keyframe_screen, false, is_handle_selected);
                if (handle_state != HandleState::NONE && moving_curve == -1 && !selecting_quad)
                {
                    over_curve_or_point = true;
                    over_handle = true;
                    over_curve = -1;

                    if (handle_state == HandleState::CLICKED)
                    {
                        if (!io.KeyShift) handle_selection.clear();
                        handle_selection.insert({c, k * 2 + 1});
                    }
                }
            }
        }

        // Draw keyframes
        for (int k = 0; k < keyframe_count; k++)
        {
            const Keyframe& keyframe = curve.m_keyframes.at(k);

            ImVec2 keyframe_range = point_to_range(keyframe.m_pos);

            const int draw_state =
                DrawKeyframe(draw_list,
                             keyframe_range,
                             view_size,
                             offset,
                             (selection.find({c, k}) != selection.end() && moving_curve == -1 && !scrolling_v),
                             seq.IsKeyframeXMoveable(c, k),
                             seq.IsKeyframeYMoveable(c, k));

            // Display keyframe value near point
            char point_val_text[128];
            const ImVec2 point_draw_pos = keyframe_range * view_size + offset;
            ImFormatString(point_val_text, IM_ARRAYSIZE(point_val_text), "%.0f|%.2f", keyframe.m_pos.x, keyframe.m_pos.y);
            draw_list->AddText({point_draw_pos.x - 4.0f, point_draw_pos.y + 7.0f}, 0xFFFFFFFF, point_val_text);

            if (draw_state && moving_curve == -1 && !selecting_quad)
            {
                over_curve_or_point = true;
                over_selected_point = true;
                over_curve = -1;
                over_handle = false;

                if (draw_state == 2)
                {
                    if (!io.KeyShift && selection.find({c, k}) == selection.end()) selection.clear();
                    selection.insert({c, k});
                }
            }

            // Right-click on keyframe - open context menu
            if (draw_state && io.MouseClicked[1])
            {
                // If right-clicked keyframe is not in selection, replace selection with it
                if (selection.find({c, k}) == selection.end())
                {
                    selection.clear();
                    selection.insert({c, k});
                }
                ImGui::OpenPopup("KeyframeContextMenu");
            }
        }
    }
    if (local_over_curve == -1) over_curve = -1;

    // Move keyframe selection
    static bool points_moved = false;
    static ImVec2 mouse_pos_origin;
    static std::vector<ImVec2> original_points;
    if (over_selected_point && io.MouseDown[0])
    {
        if ((fabsf(io.MouseDelta.x) > 0.0f || fabsf(io.MouseDelta.y) > 0.0f) && !selection.empty())
        {
            if (!points_moved)
            {
                Sequence::BeginEdit(0);
                mouse_pos_origin = io.MousePos;
                original_points.resize(selection.size());
                int index = 0;
                for (const auto& sel : selection)
                {
                    const Curve& curve = seq.m_curves.at(sel.m_curve_index);
                    original_points.at(index++) = curve.m_keyframes.at(sel.m_keyframe_index).m_pos;
                }
            }
            points_moved = true;
            ret = 1;

            int original_index = 0;
            for (const auto& sel : selection)
            {
                const ImVec2 original = original_points.at(original_index);
                const ImVec2 delta = (io.MousePos - mouse_pos_origin) * size_of_pixel;

                const bool x_moveable = seq.IsKeyframeXMoveable(sel.m_curve_index, sel.m_keyframe_index);
                const bool y_moveable = seq.IsKeyframeYMoveable(sel.m_curve_index, sel.m_keyframe_index);

                const ImVec2 constrained_delta = ImVec2(x_moveable ? delta.x : 0.0f, y_moveable ? delta.y : 0.0f);

                const ImVec2 p = range_to_point(point_to_range(original) + constrained_delta);

                seq.EditKeyframe(sel.m_curve_index, sel.m_keyframe_index, p);
                original_index++;
            }
        }
    }
    if (over_selected_point && !io.MouseDown[0])
    {
        over_selected_point = false;
        if (points_moved)
        {
            points_moved = false;
            Sequence::EndEdit();
        }
    }

    // Move handle selection
    static bool handles_moved = false;
    if (over_handle && io.MouseDown[0] && !handle_selection.empty())
    {
        if (fabsf(io.MouseDelta.x) > 0.f || fabsf(io.MouseDelta.y) > 0.f)
        {
            if (!handles_moved)
            {
                Sequence::BeginEdit(0);
                handles_moved = true;
            }
            ret = 1;

            for (const auto& sel : handle_selection)
            {
                Curve& curve = seq.m_curves.at(sel.m_curve_index);
                int keyframe_idx = sel.m_keyframe_index / 2;
                bool is_out = (sel.m_keyframe_index % 2) == 1;
                const Keyframe& keyframe = curve.m_keyframes.at(keyframe_idx);

                // Get keyframe screen position
                ImVec2 keyframe_screen = point_to_range(keyframe.m_pos) * view_size + offset;

                // Mouse position relative to keyframe in screen space
                ImVec2 screen_delta = io.MousePos - keyframe_screen;

                // Convert screen delta to curve space offset
                ImVec2 curve_offset = ImVec2(screen_delta.x * range.x / view_size.x, screen_delta.y * range.y / view_size.y);

                if (is_out)
                {
                    if (keyframe_screen.x < io.MousePos.x)
                    {
                        SetOutHandleOffset(curve, keyframe_idx, curve_offset);
                    }
                }
                else
                {
                    if (keyframe_screen.x > io.MousePos.x)
                    {
                        SetInHandleOffset(curve, keyframe_idx, curve_offset);
                    }
                }
            }
        }
    }
    if (over_handle && !io.MouseDown[0])
    {
        over_handle = false;
        if (handles_moved)
        {
            handles_moved = false;
            Sequence::EndEdit();
        }
    }

    // Remove keyframe (double-click)
    if (over_selected_point && selection.size() == 1 && io.MouseDoubleClicked[0])
    {
        Sequence::BeginEdit(selection.begin()->m_curve_index);
        seq.RemoveKeyframeAtIdx(selection.begin()->m_curve_index, selection.begin()->m_keyframe_index);
        selection.clear();
        over_selected_point = false;
        Sequence::EndEdit();
    }

    // Add keyframe (double-click on curve)
    if (over_curve != -1 && io.MouseDoubleClicked[0])
    {
        const ImVec2 np = range_to_point((io.MousePos - offset) / view_size);
        Sequence::BeginEdit(over_curve);
        seq.AddKeyframeAtPos(over_curve, np);
        Sequence::EndEdit();
        ret = 1;
    }

    // Move entire curve
    if (moving_curve != -1)
    {
        const Curve& curve = seq.m_curves.at(moving_curve);
        const int keyframe_count = GetKeyframeCount(curve);
        if (!points_moved)
        {
            mouse_pos_origin = io.MousePos;
            points_moved = true;
            original_points.resize(keyframe_count);
            for (int k = 0; k < keyframe_count; k++)
            {
                original_points.at(k) = curve.m_keyframes.at(k).m_pos;
            }
        }
        if (keyframe_count >= 1)
        {
            for (int k = 0; k < keyframe_count; k++)
            {
                seq.EditKeyframe(
                    moving_curve,
                    k,
                    range_to_point(point_to_range(original_points.at(k)) + (io.MousePos - mouse_pos_origin) * size_of_pixel));
            }
            ret = 1;
        }
        if (!io.MouseDown[0])
        {
            moving_curve = -1;
            points_moved = false;
            Sequence::EndEdit();
        }
    }
    if (moving_curve == -1 && over_curve != -1 && ImGui::IsMouseClicked(0) && selection.empty() && !selecting_quad)
    {
        moving_curve = over_curve;
        Sequence::BeginEdit(over_curve);
    }

    // Quad selection
    if (selecting_quad)
    {
        const ImVec2 bmin = ImMin(quad_selection, io.MousePos);
        const ImVec2 bmax = ImMax(quad_selection, io.MousePos);
        draw_list->AddRectFilled(bmin, bmax, 0x40FF0000, 1.f);
        draw_list->AddRect(bmin, bmax, 0xFFFF0000, 1.f);
        const ImRect selection_quad(bmin, bmax);
        if (!io.MouseDown[0])
        {
            if (!io.KeyShift) selection.clear();
            for (int c = 0; c < curve_count; c++)
            {
                if (!seq.GetCurveVisibility(c)) continue;

                const Curve& curve = seq.m_curves.at(c);
                const int keyframe_count = GetKeyframeCount(curve);
                if (keyframe_count < 1) continue;

                for (int k = 0; k < keyframe_count; k++)
                {
                    const ImVec2 center = point_to_range(curve.m_keyframes.at(k).m_pos) * view_size + offset;
                    if (selection_quad.Contains(center)) selection.insert({c, k});
                }
            }
            selecting_quad = false;
        }
    }
    if (!over_curve_or_point && ImGui::IsMouseClicked(0) && !selecting_quad && moving_curve == -1 && !over_selected_point &&
        container.Contains(io.MousePos) && !ImGui::IsPopupOpen("KeyframeContextMenu"))
    {
        selecting_quad = true;
        quad_selection = io.MousePos;
    }

    if (clipping_rect) draw_list->PopClipRect();

    // Context menu
    if (ImGui::BeginPopup("KeyframeContextMenu"))
    {
        if (!selection.empty())
        {
            // Gather info about selection
            bool all_smooth_editable = true;
            bool all_in_editable = true;
            bool all_out_editable = true;
            bool all_both_editable = true;
            bool all_deletable = true;
            bool any_locked = false;

            // For checkmarks - track if ALL selected keyframes have this property
            bool all_smooth_auto = true;
            bool all_smooth_free = true;
            bool all_smooth_flat = true;
            bool all_in_free = true;
            bool all_in_linear = true;
            bool all_in_constant = true;
            bool all_in_weighted = true;
            bool all_out_free = true;
            bool all_out_linear = true;
            bool all_out_constant = true;
            bool all_out_weighted = true;

            for (const auto& sel : selection)
            {
                const Curve& curve = seq.m_curves.at(sel.m_curve_index);
                const Keyframe& keyframe = curve.m_keyframes.at(sel.m_keyframe_index);
                const int keyframe_count = GetKeyframeCount(curve);
                const bool is_first = (sel.m_keyframe_index == 0);
                const bool is_last = (sel.m_keyframe_index == keyframe_count - 1);

                if (curve.m_handle_type_locked)
                {
                    any_locked = true;
                    all_smooth_editable = false;
                    all_in_editable = false;
                    all_out_editable = false;
                    all_both_editable = false;
                }
                else
                {
                    if (!IsInHandleEditable(curve, sel.m_keyframe_index)) all_in_editable = false;
                    if (!IsOutHandleEditable(curve, sel.m_keyframe_index)) all_out_editable = false;
                    if (!IsInHandleEditable(curve, sel.m_keyframe_index) || !IsOutHandleEditable(curve, sel.m_keyframe_index))
                        all_both_editable = false;
                }

                if (is_first || is_last) all_deletable = false;

                // Check smooth types
                if (!(keyframe.m_handle_type == HandleType::SMOOTH && keyframe.m_in.m_smooth_type == Handle::SmoothType::AUTO))
                    all_smooth_auto = false;
                if (!(keyframe.m_handle_type == HandleType::SMOOTH && keyframe.m_in.m_smooth_type == Handle::SmoothType::FREE))
                    all_smooth_free = false;
                if (!(keyframe.m_handle_type == HandleType::SMOOTH && keyframe.m_in.m_smooth_type == Handle::SmoothType::FLAT))
                    all_smooth_flat = false;

                // Check in handle types
                if (!(keyframe.m_handle_type == HandleType::BROKEN && keyframe.m_in.m_broken_type == Handle::BrokenType::FREE))
                    all_in_free = false;
                if (!(keyframe.m_handle_type == HandleType::BROKEN &&
                      keyframe.m_in.m_broken_type == Handle::BrokenType::LINEAR))
                    all_in_linear = false;
                if (!(keyframe.m_handle_type == HandleType::BROKEN &&
                      keyframe.m_in.m_broken_type == Handle::BrokenType::CONSTANT))
                    all_in_constant = false;
                if (!keyframe.m_in.m_weighted) all_in_weighted = false;

                // Check out handle types
                if (!(keyframe.m_handle_type == HandleType::BROKEN && keyframe.m_out.m_broken_type == Handle::BrokenType::FREE))
                    all_out_free = false;
                if (!(keyframe.m_handle_type == HandleType::BROKEN &&
                      keyframe.m_out.m_broken_type == Handle::BrokenType::LINEAR))
                    all_out_linear = false;
                if (!(keyframe.m_handle_type == HandleType::BROKEN &&
                      keyframe.m_out.m_broken_type == Handle::BrokenType::CONSTANT))
                    all_out_constant = false;
                if (!keyframe.m_out.m_weighted) all_out_weighted = false;
            }

            // Smooth submenu
            if (any_locked)
            {
                if (ImGui::BeginMenu("Locked By Curve Handle Type", false))
                {
                    ImGui::EndMenu();
                }
            }
            else
            {
                if (ImGui::BeginMenu("Smooth", all_smooth_editable))
                {
                    if (ImGui::MenuItem("Auto", nullptr, all_smooth_auto))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetKeyframeSmoothType(curve, sel.m_keyframe_index, Handle::SmoothType::AUTO);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Free", nullptr, all_smooth_free))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetKeyframeSmoothType(curve, sel.m_keyframe_index, Handle::SmoothType::FREE);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Flat", nullptr, all_smooth_flat))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetKeyframeSmoothType(curve, sel.m_keyframe_index, Handle::SmoothType::FLAT);
                        }
                        Sequence::EndEdit();
                    }
                    ImGui::EndMenu();
                }

                // Left Handle submenu
                if (ImGui::BeginMenu("Left Handle", all_in_editable))
                {
                    if (ImGui::MenuItem("Free", nullptr, all_in_free))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetInHandleBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::FREE);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Linear", nullptr, all_in_linear))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetInHandleBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::LINEAR);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Constant", nullptr, all_in_constant, false))
                    {
                        // Disabled for in-handle
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Weighted", nullptr, all_in_weighted))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetInHandleWeighted(curve, sel.m_keyframe_index, !all_in_weighted);
                        }
                        Sequence::EndEdit();
                    }
                    ImGui::EndMenu();
                }

                // Right Handle submenu
                if (ImGui::BeginMenu("Right Handle", all_out_editable))
                {
                    if (ImGui::MenuItem("Free", nullptr, all_out_free))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetOutHandleBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::FREE);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Linear", nullptr, all_out_linear))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetOutHandleBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::LINEAR);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Constant", nullptr, all_out_constant))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetOutHandleBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::CONSTANT);
                        }
                        Sequence::EndEdit();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Weighted", nullptr, all_out_weighted))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetOutHandleWeighted(curve, sel.m_keyframe_index, !all_out_weighted);
                        }
                        Sequence::EndEdit();
                    }
                    ImGui::EndMenu();
                }

                // Both Handles submenu
                if (ImGui::BeginMenu("Both Handles", all_both_editable))
                {
                    bool all_both_free = all_in_free && all_out_free;
                    bool all_both_linear = all_in_linear && all_out_linear;
                    bool all_both_weighted = all_in_weighted && all_out_weighted;

                    if (ImGui::MenuItem("Free", nullptr, all_both_free))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetBothHandlesBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::FREE);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Linear", nullptr, all_both_linear))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetBothHandlesBrokenType(curve, sel.m_keyframe_index, Handle::BrokenType::LINEAR);
                        }
                        Sequence::EndEdit();
                    }
                    if (ImGui::MenuItem("Constant", nullptr, false, false))
                    {
                        // Disabled for both
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Weighted", nullptr, all_both_weighted))
                    {
                        Sequence::BeginEdit(0);
                        for (const auto& sel : selection)
                        {
                            Curve& curve = seq.m_curves.at(sel.m_curve_index);
                            SetBothHandlesWeighted(curve, sel.m_keyframe_index, !all_both_weighted);
                        }
                        Sequence::EndEdit();
                    }
                    ImGui::EndMenu();
                }

                ImGui::Separator();

                // Reset handles
                if (ImGui::MenuItem("Reset Handles", nullptr, false, !any_locked))
                {
                    Sequence::BeginEdit(0);
                    for (const auto& sel : selection)
                    {
                        seq.ResetHandlesForKeyframe(sel.m_curve_index, sel.m_keyframe_index);
                    }
                    Sequence::EndEdit();
                }
            }

            // Delete Keyframes
            if (ImGui::MenuItem("Delete Keyframes", nullptr, false, all_deletable))
            {
                Sequence::BeginEdit(0);
                std::vector<EditPoint> to_delete;
                for (const auto& sel : selection)
                {
                    const Curve& curve = seq.m_curves.at(sel.m_curve_index);
                    const int keyframe_count = GetKeyframeCount(curve);
                    const bool is_first = (sel.m_keyframe_index == 0);
                    const bool is_last = (sel.m_keyframe_index == keyframe_count - 1);
                    if (!is_first && !is_last)
                    {
                        to_delete.push_back(sel);
                    }
                }
                std::sort(to_delete.begin(),
                          to_delete.end(),
                          [](const EditPoint& a, const EditPoint& b)
                          {
                              if (a.m_curve_index != b.m_curve_index) return a.m_curve_index > b.m_curve_index;
                              return a.m_keyframe_index > b.m_keyframe_index;
                          });
                for (const auto& sel : to_delete)
                {
                    seq.RemoveKeyframeAtIdx(sel.m_curve_index, sel.m_keyframe_index);
                }
                Sequence::EndEdit();
                selection.clear();
            }
        }

        ImGui::EndPopup();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    if (selected_points)
    {
        selected_points->resize(static_cast<int>(selection.size()));
        int index = 0;
        for (const auto& point : selection) (*selected_points)[index++] = point;
    }
    _freea(curves_index);
    return ret;
}

glm::quat SampleQuatForAnimation(Sequence& seq, float time)
{
    const Curve& curve_w = seq.m_curves.at(0);
    const Curve& curve_x = seq.m_curves.at(1);
    const Curve& curve_y = seq.m_curves.at(2);
    const Curve& curve_z = seq.m_curves.at(3);
    const Curve& curve_spins = seq.m_curves.at(4);

    const int keyframe_count = GetKeyframeCount(curve_w);
    if (keyframe_count == 0) return {1.0f, 0.0f, 0.0f, 0.0f};

    // Before first keyframe
    if (time <= curve_w.m_keyframes.at(0).Time())
    {
        return {curve_w.m_keyframes.at(0).Value(),
                curve_x.m_keyframes.at(0).Value(),
                curve_y.m_keyframes.at(0).Value(),
                curve_z.m_keyframes.at(0).Value()};
    }

    // After last keyframe
    if (time >= curve_w.m_keyframes.at(keyframe_count - 1).Time())
    {
        return {curve_w.m_keyframes.at(keyframe_count - 1).Value(),
                curve_x.m_keyframes.at(keyframe_count - 1).Value(),
                curve_y.m_keyframes.at(keyframe_count - 1).Value(),
                curve_z.m_keyframes.at(keyframe_count - 1).Value()};
    }

    // Find segment
    const int seg = FindSegmentIndex(curve_w, time);
    if (seg < 0)
    {
        return {curve_w.m_keyframes.at(0).Value(),
                curve_x.m_keyframes.at(0).Value(),
                curve_y.m_keyframes.at(0).Value(),
                curve_z.m_keyframes.at(0).Value()};
    }

    const Keyframe& k0 = curve_w.m_keyframes.at(seg);
    const Keyframe& k1 = curve_w.m_keyframes.at(seg + 1);

    const glm::quat q_a(curve_w.m_keyframes.at(seg).Value(),
                        curve_x.m_keyframes.at(seg).Value(),
                        curve_y.m_keyframes.at(seg).Value(),
                        curve_z.m_keyframes.at(seg).Value());

    const glm::quat q_b(curve_w.m_keyframes.at(seg + 1).Value(),
                        curve_x.m_keyframes.at(seg + 1).Value(),
                        curve_y.m_keyframes.at(seg + 1).Value(),
                        curve_z.m_keyframes.at(seg + 1).Value());

    const int spins = static_cast<int>(curve_spins.m_keyframes.at(seg + 1).Value());

    // CONSTANT - step function
    if (k0.m_handle_type == HandleType::BROKEN && k0.m_out.m_broken_type == Handle::BrokenType::CONSTANT)
    {
        return q_a;
    }

    const float segment_duration = k1.Time() - k0.Time();
    if (segment_duration < 1e-6f) return q_a;

    float segment_t = (time - k0.Time()) / segment_duration;

    // FLAT - smoothstep easing
    if (k0.m_handle_type == HandleType::SMOOTH && k0.m_out.m_smooth_type == Handle::SmoothType::FLAT)
    {
        segment_t = segment_t * segment_t * (3.0f - 2.0f * segment_t);
    }

    // LINEAR - use segment_t as-is

    return glm::slerp(q_a, q_b, segment_t, spins);
}

}  // namespace tanim::sequencer
