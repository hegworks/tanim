#pragma once

#include <entt/fwd.hpp>

namespace tanim
{

entt::entity FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find);
void LogError(const std::string& message);
void LogInfo(const std::string& message);

}  // namespace tanim
