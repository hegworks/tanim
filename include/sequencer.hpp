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

struct ImRect;

namespace tanim
{
struct Sequence;
}

namespace tanim::sequencer
{

enum class HandleState : uint8_t
{
    NONE,
    HOVERED,
    CLICKED
};

struct EditPoint
{
    int m_curve_index;
    int m_keyframe_index;
    bool operator<(const EditPoint& other) const
    {
        if (m_curve_index < other.m_curve_index) return true;
        if (m_curve_index > other.m_curve_index) return false;

        if (m_keyframe_index < other.m_keyframe_index) return true;
        return false;
    }
};

int Edit(Sequence& seq,
         const ImVec2& size,
         unsigned int id,
         const ImRect* clipping_rect = nullptr,
         ImVector<EditPoint>* selected_points = nullptr);

glm::quat SampleQuatForAnimation(Sequence& seq, float time);

}  // namespace tanim::sequencer
