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
#include "tanim/include/hermite.hpp"
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

#ifndef IMGUI_DEFINE_MATH_OPERATORS
static ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }

static ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }

static ImVec2 operator*(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x * b.x, a.y * b.y); }

static ImVec2 operator/(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x / b.x, a.y / b.y); }

static ImVec2 operator*(const ImVec2& a, const float b) { return ImVec2(a.x * b, a.y * b); }
#endif

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

static int DrawTangentHandle(ImDrawList* draw_list,
                             const ImVec2& handle_screen_pos,
                             const ImVec2& keyframe_screen_pos,
                             bool is_in_handle,
                             bool is_selected)
{
    int ret = 0;
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
        ret = 1;
        fill_color = 0xFFFFFFFF;  // White fill on hover
        if (io.MouseDown[0]) ret = 2;
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

static int DrawKeyframe(ImDrawList* draw_list, const ImVec2& pos, const ImVec2& size, const ImVec2& offset, bool edited)
{
    int ret = 0;
    ImGuiIO& io = ImGui::GetIO();

    static constexpr ImVec2 LOCAL_OFFSETS[4] = {ImVec2(1, 0), ImVec2(0, 1), ImVec2(-1, 0), ImVec2(0, -1)};
    ImVec2 offsets[4];
    for (int i = 0; i < 4; i++)
    {
        offsets[i] = pos * size + LOCAL_OFFSETS[i] * 4.5f + offset;
    }

    const ImVec2 center = pos * size + offset;
    const ImRect anchor(center - ImVec2(5, 5), center + ImVec2(5, 5));
    draw_list->AddConvexPolyFilled(offsets, 4, 0xFF000000);
    if (anchor.Contains(io.MousePos))
    {
        ret = 1;
        if (io.MouseDown[0]) ret = 2;
    }

    if (edited)
        draw_list->AddPolyline(offsets, 4, 0xFFFFFFFF, true, 3.0f);
    else if (ret)
        draw_list->AddPolyline(offsets, 4, 0xFF80B0FF, true, 2.0f);
    else
        draw_list->AddPolyline(offsets, 4, 0xFF0080FF, true, 2.0f);

    return ret;
}

static ImVec2 GetTangentHandleScreenPos(const Keyframe& keyframe,
                                        bool is_in_tangent,
                                        const ImVec2& keyframe_screen_pos,
                                        const ImVec2& view_size,
                                        const ImVec2& range)
{
    const Tangent& tangent = is_in_tangent ? keyframe.m_in : keyframe.m_out;

    // m_offset is in curve space, convert to screen space
    ImVec2 screen_offset = ImVec2(tangent.m_offset.x * view_size.x / range.x, tangent.m_offset.y * view_size.y / range.y);

    return keyframe_screen_pos + screen_offset;
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

    static int right_clicked_curve = -1;
    static int right_clicked_keyframe = -1;

    // For tangent handles
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
                (k0.m_tangent_type == TangentType::BROKEN && k0.m_out.m_broken_type == Tangent::BrokenType::CONSTANT);

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

        // Draw tangent handles
        for (int k = 0; k < keyframe_count; k++)
        {
            const Keyframe& keyframe = curve.m_keyframes.at(k);
            ImVec2 keyframe_range = point_to_range(keyframe.m_pos);
            ImVec2 keyframe_screen = keyframe_range * view_size + offset;

            // Draw in-handle
            if (ShouldShowInTangentHandle(curve, k))
            {
                ImVec2 handle_screen = GetTangentHandleScreenPos(keyframe, true, keyframe_screen, view_size, range);
                bool is_handle_selected = handle_selection.find({c, k * 2}) != handle_selection.end();

                int handle_state = DrawTangentHandle(draw_list, handle_screen, keyframe_screen, true, is_handle_selected);
                if (handle_state && moving_curve == -1 && !selecting_quad)
                {
                    over_curve_or_point = true;
                    over_handle = true;
                    over_curve = -1;

                    if (handle_state == 2)
                    {
                        if (!io.KeyShift) handle_selection.clear();
                        handle_selection.insert({c, k * 2});
                    }
                }
            }

            // Draw out-handle
            if (ShouldShowOutTangentHandle(curve, k))
            {
                ImVec2 handle_screen = GetTangentHandleScreenPos(keyframe, false, keyframe_screen, view_size, range);
                bool is_handle_selected = handle_selection.find({c, k * 2 + 1}) != handle_selection.end();

                int handle_state = DrawTangentHandle(draw_list, handle_screen, keyframe_screen, false, is_handle_selected);
                if (handle_state && moving_curve == -1 && !selecting_quad)
                {
                    over_curve_or_point = true;
                    over_handle = true;
                    over_curve = -1;

                    if (handle_state == 2)
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
                             (selection.find({c, k}) != selection.end() && moving_curve == -1 && !scrolling_v));

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
                selection.clear();
                selection.insert({c, k});
                right_clicked_curve = c;
                right_clicked_keyframe = k;
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
                const ImVec2 p = range_to_point(point_to_range(original_points[original_index]) +
                                                (io.MousePos - mouse_pos_origin) * size_of_pixel);

                seq.EditKeyframe(sel.m_curve_index, sel.m_keyframe_index, p);
                original_index++;

                // const int newIndex = seq.EditKeyframe(sel.m_curve_index, sel.m_keyframe_index, p);
                // if (newIndex != sel.m_keyframe_index)
                // {
                //     selection.erase(sel);
                //     selection.insert({sel.m_curve_index, newIndex});
                // }
                // original_index++;
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

    // Move tangent handle selection
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
                    SetOutTangentOffset(curve, keyframe_idx, curve_offset);
                }
                else
                {
                    SetInTangentOffset(curve, keyframe_idx, curve_offset);
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
        container.Contains(io.MousePos))
    {
        selecting_quad = true;
        quad_selection = io.MousePos;
    }

    if (clipping_rect) draw_list->PopClipRect();

    // Context menu
    if (ImGui::BeginPopup("KeyframeContextMenu"))
    {
        if (right_clicked_curve >= 0 && right_clicked_keyframe >= 0)
        {
            const Curve& curve = seq.m_curves.at(right_clicked_curve);
            const Keyframe& keyframe = curve.m_keyframes.at(right_clicked_keyframe);
            const int keyframe_count = GetKeyframeCount(curve);

            const bool is_first = (right_clicked_keyframe == 0);
            const bool is_last = (right_clicked_keyframe == keyframe_count - 1);
            const bool in_editable = !is_first;
            const bool out_editable = !is_last;

            // Smooth submenu
            if (ImGui::BeginMenu("Smooth"))
            {
                bool is_smooth_auto = (keyframe.m_tangent_type == TangentType::SMOOTH &&
                                       keyframe.m_in.m_smooth_type == Tangent::SmoothType::AUTO);
                bool is_smooth_free = (keyframe.m_tangent_type == TangentType::SMOOTH &&
                                       keyframe.m_in.m_smooth_type == Tangent::SmoothType::FREE);
                bool is_smooth_flat = (keyframe.m_tangent_type == TangentType::SMOOTH &&
                                       keyframe.m_in.m_smooth_type == Tangent::SmoothType::FLAT);

                if (ImGui::MenuItem("Auto", nullptr, is_smooth_auto))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeSmoothType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::SmoothType::AUTO);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Free", nullptr, is_smooth_free))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeSmoothType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::SmoothType::FREE);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Flat", nullptr, is_smooth_flat))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeSmoothType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::SmoothType::FLAT);
                    Sequence::EndEdit();
                }
                ImGui::EndMenu();
            }

            // Broken submenu
            if (ImGui::BeginMenu("Broken"))
            {
                bool all_broken_free = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                        keyframe.m_in.m_broken_type == Tangent::BrokenType::FREE &&
                                        keyframe.m_out.m_broken_type == Tangent::BrokenType::FREE);
                bool all_broken_linear = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                          keyframe.m_in.m_broken_type == Tangent::BrokenType::LINEAR &&
                                          keyframe.m_out.m_broken_type == Tangent::BrokenType::LINEAR);
                bool all_broken_constant = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                            keyframe.m_in.m_broken_type == Tangent::BrokenType::CONSTANT &&
                                            keyframe.m_out.m_broken_type == Tangent::BrokenType::CONSTANT);

                if (ImGui::MenuItem("Free", nullptr, all_broken_free))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeBrokenType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::BrokenType::FREE,
                                          Tangent::BrokenType::FREE);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Linear", nullptr, all_broken_linear))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeBrokenType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::BrokenType::LINEAR,
                                          Tangent::BrokenType::LINEAR);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Constant", nullptr, all_broken_constant))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetKeyframeBrokenType(seq.m_curves.at(right_clicked_curve),
                                          right_clicked_keyframe,
                                          Tangent::BrokenType::CONSTANT,
                                          Tangent::BrokenType::CONSTANT);
                    Sequence::EndEdit();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            // In Tangent submenu
            if (ImGui::BeginMenu("In Tangent", in_editable))
            {
                bool is_in_free = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                   keyframe.m_in.m_broken_type == Tangent::BrokenType::FREE);
                bool is_in_linear = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                     keyframe.m_in.m_broken_type == Tangent::BrokenType::LINEAR);
                bool is_in_constant = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                       keyframe.m_in.m_broken_type == Tangent::BrokenType::CONSTANT);
                bool is_in_weighted = keyframe.m_in.m_weighted;

                if (ImGui::MenuItem("Free", nullptr, is_in_free))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetInTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                           right_clicked_keyframe,
                                           Tangent::BrokenType::FREE);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Linear", nullptr, is_in_linear))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetInTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                           right_clicked_keyframe,
                                           Tangent::BrokenType::LINEAR);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Constant", nullptr, is_in_constant))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetInTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                           right_clicked_keyframe,
                                           Tangent::BrokenType::CONSTANT);
                    Sequence::EndEdit();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Weighted", nullptr, is_in_weighted))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetInTangentWeighted(seq.m_curves.at(right_clicked_curve), right_clicked_keyframe, !is_in_weighted);
                    Sequence::EndEdit();
                }
                ImGui::EndMenu();
            }

            // Out Tangent submenu
            if (ImGui::BeginMenu("Out Tangent", out_editable))
            {
                bool is_out_free = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                    keyframe.m_out.m_broken_type == Tangent::BrokenType::FREE);
                bool is_out_linear = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                      keyframe.m_out.m_broken_type == Tangent::BrokenType::LINEAR);
                bool is_out_constant = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                        keyframe.m_out.m_broken_type == Tangent::BrokenType::CONSTANT);
                bool is_out_weighted = keyframe.m_out.m_weighted;

                if (ImGui::MenuItem("Free", nullptr, is_out_free))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetOutTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                            right_clicked_keyframe,
                                            Tangent::BrokenType::FREE);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Linear", nullptr, is_out_linear))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetOutTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                            right_clicked_keyframe,
                                            Tangent::BrokenType::LINEAR);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Constant", nullptr, is_out_constant))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetOutTangentBrokenType(seq.m_curves.at(right_clicked_curve),
                                            right_clicked_keyframe,
                                            Tangent::BrokenType::CONSTANT);
                    Sequence::EndEdit();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Weighted", nullptr, is_out_weighted))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetOutTangentWeighted(seq.m_curves.at(right_clicked_curve), right_clicked_keyframe, !is_out_weighted);
                    Sequence::EndEdit();
                }
                ImGui::EndMenu();
            }

            // Both Tangents submenu
            if (ImGui::BeginMenu("Both Tangents"))
            {
                bool both_free = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                  keyframe.m_in.m_broken_type == Tangent::BrokenType::FREE &&
                                  keyframe.m_out.m_broken_type == Tangent::BrokenType::FREE);
                bool both_linear = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                    keyframe.m_in.m_broken_type == Tangent::BrokenType::LINEAR &&
                                    keyframe.m_out.m_broken_type == Tangent::BrokenType::LINEAR);
                bool both_constant = (keyframe.m_tangent_type == TangentType::BROKEN &&
                                      keyframe.m_in.m_broken_type == Tangent::BrokenType::CONSTANT &&
                                      keyframe.m_out.m_broken_type == Tangent::BrokenType::CONSTANT);
                bool both_weighted = keyframe.m_in.m_weighted && keyframe.m_out.m_weighted;

                if (ImGui::MenuItem("Free", nullptr, both_free))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetBothTangentsBrokenType(seq.m_curves.at(right_clicked_curve),
                                              right_clicked_keyframe,
                                              Tangent::BrokenType::FREE);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Linear", nullptr, both_linear))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetBothTangentsBrokenType(seq.m_curves.at(right_clicked_curve),
                                              right_clicked_keyframe,
                                              Tangent::BrokenType::LINEAR);
                    Sequence::EndEdit();
                }
                if (ImGui::MenuItem("Constant", nullptr, both_constant))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetBothTangentsBrokenType(seq.m_curves.at(right_clicked_curve),
                                              right_clicked_keyframe,
                                              Tangent::BrokenType::CONSTANT);
                    Sequence::EndEdit();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Weighted", nullptr, both_weighted))
                {
                    Sequence::BeginEdit(right_clicked_curve);
                    SetBothTangentsWeighted(seq.m_curves.at(right_clicked_curve), right_clicked_keyframe, !both_weighted);
                    Sequence::EndEdit();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            // Reset Tangents
            if (ImGui::MenuItem("Reset Tangents"))
            {
                Sequence::BeginEdit(right_clicked_curve);
                seq.ResetTangentsForKeyframe(right_clicked_curve, right_clicked_keyframe);
                Sequence::EndEdit();
            }

            // Delete Keyframe
            bool can_delete = !is_first && !is_last;
            if (ImGui::MenuItem("Delete Keyframe", nullptr, false, can_delete))
            {
                Sequence::BeginEdit(right_clicked_curve);
                seq.RemoveKeyframeAtIdx(right_clicked_curve, right_clicked_keyframe);
                Sequence::EndEdit();
                selection.clear();
            }
        }

        ImGui::EndPopup();
    }
    else
    {
        right_clicked_curve = -1;
        right_clicked_keyframe = -1;
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

glm::quat SampleQuatForAnimation(Sequence& /*seq*/, float /*time*/)
{
    // const auto& pts_w = seq.GetCurvePointsList(0);
    // const auto& pts_x = seq.GetCurvePointsList(1);
    // const auto& pts_y = seq.GetCurvePointsList(2);
    // const auto& pts_z = seq.GetCurvePointsList(3);
    // const auto& pts_spins = seq.GetCurvePointsList(4);
    // const int pt_count = (int)pts_w.size();
    //
    // for (int i = 0; i < pt_count - 1; i++)
    // {
    //     if (time >= pts_w.at(i).x && time <= pts_w.at(i + 1).x)
    //     {
    //         const float segment_t = (time - pts_w.at(i).x) / (pts_w.at(i + 1).x - pts_w.at(i).x);
    //         const auto q_a = glm::quat(pts_w.at(i).y, pts_x.at(i).y, pts_y.at(i).y, pts_z.at(i).y);
    //         const auto q_b = glm::quat(pts_w.at(i + 1).y, pts_x.at(i + 1).y, pts_y.at(i + 1).y, pts_z.at(i + 1).y);
    //
    //         if (seq.GetCurveLerpType(0) == LerpType::LINEAR)
    //         {
    //             return glm::slerp(q_a, q_b, segment_t, (int)pts_spins.at(i + 1).y);
    //         }
    //         else if (seq.GetCurveLerpType(0) == LerpType::SMOOTH)
    //         {
    //             const float smoothT = smoothstep(0.0f, 1.0f, segment_t);
    //             return glm::slerp(q_a, q_b, smoothT, (int)pts_spins.at(i + 1).y);
    //         }
    //         else if (seq.GetCurveLerpType(0) == LerpType::DISCRETE)
    //         {
    //             return q_a;
    //         }
    //         else
    //         {
    //             assert(0);  /// Unsupported LerpType
    //         }
    //     }
    // }
    //
    // const auto q_0 = glm::quat(pts_w.at(0).y, pts_x.at(0).y, pts_y.at(0).y, pts_z.at(0).y);
    // if (time <= pts_w.at(0).x) return q_0;
    //
    // const auto q_last =
    //     glm::quat(pts_w.at(pt_count - 1).y, pts_x.at(pt_count - 1).y, pts_y.at(pt_count - 1).y, pts_z.at(pt_count - 1).y);
    // if (time >= pts_w.at(pt_count - 1).x) return q_last;

    // return q_0;

    return {};
}

}  // namespace tanim::sequencer
