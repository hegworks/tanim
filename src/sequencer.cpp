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
#include "tanim/include/includes.hpp"
#include "tanim/include/sequence.hpp"

#include <stdint.h>
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

static float smoothstep(float edge0, float edge1, float x)
{
    x = ImClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3 - 2 * x);
}

static float distance(float x, float y, float x1, float y1, float x2, float y2)
{
    float A = x - x1;
    float B = y - y1;
    float C = x2 - x1;
    float D = y2 - y1;

    float dot = A * C + B * D;
    float len_sq = C * C + D * D;
    float param = -1.f;
    if (len_sq > FLT_EPSILON) param = dot / len_sq;

    float xx, yy;

    if (param < 0.f)
    {
        xx = x1;
        yy = y1;
    }
    else if (param > 1.f)
    {
        xx = x2;
        yy = y2;
    }
    else
    {
        xx = x1 + param * C;
        yy = y1 + param * D;
    }

    float dx = x - xx;
    float dy = y - yy;
    return sqrtf(dx * dx + dy * dy);
}

// Cubic bezier evaluation: P(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3
static ImVec2 CubicBezier(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float t)
{  // REF: by claude.ai
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    return ImVec2(uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
                  uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y);
}

// Find t for a given X using Newton-Raphson iteration (for sampling by time)
static float FindTForX(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, float targetX)
{  // REF: by claude.ai
    // Initial guess using linear interpolation
    float t = (targetX - p0.x) / (p3.x - p0.x);
    t = ImClamp(t, 0.0f, 1.0f);

    // Newton-Raphson iterations
    for (int i = 0; i < 8; i++)
    {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;

        // Current X value
        float x = u * u * u * p0.x + 3 * u * u * t * p1.x + 3 * u * t * t * p2.x + t * t * t * p3.x;
        float error = x - targetX;

        if (fabsf(error) < 1e-6f) break;

        // Derivative of X with respect to t
        float dx = 3 * uu * (p1.x - p0.x) + 6 * u * t * (p2.x - p1.x) + 3 * tt * (p3.x - p0.x);

        if (fabsf(dx) < 1e-6f) break;

        t -= error / dx;
        t = ImClamp(t, 0.0f, 1.0f);
    }

    return t;
}

static int DrawBezierHandle(ImDrawList* draw_list,
                            ImVec2 pos,
                            ImVec2 keyframePos,
                            const ImVec2& viewSize,
                            const ImVec2& offset,
                            bool isInHandle,
                            bool isSelected)
{  // REF: by claude.ai
    int ret = 0;
    ImGuiIO& io = ImGui::GetIO();

    ImVec2 handleScreenPos = pos * viewSize + offset;
    ImVec2 keyScreenPos = keyframePos * viewSize + offset;

    // Draw line from keyframe to handle
    draw_list->AddLine(keyScreenPos, handleScreenPos, 0xFF808080, 1.0f);

    // Handle colors
    ImU32 borderColor = isInHandle ? 0xFF0000FF : 0xFFFF0000;  // Red for in, Blue for out (BGR)
    ImU32 fillColor = 0x00000000;                              // Transparent by default

    const float radius = 4.0f;
    const ImRect handleRect(handleScreenPos - ImVec2(radius, radius), handleScreenPos + ImVec2(radius, radius));

    bool hovered = handleRect.Contains(io.MousePos);
    if (hovered)
    {
        ret = 1;
        fillColor = 0xFFFFFFFF;  // White fill on hover
        if (io.MouseDown[0]) ret = 2;
    }

    if (isSelected)
    {
        fillColor = 0xFFFFFFFF;
    }

    // Draw handle circle
    if (fillColor != 0x00000000) draw_list->AddCircleFilled(handleScreenPos, radius, fillColor);
    draw_list->AddCircle(handleScreenPos, radius, borderColor, 0, 2.0f);

    return ret;
}

