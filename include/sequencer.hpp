// REF (tanim): originally based on the imguizmo's ImCurveEdit.h:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/ImCurveEdit.h

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

#pragma once
#include "tanim/include/includes.hpp"

#include <stdint.h>
#include <vector>

struct ImRect;

namespace tanim::sequencer
{

enum class LerpType
{
    NONE,
    DISCRETE,
    LINEAR,
    SMOOTH,
    BEZIER,
};

struct EditPoint
{
    int curveIndex;
    int pointIndex;
    bool operator<(const EditPoint& other) const
    {
        if (curveIndex < other.curveIndex) return true;
        if (curveIndex > other.curveIndex) return false;

        if (pointIndex < other.pointIndex) return true;
        return false;
    }
};

struct SequenceInterface
{
    bool m_focused = false;
    virtual int CurveCount() = 0;
    virtual bool GetCurveVisibility(int curve_idx) = 0;
    virtual void SetCurveVisibility(int curve_idx, bool visibility) = 0;
    virtual LerpType GetCurveLerpType(int curve_idx) = 0;
    virtual ImVec2 GetMinPointValue() = 0;
    virtual ImVec2 GetMaxPointValue() = 0;
    virtual void SetMinPointValue(ImVec2 min) = 0;
    virtual void SetMaxPointValue(ImVec2 max) = 0;
    virtual int GetCurvePointCount(int curve_index) = 0;
    virtual uint32_t GetCurveColor(int curve_index) = 0;
    virtual const std::vector<ImVec2>& GetCurvePointsList(int curve_index) = 0;
    virtual int EditPoint(int curve_index, int point_index, ImVec2 value) = 0;
    virtual void AddPoint(int curve_index, ImVec2 value) = 0;
    virtual void RemovePoint(int curve_index, int point_index) = 0;
    virtual unsigned int GetBackgroundColor() = 0;
    // handle undo/redo through this functions
    virtual void BeginEdit(int curve_index) = 0;
    virtual void EndEdit() = 0;

    virtual ~SequenceInterface() = default;
};

int Edit(SequenceInterface& delegate,
         const ImVec2& size,
         unsigned int id,
         const ImRect* clippingRect = NULL,
         ImVector<EditPoint>* selectedPoints = NULL);

static ImVec2 PointToRange(const ImVec2& point, const ImVec2& min, const ImVec2& max);

ImVec2 SampleCurveForDrawing(const std::vector<ImVec2>& pts,
                             size_t ptCount,
                             float t,
                             LerpType curveType,
                             const ImVec2& min,
                             const ImVec2& max);

float SampleCurveForAnimation(const std::vector<ImVec2>& pts, float time, LerpType curveType);

}  // namespace tanim::sequencer
