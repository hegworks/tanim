#pragma once
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
