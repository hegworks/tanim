#pragma once

#include "enums.hpp"

#include <any>
#include <string>
#include <vector>

namespace tanim
{

namespace internal
{
class Timeline;
}

/// Data related to entt::entity
struct EntityData
{
    /// A Unique Identifier. Anything that you can later retrieve the entt::entity from within your entt::registry, in your
    /// implementation of tanim::FindEntityOfUID function. For example the name of your entities.
    std::string m_uid{};

    /// Decorative. Anything you want to be displayed in the timeline editor instead of the m_uid. Useful when you set arbitrary
    /// numbers or other meaningless values as the m_uid. Can also be the same value as m_uid.
    std::string m_display{};
};

/// Serializable data for a timeline. Fully handled by Tanim. Shouldn't be directly edited by user. But user is responsible for
/// keeping this data in the memory and passing it to Tanim in the API calls that need it. For example, can be wrapped as a
/// resource that multiple entities can reference in their Tanim components.
///
/// Serialize TimelineData by calling tanim::Serialize which returns a string that
/// can be passed to tanim::Deserialize to deserialize the data from the string back into TimelineData.
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

/// Data unique to each entity that has a TimelineData. Fully handled by Tanim. But user is responsible for keeping this data
/// in the memory and passing it to Tanim in the API calls that need it. For example, can be put in the application's Tanim
/// component struct, along with the TimelineData resource, then attached to entities. m_user_data can be used to store any
/// other data that the user wants to store in that component. Since this struct is passed to the tanim::FindEntityOfUID by
/// Tanim, the user can store any data that can help them retrieve the entt::entity from the m_uid; For example the entt::entity
/// that has this component, and a cached list of all the entities that are or can be animated.
struct ComponentData
{
    /// can be used to store any other data that the user wants to store. check ComponentData docs for more info.
    std::any m_user_data{};

private:
    friend class internal::Timeline;
    float m_player_time{0};
    bool m_player_playing{false};
};

}  // namespace tanim
