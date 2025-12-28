#pragma once

#include "tanim/include/includes.hpp"

namespace tanim
{

std::optional<entt::entity> FindEntityOfUID(entt::entity root_entity, const std::string& uid_to_find);
void LogError(const std::string& message);
void LogInfo(const std::string& message);

}  // namespace tanim
