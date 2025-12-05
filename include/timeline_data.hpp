#pragma once
#include "tanim/include/helpers.hpp"
#include "tanim/include/sequence.hpp"

namespace tanim
{

struct TimelineData
{
    entt::entity m_entity{entt::null};
    int m_first_frame{0};
    int m_last_frame{10};
    int m_min_frame{0};
    int m_max_frame{500};
    std::string m_name{"New Timeline"};
    std::vector<Sequence> m_sequences{};
    uint64_t m_id{0};
    bool m_play_immediately{true};
    int m_player_samples{60};  // SamplesPerSecond
    float m_player_time{0};
    bool m_player_playing{false};

    int PlayerFrame() const { return helpers::SecondsToFrame(m_player_time, m_player_samples); }
    float PlayerSampleTime() const { return helpers::SecondsToSampleTime(m_player_time, m_player_samples); }
    float LastFrameTime() const { return helpers::FrameToSeconds(m_last_frame, m_player_samples); }
    void SetPlayerTimeFromFrame(int frame_num) { m_player_time = helpers::FrameToSeconds(frame_num, m_player_samples); }
    void SetPlayerTimeFromSeconds(float time) { m_player_time = time; }

    TimelineData(entt::entity entity) : m_entity(entity), m_sequences({}) {}

    TimelineData(entt::entity entity,
                 int first_frame,
                 int last_frame,
                 int min_frame,
                 int max_frame,
                 const std::string& name,
                 const std::vector<tanim::Sequence>& sequences)
        : m_entity(entity),
          m_first_frame(first_frame),
          m_last_frame(last_frame),
          m_min_frame(min_frame),
          m_max_frame(max_frame),
          m_name(name),
          m_sequences(sequences)
    {
    }
};

}  // namespace tanim
