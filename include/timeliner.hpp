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

#include <cstddef>

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
    bool focused = false;
    virtual int GetFirstFrame() const = 0;
    virtual int GetLastFrame() const = 0;
    virtual int GetSequenceCount() const = 0;

    virtual void BeginEdit(int /*index*/) {}
    virtual void EndEdit() {}
    virtual int GetSequenceTypeCount() const { return 0; }
    virtual const char* GetSequenceTypeName(int /*typeIndex*/) const { return ""; }
    virtual const char* GetSequenceLabel(int /*index*/) const { return ""; }
    virtual const char* GetCollapseFmt() const { return "%d Frames / %d entries"; }

    virtual void MultiGet(int index, int** start, int** end, int* type, unsigned int* color) = 0;
    virtual void AddSequence(int /*type*/) {}
    virtual void DeleteSequence(int /*index*/) {}
    virtual void Duplicate(int /*index*/) {}

    virtual void EditFirstFrame(int newStart) = 0;
    virtual void EditLastFrame(int newEnd) = 0;

    virtual void Copy() {}
    virtual void Paste() {}

    virtual size_t GetCustomHeight(int /*index*/) { return 0; }

    virtual void DoubleClick(int /*index*/) {}

    virtual void CustomDraw(int /*index*/,
                            ImDrawList* /*draw_list*/,
                            const ImRect& /*rc*/,
                            const ImRect& /*legendRect*/,
                            const ImRect& /*clippingRect*/,
                            const ImRect& /*legendClippingRect*/)
    {
    }

    virtual void CustomDrawCompact(int /*index*/,
                                   ImDrawList* /*draw_list*/,
                                   const ImRect& /*rc*/,
                                   const ImRect& /*clippingRect*/)
    {
    }

    virtual ~TimelineInterface() = default;
};

// return true if selection is made
bool Timeliner(TimelineInterface* timeline,
               int* current_frame,
               bool* expanded,
               int* selected_sequence,
               int* first_frame,
               int timeliner_flags);

}  // namespace tanim::timeliner
