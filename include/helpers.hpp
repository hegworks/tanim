#pragma once

#include "tanim/include/includes.hpp"

#include <cassert>
#include <cmath>

namespace tanim::helpers
{

inline int SecondsToFrame(float time, int samples) { return static_cast<int>(std::floorf(time * static_cast<float>(samples))); }

inline float SecondsToSampleTime(float time, int samples) { return time * static_cast<float>(samples); }

inline float FrameToSeconds(int frame, int samples)
{
    assert(samples > 0);
    return static_cast<float>(frame) * (1.0f / static_cast<float>(samples));
}

template <typename EnumType>
static bool InspectEnum(EnumType& enum_, const std::vector<EnumType>& exclusions = {}, const std::string& custom_name = {})
{
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
    return changed;
}

}  // namespace tanim::helpers