static int DrawPoint(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited)
{
    int ret = 0;
    ImGuiIO& io = ImGui::GetIO();

    static const ImVec2 localOffsets[4] = {ImVec2(1, 0), ImVec2(0, 1), ImVec2(-1, 0), ImVec2(0, -1)};
    ImVec2 offsets[4];
    for (int i = 0; i < 4; i++)
    {
        offsets[i] = pos * size + localOffsets[i] * 4.5f + offset;
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

int Edit(SequenceInterface& delegate,
         const ImVec2& size,
         unsigned int id,
         const ImRect* clippingRect,
         ImVector<EditPoint>* selectedPoints)
{
    static bool selectingQuad = false;
    static ImVec2 quadSelection;
    static int overCurve = -1;
    static int movingCurve = -1;
    static bool scrollingV = false;
    static std::set<EditPoint> selection;
    static bool overSelectedPoint = false;

    static int rightClickedCurve = -1;
    static int rightClickedKeyframe = -1;

    // for bezier
    static std::set<EditPoint> handleSelection;  // Separate selection for handles
    static bool overHandle = false;

    int ret = 0;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::BeginChild(id, size, ImGuiChildFlags_FrameStyle);
    delegate.m_focused = ImGui::IsWindowFocused();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (clippingRect) draw_list->PushClipRect(clippingRect->Min, clippingRect->Max, true);

    const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(0.f, size.y);
    const ImVec2 ssize(size.x, -size.y);
    const ImRect container(offset + ImVec2(0.f, ssize.y), offset + ImVec2(ssize.x, 0.f));
    ImVec2 min = delegate.GetDrawMin();
    ImVec2 max = delegate.GetDrawMax();

    // handle zoom and VScroll
    if (container.Contains(io.MousePos))
    {
        if (fabsf(io.MouseWheel) > FLT_EPSILON)
        {
            const float r = (io.MousePos.y - offset.y) / ssize.y;
            float ratioY = ImLerp(min.y, max.y, r);
            auto scaleValue = [&](float v)
            {
                v -= ratioY;
                v *= (1.f - io.MouseWheel * 0.05f);
                v += ratioY;
                return v;
            };
            min.y = scaleValue(min.y);
            max.y = scaleValue(max.y);
            delegate.SetDrawMin(min);
            delegate.SetDrawMax(max);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_F))
        {
            delegate.Fit();
        }
        if (!scrollingV && ImGui::IsMouseDown(2))
        {
            scrollingV = true;
        }
    }
    ImVec2 range = max - min + ImVec2(1.f, 0.f);  // +1 because of inclusive last frame

    const ImVec2 viewSize(size.x, -size.y);
    const ImVec2 sizeOfPixel = ImVec2(1.f, 1.f) / viewSize;
    const int curveCount = delegate.GetCurveCount();

    if (scrollingV)
    {
        float deltaH = io.MouseDelta.y * range.y * sizeOfPixel.y;
        min.y -= deltaH;
        max.y -= deltaH;
        delegate.SetDrawMin(min);
        delegate.SetDrawMax(max);
        if (!ImGui::IsMouseDown(2)) scrollingV = false;
    }

    draw_list->AddRectFilled(offset, offset + ssize, delegate.GetBackgroundColor());

    auto pointToRange = [&](ImVec2 pt) { return (pt - min) / range; };
    auto rangeToPoint = [&](ImVec2 pt) { return (pt * range) + min; };

    draw_list->AddLine(ImVec2(-1.f, -min.y / range.y) * viewSize + offset,
                       ImVec2(1.f, -min.y / range.y) * viewSize + offset,
                       0xFF000000,
                       1.5f);
    bool overCurveOrPoint = false;

    int localOverCurve = -1;
    // make sure highlighted curve is rendered last
    int* curvesIndex = (int*)_malloca(sizeof(int) * curveCount);
    for (size_t c = 0; c < curveCount; c++) curvesIndex[c] = int(c);
    int highLightedCurveIndex = -1;
    if (overCurve != -1 && curveCount)
    {
        ImSwap(curvesIndex[overCurve], curvesIndex[curveCount - 1]);
        highLightedCurveIndex = overCurve;
    }

    for (size_t cur = 0; cur < curveCount; cur++)
    {
        int c = curvesIndex[cur];
        if (!delegate.GetCurveVisibility(c)) continue;
        const size_t ptCount = delegate.GetCurvePointCount(c);
        if (ptCount < 1) continue;
        LerpType curveType = delegate.GetCurveLerpType(c);
        if (curveType == LerpType::NONE) continue;
        const std::vector<ImVec2>& pts = delegate.GetCurvePointsList(c);
        uint32_t curveColor = delegate.GetCurveColor(c);
        if ((c == highLightedCurveIndex && selection.empty() && !selectingQuad) || movingCurve == c) curveColor = 0xFFFFFFFF;

        for (size_t p = 0; p < ptCount - 1; p++)
        {
            if (curveType == LerpType::SMOOTH || curveType == LerpType::LINEAR)
            {
                size_t subStepCount = (curveType == LerpType::SMOOTH) ? 20 : 2;  // TODO(tanim) hardcoded 20
                float step = 1.f / float(subStepCount - 1);

                for (size_t substep = 0; substep < subStepCount - 1; substep++)
                {
                    float t1 = float(p + float(substep) * step) / float(ptCount - 1);
                    float t2 = float(p + float(substep + 1) * step) / float(ptCount - 1);

                    const ImVec2 pos1 = SampleCurveForDrawing(pts, ptCount, t1, curveType, min, max) * viewSize + offset;
                    const ImVec2 pos2 = SampleCurveForDrawing(pts, ptCount, t2, curveType, min, max) * viewSize + offset;

                    if (distance(io.MousePos.x, io.MousePos.y, pos1.x, pos1.y, pos2.x, pos2.y) < 8.f && !scrollingV)
                    {
                        localOverCurve = int(c);
                        overCurve = int(c);
                        overCurveOrPoint = true;
                    }
                    draw_list->AddLine(pos1, pos2, curveColor, 1.3f);
                }  // substep
            }
            else if (curveType == LerpType::DISCRETE)
            {
                const ImVec2 p1 = pointToRange(pts[p]);
                const ImVec2 p2 = pointToRange(pts[p + 1]);

                ImVec2 dp1 = p1 * viewSize + offset;
                ImVec2 dp2 = ImVec2(p2.x, p1.y) * viewSize + offset;
                ImVec2 dp3 = p2 * viewSize + offset;
                draw_list->AddLine(dp1, dp2, curveColor, 1.3f);  // horizontal
                draw_list->AddLine(dp2, dp3, curveColor, 1.3f);  // vertical

                if ((distance(io.MousePos.x, io.MousePos.y, dp1.x, dp1.y, dp3.x, dp1.y) < 8.f ||
                     distance(io.MousePos.x, io.MousePos.y, dp3.x, dp1.y, dp3.x, dp3.y) < 8.f)
                    /*&& localOverCurve == -1*/)
                {
                    localOverCurve = int(c);
                    overCurve = int(c);
                    overCurveOrPoint = true;
                }
            }
            else if (curveType == LerpType::BEZIER)
            {  // REF: by claude.ai
                // For Bezier, ptCount is keyframe count, actual vector has 3*ptCount points
                const size_t keyframeCount = ptCount;
                if (keyframeCount < 2) continue;

                // Draw bezier curve segments
                for (size_t seg = 0; seg < keyframeCount - 1; seg++)
                {
                    size_t idx0 = seg * 3;
                    ImVec2 p0 = pts[idx0 + 1];  // keyframe
                    ImVec2 p1 = pts[idx0 + 2];  // out-handle
                    ImVec2 p2 = pts[idx0 + 3];  // next in-handle
                    ImVec2 p3 = pts[idx0 + 4];  // next keyframe

                    const int subSteps = 32;
                    for (int step = 0; step < subSteps; step++)
                    {
                        float t1 = (float)step / (float)subSteps;
                        float t2 = (float)(step + 1) / (float)subSteps;

                        ImVec2 pt1 = CubicBezier(p0, p1, p2, p3, t1);
                        ImVec2 pt2 = CubicBezier(p0, p1, p2, p3, t2);

                        ImVec2 pos1 = pointToRange(pt1) * viewSize + offset;
                        ImVec2 pos2 = pointToRange(pt2) * viewSize + offset;

                        if (distance(io.MousePos.x, io.MousePos.y, pos1.x, pos1.y, pos2.x, pos2.y) < 8.f && !scrollingV)
                        {
                            localOverCurve = int(c);
                            overCurve = int(c);
                            overCurveOrPoint = true;
                        }

                        draw_list->AddLine(pos1, pos2, curveColor, 1.3f);
                    }
                }

                // Draw handles for each keyframe
                for (size_t k = 0; k < keyframeCount; k++)
                {
                    size_t idx = k * 3;
                    ImVec2 keyframe = pts[idx + 1];
                    ImVec2 keyframeRange = pointToRange(keyframe);

                    // Draw in-handle (if not first keyframe)
                    if (k > 0)
                    {
                        ImVec2 inHandle = pts[idx];
                        ImVec2 inHandleRange = pointToRange(inHandle);
                        bool isHandleSelected = handleSelection.find({int(c), int(idx)}) != handleSelection.end();

                        int handleState =
                            DrawBezierHandle(draw_list, inHandleRange, keyframeRange, viewSize, offset, true, isHandleSelected);
                        if (handleState && movingCurve == -1 && !selectingQuad)
                        {
                            overCurveOrPoint = true;
                            overHandle = true;
                            overCurve = -1;

                            if (handleState == 2)
                            {
                                if (!io.KeyShift) handleSelection.clear();
                                handleSelection.insert({int(c), int(idx)});
                            }
                        }
                    }

                    // Draw out-handle (if not last keyframe)
                    if (k < keyframeCount - 1)
                    {
                        ImVec2 outHandle = pts[idx + 2];
                        ImVec2 outHandleRange = pointToRange(outHandle);
                        bool isHandleSelected = handleSelection.find({int(c), int(idx + 2)}) != handleSelection.end();

                        int handleState = DrawBezierHandle(draw_list,
                                                           outHandleRange,
                                                           keyframeRange,
                                                           viewSize,
                                                           offset,
                                                           false,
                                                           isHandleSelected);
                        if (handleState && movingCurve == -1 && !selectingQuad)
                        {
                            overCurveOrPoint = true;
                            overHandle = true;
                            overCurve = -1;

                            if (handleState == 2)
                            {
                                if (!io.KeyShift) handleSelection.clear();
                                handleSelection.insert({int(c), int(idx + 2)});
                            }
                        }
                    }
                }
            }
        }  // point loop

        const bool isBezier = (curveType == LerpType::BEZIER);
        for (size_t p = 0; p < ptCount; p++)
        {
            const ImVec2 keyframe = isBezier ? pts[p * 3 + 1] : pts[p];

            const int drawState =
                DrawPoint(draw_list,
                          pointToRange(keyframe),
                          viewSize,
                          offset,
                          (selection.find({int(c), int(p)}) != selection.end() && movingCurve == -1 && !scrollingV));

            // Display point value near point
            {
                char point_val_text[512];
                const ImVec2 point_draw_pos = pointToRange(keyframe) * viewSize + offset;
                ImFormatString(point_val_text, IM_ARRAYSIZE(point_val_text), "%.0f|%.2f", keyframe.x, keyframe.y);
                draw_list->AddText({point_draw_pos.x - 4.0f, point_draw_pos.y + 7.0f}, 0xFFFFFFFF, point_val_text);
            }

            if (drawState && movingCurve == -1 && !selectingQuad)
            {
                overCurveOrPoint = true;
                overSelectedPoint = true;
                overCurve = -1;
                if (isBezier) overHandle = false;

                if (drawState == 2)
                {
                    if (!io.KeyShift && selection.find({int(c), int(p)}) == selection.end()) selection.clear();
                    selection.insert({int(c), int(p)});
                }
            }

            // Right-click on keyframe - open context menu
            if (drawState && io.MouseClicked[1])
            {
                rightClickedCurve = int(c);
                rightClickedKeyframe = int(p);
                ImGui::OpenPopup("KeyframeContextMenu");
            }
        }
    }  // curves loop

    if (localOverCurve == -1) overCurve = -1;

    // move selection
    static bool pointsMoved = false;
    static ImVec2 mousePosOrigin;
    static std::vector<ImVec2> originalPoints;

    if (overSelectedPoint && io.MouseDown[0])
    {
        if ((fabsf(io.MouseDelta.x) > 0.f || fabsf(io.MouseDelta.y) > 0.f) && !selection.empty())
        {
            if (!pointsMoved)
            {
                delegate.BeginEdit(0);
                mousePosOrigin = io.MousePos;
                originalPoints.resize(selection.size());
                int index = 0;
                for (auto& sel : selection)
                {  // REF: by claude.ai
                    const std::vector<ImVec2>& pts = delegate.GetCurvePointsList(sel.curveIndex);
                    LerpType lt = delegate.GetCurveLerpType(sel.curveIndex);
                    if (lt == LerpType::BEZIER)
                    {
                        // For Bezier, sel.pointIndex is keyframe index, get raw index
                        originalPoints[index++] = pts[sel.pointIndex * 3 + 1];
                    }
                    else
                    {
                        originalPoints[index++] = pts[sel.pointIndex];
                    }
                }
            }
            pointsMoved = true;
            ret = 1;

            auto prevSelection = selection;
            int originalIndex = 0;
            for (auto& sel : prevSelection)
            {
                const ImVec2 p =
                    rangeToPoint(pointToRange(originalPoints[originalIndex]) + (io.MousePos - mousePosOrigin) * sizeOfPixel);

                LerpType lt = delegate.GetCurveLerpType(sel.curveIndex);

                if (lt == LerpType::BEZIER)
                {
                    // Pass raw keyframe index - EditPoint handles the rest
                    int rawIdx = sel.pointIndex * 3 + 1;
                    delegate.EditPoint(sel.curveIndex, rawIdx, p);
                    // Bezier keyframes don't reorder, so no index update needed
                }
                else
                {
                    const int newIndex = delegate.EditPoint(sel.curveIndex, sel.pointIndex, p);
                    if (newIndex != sel.pointIndex)
                    {
                        selection.erase(sel);
                        selection.insert({sel.curveIndex, newIndex});
                    }
                }
                originalIndex++;
            }
        }
    }

    if (overSelectedPoint && !io.MouseDown[0])
    {
        overSelectedPoint = false;
        if (pointsMoved)
        {
            pointsMoved = false;
            delegate.EndEdit();
        }
    }

    // Move handle selection (Bezier handles)
    static bool handlesMoved = false;
    static ImVec2 handleMousePosOrigin;
    static std::vector<ImVec2> originalHandlePositions;

    if (overHandle && io.MouseDown[0] && !handleSelection.empty())
    {
        if ((fabsf(io.MouseDelta.x) > 0.f || fabsf(io.MouseDelta.y) > 0.f))
        {
            if (!handlesMoved)
            {
                delegate.BeginEdit(0);
                handleMousePosOrigin = io.MousePos;
                originalHandlePositions.resize(handleSelection.size());
                int index = 0;
                for (auto& sel : handleSelection)
                {
                    const std::vector<ImVec2>& pts = delegate.GetCurvePointsList(sel.curveIndex);
                    originalHandlePositions[index++] = pts[sel.pointIndex];
                }
            }
            handlesMoved = true;
            ret = 1;

            int originalIndex = 0;
            for (auto& sel : handleSelection)
            {
                ImVec2 newPos = rangeToPoint(pointToRange(originalHandlePositions[originalIndex]) +
                                             (io.MousePos - handleMousePosOrigin) * sizeOfPixel);

                // Delegate all constraint logic to EditPoint
                delegate.EditPoint(sel.curveIndex, sel.pointIndex, newPos);
                originalIndex++;
            }
        }
    }

    if (overHandle && !io.MouseDown[0])
    {
        overHandle = false;
        if (handlesMoved)
        {
            handlesMoved = false;
            delegate.EndEdit();
        }
    }

    // remove point
    if (overSelectedPoint && selection.size() == 1 && io.MouseDoubleClicked[0])
    {
        delegate.BeginEdit(selection.begin()->curveIndex);
        delegate.RemovePoint(selection.begin()->curveIndex, selection.begin()->pointIndex);
        selection.clear();
        overSelectedPoint = false;
        delegate.EndEdit();
    }

    // add point
    if (overCurve != -1 && io.MouseDoubleClicked[0])
    {
        const ImVec2 np = rangeToPoint((io.MousePos - offset) / viewSize);
        delegate.BeginEdit(overCurve);
        delegate.AddPoint(overCurve, np);
        delegate.EndEdit();
        ret = 1;
    }

    // move curve
    if (movingCurve != -1)
    {
        const size_t ptCount = delegate.GetCurvePointCount(movingCurve);
        const std::vector<ImVec2>& pts = delegate.GetCurvePointsList(movingCurve);
        if (!pointsMoved)
        {
            mousePosOrigin = io.MousePos;
            pointsMoved = true;
            originalPoints.resize(ptCount);
            for (size_t index = 0; index < ptCount; index++)
            {
                originalPoints[index] = pts[index];
            }
        }
        if (ptCount >= 1)
        {
            for (size_t p = 0; p < ptCount; p++)
            {
                delegate.EditPoint(
                    movingCurve,
                    int(p),
                    rangeToPoint(pointToRange(originalPoints[p]) + (io.MousePos - mousePosOrigin) * sizeOfPixel));
            }
            ret = 1;
        }
        if (!io.MouseDown[0])
        {
            movingCurve = -1;
            pointsMoved = false;
            delegate.EndEdit();
        }
    }
    if (movingCurve == -1 && overCurve != -1 && ImGui::IsMouseClicked(0) && selection.empty() && !selectingQuad)
    {
        movingCurve = overCurve;
        delegate.BeginEdit(overCurve);
    }

    // quad selection
    if (selectingQuad)
    {
        const ImVec2 bmin = ImMin(quadSelection, io.MousePos);
        const ImVec2 bmax = ImMax(quadSelection, io.MousePos);
        draw_list->AddRectFilled(bmin, bmax, 0x40FF0000, 1.f);
        draw_list->AddRect(bmin, bmax, 0xFFFF0000, 1.f);
        const ImRect selectionQuad(bmin, bmax);
        if (!io.MouseDown[0])
        {
            if (!io.KeyShift) selection.clear();
            // select everythnig is quad
            for (int c = 0; c < curveCount; c++)
            {
                if (!delegate.GetCurveVisibility(c)) continue;

                const int ptCount = delegate.GetCurvePointCount(c);
                if (ptCount < 1) continue;

                const std::vector<ImVec2>& pts = delegate.GetCurvePointsList(c);
                for (int p = 0; p < ptCount; p++)
                {
                    const ImVec2 center = pointToRange(pts[p]) * viewSize + offset;
                    if (selectionQuad.Contains(center)) selection.insert({int(c), int(p)});
                }
            }
            // done
            selectingQuad = false;
        }
    }
    if (!overCurveOrPoint && ImGui::IsMouseClicked(0) && !selectingQuad && movingCurve == -1 && !overSelectedPoint &&
        container.Contains(io.MousePos))
    {
        selectingQuad = true;
        quadSelection = io.MousePos;
    }
    if (clippingRect) draw_list->PopClipRect();

    // Bezier keyframe context menu
    if (ImGui::BeginPopup("KeyframeContextMenu"))
    {
        if (rightClickedCurve >= 0 && rightClickedKeyframe >= 0)
        {
            LerpType lt = delegate.GetCurveLerpType(rightClickedCurve);

            if (lt == LerpType::BEZIER)
            {
                if (ImGui::MenuItem("Reset Tangents"))
                {
                    delegate.BeginEdit(rightClickedCurve);
                    delegate.ResetTangentsForKeyframe(rightClickedCurve, rightClickedKeyframe);
                    delegate.EndEdit();
                }
                ImGui::Separator();
            }

            // General keyframe options (work for all curve types)
            int keyframeCount = delegate.GetCurvePointCount(rightClickedCurve);
            bool canDelete = rightClickedKeyframe > 0 && rightClickedKeyframe < keyframeCount - 1;

            if (ImGui::MenuItem("Delete Keyframe", nullptr, false, canDelete))
            {
                delegate.BeginEdit(rightClickedCurve);
                delegate.RemovePoint(rightClickedCurve, rightClickedKeyframe);
                delegate.EndEdit();
                selection.clear();
            }
        }

        ImGui::EndPopup();
    }
    else
    {
        // Reset when popup closes
        rightClickedCurve = -1;
        rightClickedKeyframe = -1;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    if (selectedPoints)
    {
        selectedPoints->resize(int(selection.size()));
        int index = 0;
        for (auto& point : selection) (*selectedPoints)[index++] = point;
    }
    _freea(curvesIndex);
    return ret;
}

ImVec2 PointToRange(const ImVec2& point, const ImVec2& min, const ImVec2& max) { return (point - min) / (max - min); }

ImVec2 SampleCurveForDrawing(const std::vector<ImVec2>& pts,
                             size_t ptCount,  // keyframe count for Bezier
                             float t,
                             LerpType curveType,
                             const ImVec2& min,
                             const ImVec2& max)
{
    t = ImClamp(t, 0.0f, 1.0f);

    if (curveType == LerpType::BEZIER)
    {
        // For Bezier: pts has 3*ptCount entries, ptCount is keyframe count
        if (ptCount < 2) return PointToRange(pts[1], min, max + ImVec2(1.f, 0.f));

        float segmentFloat = t * (ptCount - 1);
        size_t segmentIndex = (size_t)segmentFloat;

        if (segmentIndex >= ptCount - 1)
        {
            segmentIndex = ptCount - 2;
            segmentFloat = (float)(ptCount - 1);
        }

        float localT = segmentFloat - (float)segmentIndex;

        // Get the 4 control points for this segment
        // Keyframe i is at index 3*i+1, its out-handle at 3*i+2
        // Keyframe i+1 is at index 3*(i+1)+1, its in-handle at 3*(i+1)
        size_t idx0 = segmentIndex * 3;
        ImVec2 p0 = pts[idx0 + 1];  // keyframe i
        ImVec2 p1 = pts[idx0 + 2];  // keyframe i out-handle
        ImVec2 p2 = pts[idx0 + 3];  // keyframe i+1 in-handle
        ImVec2 p3 = pts[idx0 + 4];  // keyframe i+1

        ImVec2 result = CubicBezier(p0, p1, p2, p3, localT);
        return PointToRange(result, min, max + ImVec2(1.f, 0.f));
    }

    float segmentFloat = t * (ptCount - 1);
    size_t segmentIndex = (size_t)segmentFloat;

    if (segmentIndex >= ptCount - 1)
    {
        segmentIndex = ptCount - 2;
        segmentFloat = (float)(ptCount - 1);
    }

    const ImVec2 p1 = PointToRange(pts.at(segmentIndex), min, max + ImVec2(1.f, 0.f));
    const ImVec2 p2 = PointToRange(pts.at(segmentIndex + 1), min, max + ImVec2(1.f, 0.f));

    float localT = segmentFloat - (float)segmentIndex;

    if (curveType == LerpType::LINEAR)
    {
        return ImLerp(p1, p2, localT);
    }
    else if (curveType == LerpType::SMOOTH)
    {
        float smoothT = smoothstep(0.0f, 1.0f, localT);
        return ImVec2(ImLerp(p1.x, p2.x, localT), ImLerp(p1.y, p2.y, smoothT));
    }
    else
    {
        assert(0);  /// Unsupported LerpType
    }

    return p1;
}

// TODO(tanim) this should not get the CurveType, it should detect the curve between segments itself
float SampleCurveForAnimation(const std::vector<ImVec2>& pts,
                              float time,
                              LerpType curve_type,
                              RepresentationMeta /*representation_meta*/)
{
    if (curve_type == LerpType::BEZIER)
    {
        // For Bezier: pts has 3*keyframeCount entries
        // Keyframes are at indices 1, 4, 7, 10, ... (3*i + 1)
        const int keyframeCount = (int)pts.size() / 3;
        if (keyframeCount < 1) return 0.0f;

        // Find which segment contains this time
        for (int i = 0; i < keyframeCount - 1; i++)
        {
            ImVec2 key0 = pts[i * 3 + 1];
            ImVec2 key1 = pts[(i + 1) * 3 + 1];

            if (time >= key0.x && time <= key1.x)
            {
                ImVec2 p0 = key0;
                ImVec2 p1 = pts[i * 3 + 2];    // key0 out-handle
                ImVec2 p2 = pts[(i + 1) * 3];  // key1 in-handle
                ImVec2 p3 = key1;

                float t = FindTForX(p0, p1, p2, p3, time);
                ImVec2 result = CubicBezier(p0, p1, p2, p3, t);
                return result.y;
            }
        }

        // Before first keyframe
        if (time <= pts[1].x) return pts[1].y;
        // After last keyframe
        if (time >= pts[(keyframeCount - 1) * 3 + 1].x) return pts[(keyframeCount - 1) * 3 + 1].y;

        return pts[1].y;
    }

    const int ptCount = (int)pts.size();
    for (int i = 0; i < ptCount - 1; i++)
    {
        if (time >= pts.at(i).x && time <= pts.at(i + 1).x)
        {
            const float segmentT = (time - pts.at(i).x) / (pts.at(i + 1).x - pts.at(i).x);

            if (curve_type == LerpType::LINEAR)
            {
                return ImLerp(pts.at(i).y, pts.at(i + 1).y, segmentT);
            }
            else if (curve_type == LerpType::SMOOTH)
            {
                const float smoothT = smoothstep(0.0f, 1.0f, segmentT);
                return ImLerp(pts.at(i).y, pts.at(i + 1).y, smoothT);
            }
            else if (curve_type == LerpType::DISCRETE)
            {
                return pts.at(i).y;
            }
            else
            {
                assert(0);  /// Unsupported LerpType
            }
        }
    }

    if (time <= pts.at(0).x) return pts.at(0).y;
    if (time >= pts.at(ptCount - 1).x) return pts.at(ptCount - 1).y;

    return pts.at(0).y;
}

glm::quat SampleQuatForAnimation(Sequence& seq, float time)
{
    const auto& pts_w = seq.GetCurvePointsList(0);
    const auto& pts_x = seq.GetCurvePointsList(1);
    const auto& pts_y = seq.GetCurvePointsList(2);
    const auto& pts_z = seq.GetCurvePointsList(3);
    const auto& pts_spins = seq.GetCurvePointsList(4);
    const int pt_count = (int)pts_w.size();

    for (int i = 0; i < pt_count - 1; i++)
    {
        if (time >= pts_w.at(i).x && time <= pts_w.at(i + 1).x)
        {
            const float segment_t = (time - pts_w.at(i).x) / (pts_w.at(i + 1).x - pts_w.at(i).x);
            const auto q_a = glm::quat(pts_w.at(i).y, pts_x.at(i).y, pts_y.at(i).y, pts_z.at(i).y);
            const auto q_b = glm::quat(pts_w.at(i + 1).y, pts_x.at(i + 1).y, pts_y.at(i + 1).y, pts_z.at(i + 1).y);

            if (seq.GetCurveLerpType(0) == LerpType::LINEAR)
            {
                return glm::slerp(q_a, q_b, segment_t, (int)pts_spins.at(i + 1).y);
            }
            else if (seq.GetCurveLerpType(0) == LerpType::SMOOTH)
            {
                const float smoothT = smoothstep(0.0f, 1.0f, segment_t);
                return glm::slerp(q_a, q_b, smoothT, (int)pts_spins.at(i + 1).y);
            }
            else if (seq.GetCurveLerpType(0) == LerpType::DISCRETE)
            {
                return q_a;
            }
            else
            {
                assert(0);  /// Unsupported LerpType
            }
        }
    }

    const auto q_0 = glm::quat(pts_w.at(0).y, pts_x.at(0).y, pts_y.at(0).y, pts_z.at(0).y);
    if (time <= pts_w.at(0).x) return q_0;

    const auto q_last =
        glm::quat(pts_w.at(pt_count - 1).y, pts_x.at(pt_count - 1).y, pts_y.at(pt_count - 1).y, pts_z.at(pt_count - 1).y);
    if (time >= pts_w.at(pt_count - 1).x) return q_last;

    return q_0;
}

}  // namespace tanim::sequencer
