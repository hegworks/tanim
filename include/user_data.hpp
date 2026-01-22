#pragma once

#include "enums.hpp"
// #include "tanim/include/timeline.hpp"

#include <string>
#include <vector>

namespace tanim
{

namespace internal
{
class Timeline;
}

struct EntityData
{
    std::string m_uid{};      // unique identifier
    std::string m_display{};  // optional
};

struct TimelineData
{
    int m_first_frame{0};
    int m_last_frame{10};
    int m_min_frame{0};
    int m_max_frame{500};
    std::string m_name{"New Timeline"};
    std::vector<internal::Sequence> m_sequences{};
    bool m_play_immediately{true};
    int m_player_samples{60};  // SamplesPerSecond
    internal::PlaybackType m_playback_type{internal::PlaybackType::LOOP};
    bool m_focused{false};
    bool m_expanded{true};
    int m_selected_sequence{-1};
};

struct ComponentData
{
    std::any m_user_data{};

private:
    friend class internal::Timeline;
    float m_player_time{0};
    bool m_player_playing{false};
};

}  // namespace tanim
