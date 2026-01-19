#pragma once
#include "tanim/include/registry.hpp"
#include "tanim/include/user_data.hpp"

namespace tanim
{

void Init();

void Draw();
void UpdateEditor(float dt);
void OpenForEditing(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata);
void CloseEditor();

void StartTimeline(const TimelineData& tdata, ComponentData& cdata);
void UpdateTimeline(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata,
                    float delta_time);
void StopTimeline(ComponentData& cdata);

bool IsPlaying(const ComponentData& cdata);
void Play(ComponentData& cdata);
void Pause(ComponentData& cdata);
void Stop(ComponentData& cdata);

[[nodiscard]] std::string Serialize(TimelineData& tdata);
void Deserialize(TimelineData& data, const std::string& serialized_string);

void EnterPlayMode();
void ExitPlayMode();

template <typename T>
void RegisterComponent()
{
    internal::GetRegistry().RegisterComponent<T>();
}

}  // namespace tanim
