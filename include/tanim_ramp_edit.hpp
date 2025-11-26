#pragma once
#include "tanimguizmo/tanimguizmo_curve_edit.h"

#include <algorithm>
#include <iterator>

namespace tanim
{

struct TanimRampEdit : public tanimguizmo_curve_edit::Delegate
{
    TanimRampEdit()
    {
        mPts[0][0] = ImVec2(0.f, 0.0f);
        mPts[0][1] = ImVec2(10.f, 0.0f);
        mPointCount[0] = 2;

        mPts[1][0] = ImVec2(0.f, 0.0f);
        mPts[1][1] = ImVec2(10.f, 0.0f);
        mPointCount[1] = 2;

        mPts[2][0] = ImVec2(0.f, 0.0f);
        mPts[2][1] = ImVec2(10.f, 0.0f);
        mPointCount[2] = 2;
        mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
    }

    size_t GetCurveCount() override { return 3; }

    bool IsVisible(size_t curve_index) override { return mbVisible[curve_index]; }

    size_t GetPointCount(size_t curve_index) override { return mPointCount[curve_index]; }

    uint32_t GetCurveColor(size_t curveIndex) override
    {
        uint32_t cols[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
        return cols[curveIndex];
    }

    ImVec2* GetPoints(size_t curveIndex) override { return mPts[curveIndex]; }

    tanimguizmo_curve_edit::CurveType GetCurveType(size_t curveIndex) const override
    {
        IM_UNUSED(curveIndex);  // TODO(tanim)
        return tanimguizmo_curve_edit::CurveType::CurveSmooth;
    }

    int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override
    {
        // TanimAddition
        // snap the time (x) of keyframes to integers
        value.x = floorf(value.x);

        // TanimAddition
        // return early if the keyframes are in the same frame
        if (pointIndex + 1 < (int)GetPointCount(curveIndex) && (int)mPts[curveIndex][pointIndex + 1].x == (int)value.x)
        {
            return pointIndex;
        }
        if (pointIndex - 1 > 0 && (int)mPts[curveIndex][pointIndex - 1].x == (int)value.x)
        {
            return pointIndex;
        }

        mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
        SortValues(curveIndex);

        // TanimAddition
        // force the first keyframe time (x) to 0
        mPts[curveIndex][0].x = 0;
        // force the last keyframe time (x) to the sequence's last frame
        mPts[curveIndex][GetPointCount(curveIndex) - 1].x = (float)mSequenceFrameEnd;

        for (size_t i = 0; i < GetPointCount(curveIndex); i++)
        {
            if (mPts[curveIndex][i].x == value.x) return (int)i;
        }
        return pointIndex;
    }

    void AddPoint(size_t curveIndex, ImVec2 value) override
    {
        if (mPointCount[curveIndex] >= 8) return;
        mPts[curveIndex][mPointCount[curveIndex]++] = value;
        SortValues(curveIndex);
    }

    ImVec2& GetMax() override { return mMax; }

    ImVec2& GetMin() override { return mMin; }

    void SetMin(ImVec2 min) override { mMin = min; }

    void SetMax(ImVec2 max) override { mMax = max; }

    void BeginEdit(int) override { /*TODO(tanim)*/ }
    void EndEdit() override { /*TODO(tanim)*/ }

    unsigned int GetBackgroundColor() override { return 0; }

    void SequenceFrameEndEdit(int newFrameEnd)
    {
        mSequenceFrameEnd = newFrameEnd;
        ClampLastPointsToFrameEnd();
    }

    ImVec2 mPts[3][8];
    size_t mPointCount[3];
    bool mbVisible[3];
    ImVec2 mMin{0, -1.5f};
    ImVec2 mMax{500, 1.5f};
    int mSequenceFrameEnd{1};

private:
    void SortValues(size_t curveIndex)
    {
        auto b = std::begin(mPts[curveIndex]);
        auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
        std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
    }

    void ClampLastPointsToFrameEnd()
    {
        for (size_t i = 0; i < GetCurveCount(); i++)
        {
            mPts[i][GetPointCount(i) - 1].x = (float)mSequenceFrameEnd;
            SortValues(i);
        }
    }
};

}  // namespace tanim
