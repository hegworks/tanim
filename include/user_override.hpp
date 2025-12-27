#pragma once

#include "tanim/include/includes.hpp"

namespace tanim
{

entt::entity GetEntityOfUID(const std::string& uid);
entt::entity GetNestedEntityOfUID(entt::entity root_entity, const std::string& to_find_uid);

}  // namespace tanim
