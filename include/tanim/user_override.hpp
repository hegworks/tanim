#pragma once

#include <entt/fwd.hpp>

namespace tanim
{

/// You must override this function somewhere in your application. Based on the data inside cdata, you have to return an
/// entt::entity that corresponds to the m_uid you had set in the EntityData during other API calls.
/// @param cdata The ComponentData of the entity that Tanim is looking for. You can use the data inside its m_user_data which
/// you could've filled before, to help you return the entity from the uid. For more info you can check the docs of
/// ComponentData.
/// @param uid_to_find The uid that Tanim is looking for. For more info check the docs of EntityData.
/// @return If you found the entt::entity based on the cdata & uid, return it. Otherwise, return entt::null, which will result
/// in the LogError being called from Tanim, and the animation related to the entity Tanim is looking for won't play.
entt::entity FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find);

/// You must override this function somewhere in your application. When Tanim wants to print an error, instead of doing it
/// directly, it calls this function with the message it wanted to print. You are free to do whatever you want with this. For
/// example, you can prepend something like "[TANIM]" to the message, then if you have a logging system in your application, you
/// can pass the message there; Or you can just std::cout, or printf the message as well.
/// @param message The message that Tanim wanted to report as an error.
void LogError(const std::string& message);

/// You must override this function somewhere in your application. When Tanim wants to print an info, instead of doing it
/// directly, it calls this function with the message it wanted to print. You are free to do whatever you want with this. For
/// example, you can prepend something like "[TANIM]" to the message, then if you have a logging system in your application, you
/// can pass the message there; Or you can just std::cout, or printf the message as well.
/// @param message The message that Tanim wanted to report as info.
void LogInfo(const std::string& message);

}  // namespace tanim
