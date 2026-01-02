#pragma once

#include "tanim/include/includes.hpp"

#include <cassert>

namespace tanim
{
#ifndef IMGUI_DEFINE_MATH_OPERATORS
inline ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }

inline ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x - b.x, a.y - b.y); }

inline ImVec2 operator*(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x * b.x, a.y * b.y); }

inline ImVec2 operator/(const ImVec2& a, const ImVec2& b) { return ImVec2(a.x / b.x, a.y / b.y); }

inline ImVec2 operator*(const ImVec2& a, const float b) { return ImVec2(a.x * b, a.y * b); }
#endif
}  // namespace tanim

namespace tanim::helpers
{

inline int SecondsToFrame(float time, int samples) { return static_cast<int>(std::floorf(time * static_cast<float>(samples))); }

inline float SecondsToSampleTime(float time, int samples) { return time * static_cast<float>(samples); }

inline float FrameToSeconds(int frame, int samples)
{
    assert(samples > 0);
    return static_cast<float>(frame) * (1.0f / static_cast<float>(samples));
}

inline std::string MakeFullName(const std::string& uid, const std::string& struct_name, const std::string& field_name)
{
    return uid + "::" + struct_name + "::" + field_name;
}

inline std::string MakeFullName(const std::string& uid, const std::string& struct_field_name)
{
    return uid + "::" + struct_field_name;
}

inline std::string MakeStructFieldName(const std::string& struct_name, const std::string& field_name)
{
    return struct_name + "::" + field_name;
}

template <typename EnumType>
static bool InspectEnum(EnumType& enum_, const std::vector<EnumType>& exclusions = {}, const std::string& custom_name = {})
{
    ImGui::PushID(&enum_);

    bool changed = false;
    const std::string type_name = std::string(magic_enum::enum_type_name<EnumType>());
    auto current_name = magic_enum::enum_name(enum_);
    const std::string preview = current_name.empty() ? "Unknown" : std::string(current_name);
    const std::string dropdown_name = custom_name.empty() ? type_name : custom_name;

    ImGui::PushItemWidth(150.0f);
    if (ImGui::BeginCombo(dropdown_name.c_str(), preview.c_str()))
    {
        constexpr auto enum_values = magic_enum::enum_values<EnumType>();
        for (const auto& enum_value : enum_values)
        {
            if (std::find(exclusions.begin(), exclusions.end(), enum_value) != exclusions.end())
            {
                continue;
            }

            auto enum_name = magic_enum::enum_name(enum_value);
            const bool is_selected = (enum_ == enum_value);

            if (ImGui::Selectable(std::string(enum_name).c_str(), is_selected))
            {
                enum_ = enum_value;
                changed = true;
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopID();

    return changed;
}

/// <param name="i_f">input from</param>
/// <param name="i_t">input to</param>
/// <param name="o_f">output from</param>
/// <param name="o_t">output to</param>
/// <param name="v">vale</param>
/// <returns>v mapped from the range [i_f,i_t] to [o_f,o_t]</returns>
template <typename T>
T Remap(T i_f, T i_t, T o_f, T o_t, T v)
{
    return static_cast<T>(static_cast<float>(v - i_f) / static_cast<float>(i_t - i_f) * static_cast<float>(o_t - o_f) + o_f);
}

}  // namespace tanim::helpers
