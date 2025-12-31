// REF (tanim): originally based on the imguizmo's ImSequencer.h:
// https://github.com/CedricGuillemet/ImGuizmo/blob/71f14292205c3317122b39627ed98efce137086a/ImSequencer.h

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

namespace tanim
{
struct TimelineData;
}

struct ImDrawList;
struct ImRect;

namespace tanim::timeliner
{

enum TimelineEditorFlags
{
    TIMELINER_NONE = 0,
    TIMELINER_EDIT_STARTEND = 1 << 1,
    TIMELINER_CHANGE_FRAME = 1 << 3,
    TIMELINER_ADD_SEQUENCE = 1 << 4,
    TIMELINER_DELETE_SEQUENCE = 1 << 5,
    TIMELINER_COPYPASTE = 1 << 6,
    TIMELINER_ALL = TIMELINER_EDIT_STARTEND | TIMELINER_CHANGE_FRAME | TIMELINER_ADD_SEQUENCE | TIMELINER_DELETE_SEQUENCE |
                    TIMELINER_COPYPASTE,
};

struct TimelineInterface
{
    virtual ~TimelineInterface() = default;
};

// return true if selection is made
bool Timeliner(TimelineData& timeline_data,
               int* current_frame,
               bool* expanded,
               int* selected_sequence,
               int* first_frame,
               int timeliner_flags);

}  // namespace tanim::timeliner
