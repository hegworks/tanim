#pragma once

#include <entt/fwd.hpp>

#include <vector>

namespace tanim
{
struct ComponentData;
struct TimelineData;
struct EntityData;
struct Sequence;
struct RegisteredComponent;
}  // namespace tanim

namespace tanim::internal
{

const RegisteredComponent* FindMatchingComponent(const Sequence& seq, const std::vector<EntityData>& entity_datas);
void SetEditorTimelinePlayerFrame(int frame_num);
void Sample(entt::registry& registry, const std::vector<EntityData>& entity_datas, TimelineData& tdata, ComponentData& cdata);

}  // namespace tanim::internal
