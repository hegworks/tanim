#pragma once
#include "tanim/include/registry.hpp"
#include "tanim/include/timeline.hpp"
#include "tanim/include/entity_data.hpp"

namespace tanim
{

void Init();

void Draw();
void UpdateEditor(float dt);
void OpenForEditing(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& timeline_data,
                    ComponentData& component_data);
void CloseEditor();

void StartTimeline(const TimelineData& timeline_data, ComponentData& component_data);
void UpdateTimeline(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& timeline_data,
                    ComponentData& component_data,
                    float delta_time);
void StopTimeline(ComponentData& component_data);

bool IsPlaying(const ComponentData& component_data);
void Play(ComponentData& component_data);
void Pause(ComponentData& component_data);
void Stop(ComponentData& component_data);

[[nodiscard]] std::string Serialize(TimelineData& tdata);
void Deserialize(TimelineData& data, const std::string& serialized_string);

void EnterPlayMode();
void ExitPlayMode();

}  // namespace tanim
