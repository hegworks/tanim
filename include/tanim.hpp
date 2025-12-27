#pragma once
#include "registry.hpp"
#include "tanim/include/timeline.hpp"
#include "tanim/include/entity_data.hpp"

namespace tanim
{

class Tanim
{
public:
    Tanim() = default;

    static void Init();
    static void Draw();
    static void UpdateEditor(float dt);
    static void OpenForEditing(entt::registry& registry,
                               const std::vector<EntityData>& entity_datas,
                               TimelineData& timeline_data,
                               ComponentData& component_data);
    static void CloseEditor();
    static inline float m_snap_y_value = 0.1f;

    static void StartTimeline(const TimelineData& timeline_data, ComponentData& component_data);

    static void UpdateTimeline(entt::registry& registry,
                               const std::vector<EntityData>& entity_datas,
                               TimelineData& timeline_data,
                               ComponentData& component_data,
                               float delta_time);

    static void StopTimeline(ComponentData& component_data);

    static bool IsPlaying(const ComponentData& component_data);
    static void Play(ComponentData& component_data);
    static void Pause(ComponentData& component_data);
    static void Stop(ComponentData& component_data);

    static std::optional<std::reference_wrapper<const RegisteredComponent>> FindMatchingComponent(
        const Sequence& seq,
        const std::vector<EntityData>& entity_datas);

    [[nodiscard]] static std::string Serialize(TimelineData& data);
    static void Deserialize(entt::entity root_entity, TimelineData& data, const std::string& serialized_string);

    static void EnterPlayMode() { m_is_engine_in_play_mode = true; }
    static void ExitPlayMode() { m_is_engine_in_play_mode = false; }

private:
    static inline TimelineData* m_editor_timeline_data{nullptr};
    static inline ComponentData* m_editor_component_data{nullptr};
    static inline std::vector<EntityData> m_editor_entity_datas{};
    static inline entt::registry* m_editor_registry{nullptr};

    static inline bool m_is_engine_in_play_mode{};
    static inline bool m_preview{true};

    static void Sample(entt::registry& registry,
                       const std::vector<EntityData>& entity_datas,
                       TimelineData& tdata,
                       ComponentData& cdata);
};

}  // namespace tanim
