#pragma once
#include "tanim/include/helpers.hpp"
#include "tanim/include/sequence.hpp"

namespace tanim
{

struct TimelineData
{
    int m_first_frame{0};
    int m_last_frame{10};
    int m_min_frame{0};
    int m_max_frame{500};
    std::string m_name{"New Timeline"};
    std::vector<Sequence> m_sequences{};
    bool m_play_immediately{true};
    int m_player_samples{60};  // SamplesPerSecond
    PlaybackType m_playback_type{PlaybackType::LOOP};
    bool m_focused{false};
    bool m_expanded{true};
    int m_selected_sequence{-1};

    TimelineData() : m_sequences({}) {}

    TimelineData(int first_frame,
                 int last_frame,
                 int min_frame,
                 int max_frame,
                 const std::string& name,
                 const std::vector<tanim::Sequence>& sequences)
        : m_first_frame(first_frame),
          m_last_frame(last_frame),
          m_min_frame(min_frame),
          m_max_frame(max_frame),
          m_name(name),
          m_sequences(sequences)
    {
    }
};

struct ComponentData
{
    entt::entity m_root_entity{entt::null};
    float m_player_time{0};
    bool m_player_playing{false};
};

}  // namespace tanim
