# User Overrides

Three functions you must implement in your project for Tanim to integrate with your engine.

## Overview

Tanim requires you to override three functions in your codebase. These functions allow Tanim to communicate with your engine without directly depending on its implementation:

- **FindEntityOfUID**: Converts a UID string back to an `entt::entity`
- **LogError**: Reports errors through your logging system
- **LogInfo**: Reports informational messages through your logging system

These functions must be implemented in a `.cpp` file in your project, not in Tanim's code.

---

## FindEntityOfUID

```cpp
entt::entity FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find);
```

### Purpose

Converts a UID string (from `EntityData::m_uid`) back to the actual `entt::entity` that it represents. This is called frequently during animation playback and editing.

### Parameters

- `cdata`: The `ComponentData` for the entity being animated. You can use `cdata.m_user_data` to store information that helps you find entities efficiently.
- `uid_to_find`: The UID string that needs to be converted to an `entt::entity`.

### Return Value

- Return the `entt::entity` that corresponds to `uid_to_find`
- Return `entt::null` if the entity cannot be found

### When Called

This function is called:

- During animation playback (every frame per sequence)
- When creating sequences in the editor
- When recording keyframes
- When inspecting animated values

**Performance Note**: This function is called frequently, so make it as efficient as possible. Use `ComponentData::m_user_data` to cache entity lookups.

### Implementation Patterns

#### Pattern 1: Simple Name-Based Lookup

```cpp
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata,
                                   const std::string& uid_to_find)
{
    // Extract root entity from user data
    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data is empty when finding uid: " + uid_to_find);
        return entt::null;
    }

    entt::entity root = std::any_cast<entt::entity>(cdata.m_user_data);

    // Check if root matches
    if (GetEntityName(root) == uid_to_find)
        return root;

    // Search children recursively
    for (entt::entity child : GetAllChildrenRecursive(root))
    {
        if (GetEntityName(child) == uid_to_find)
            return child;
    }

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

#### Pattern 2: Cached Entity List (Recommended)

```cpp
struct TanimUserData
{
    entt::entity root_entity;
    std::vector<entt::entity> cached_entities;
};

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata,
                                   const std::string& uid_to_find)
{
    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data is empty when finding uid: " + uid_to_find);
        return entt::null;
    }

    auto* user_data = std::any_cast<TanimUserData>(&cdata.m_user_data);
    if (!user_data)
    {
        LogError("m_user_data has wrong type when finding uid: " + uid_to_find);
        return entt::null;
    }

    // Check root
    if (GetEntityName(user_data->root_entity) == uid_to_find)
        return user_data->root_entity;

    // Check cached children (fast linear search)
    for (entt::entity entity : user_data->cached_entities)
    {
        if (GetEntityName(entity) == uid_to_find)
            return entity;
    }

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

#### Pattern 3: Hash Map for Large Hierarchies

```cpp
struct TanimUserData
{
    entt::entity root_entity;
    std::unordered_map<std::string, entt::entity> uid_to_entity;
};

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata,
                                   const std::string& uid_to_find)
{
    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data is empty when finding uid: " + uid_to_find);
        return entt::null;
    }

    auto* user_data = std::any_cast<TanimUserData>(&cdata.m_user_data);
    if (!user_data)
    {
        LogError("m_user_data has wrong type when finding uid: " + uid_to_find);
        return entt::null;
    }

    // O(1) hash map lookup
    auto it = user_data->uid_to_entity.find(uid_to_find);
    if (it != user_data->uid_to_entity.end())
        return it->second;

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

### Setting Up User Data

You should populate `ComponentData::m_user_data` before calling `OpenForEditing` or `UpdateTimeline`:

```cpp
void PrepareAnimationComponent(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    // Build user data
    TanimUserData user_data;
    user_data.root_entity = entity;

    // Cache all children
    for (entt::entity child : GetAllChildrenRecursive(entity))
    {
        user_data.cached_entities.push_back(child);
    }

    // Store in component data
    anim.component_data.m_user_data = user_data;
}
```

### Error Handling

**Always return `entt::null` on failure**: Never throw exceptions or crash. Tanim will log the error you report and skip animation for that sequence.

**Log helpful error messages**: Include the UID in error messages to help users debug. Use your `LogError` implementation to report issues.

**Common Errors**:

- `m_user_data` is empty or has wrong type
- Entity with the given UID doesn't exist
- Entity was deleted but animation still references it

### Example from Real Implementation

```cpp
namespace MyEngine
{
struct TanimUserData
{
    entt::entity m_root_entity{entt::null};
    std::vector<tanim::EntityData> m_cached_entities_data;
    std::vector<entt::entity> m_cached_entities;
};
}  // namespace MyEngine

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    if (!cdata.m_user_data.has_value())
    {
        MyEngine::Log::Error("m_user_data had no value when finding uid " + uid_to_find);
        return entt::null;
    }

    const auto* user_data = std::any_cast<MyEngine::TanimUserData>(&cdata.m_user_data);
    if (!user_data)
    {
        MyEngine::Log::Error("m_user_data could not cast to MyEngine::TanimUserData when finding uid " + uid_to_find);
        return entt::null;
    }

    if (user_data->m_root_entity == entt::null)
    {
        MyEngine::Log::Error("m_root_entity in m_user_data was entt::null when finding uid " + uid_to_find);
        return entt::null;
    }

    const std::string root_entity_uid = MyEngine::EntityToTanimUID(user_data->m_root_entity);
    if (root_entity_uid == uid_to_find)
    {
        return user_data->m_root_entity;
    }

    for (const auto& child_entity : user_data->m_cached_entities)
    {
        if (MyEngine::EntityToTanimUID(child_entity) == uid_to_find)
        {
            return child_entity;
        }
    }

    MyEngine::Log::Error("no entity was found when finding uid " + uid_to_find);
    return entt::null;
}
```

---

## LogError

```cpp
void LogError(const std::string& message);
```

### Purpose

Reports error messages from Tanim through your engine's logging system. This allows you to handle Tanim errors consistently with the rest of your application.

### Parameters

- `message`: The error message from Tanim

### Implementation Examples

#### Simple Console Output

```cpp
void tanim::LogError(const std::string& message)
{
    std::cerr << "[TANIM ERROR] " << message << std::endl;
}
```

#### Custom Logging System

```cpp
void tanim::LogError(const std::string& message)
{
    MyEngine::Logger::Error("[TANIM] " + message);
}
```

#### With Color/Formatting

```cpp
void tanim::LogError(const std::string& message)
{
    MyEngine::Logger::Log(LogLevel::Error, "TANIM", message);
}
```

### Common Error Messages

You'll see errors like:

- `"FindEntityOfUID with the uid of [uid] returned entt::null"` - Entity not found during lookup
- `"entity [id] does not have a component named [ComponentName]"` - Component missing on entity
- `"Couldn't find any entity with matching details: [sequence_name]"` - Sequence target not found
- `"Versions prior to 2 are not supported. Can not deserialize."` - Old serialization format

### Best Practices

**Add a prefix**: Prepend `[TANIM]` or similar to make Tanim errors easy to identify in logs.

**Use your existing system**: Route errors through your engine's logging system for consistency.

**Don't filter errors**: Log all errors from Tanim. They indicate real problems that need attention.

**Consider log levels**: If your logging system has levels, use `Error` or `Critical` for these messages.

---

## LogInfo

```cpp
void LogInfo(const std::string& message);
```

### Purpose

Reports informational messages from Tanim through your engine's logging system. These are non-critical messages about Tanim's state or operations.

### Parameters

- `message`: The informational message from Tanim

### Implementation Examples

#### Simple Console Output

```cpp
void tanim::LogInfo(const std::string& message)
{
    std::cout << "[TANIM INFO] " << message << std::endl;
}
```

#### Custom Logging System

```cpp
void tanim::LogInfo(const std::string& message)
{
    MyEngine::Logger::Info("[TANIM] " + message);
}
```

#### With Color/Formatting

```cpp
void tanim::LogInfo(const std::string& message)
{
    MyEngine::Logger::Log(LogLevel::Info, "TANIM", message);
}
```

### Usage

Info messages are less common than errors. They might include:

- Initialization status
- Feature usage notes
- Performance warnings

### Best Practices

**Add a prefix**: Prepend `[TANIM]` to identify Tanim messages.

**Use your existing system**: Route through your engine's logging system.

**Consider filtering**: Info messages can be disabled in release builds if desired.

---

## Implementation Checklist

- [ ] Implement `FindEntityOfUID` in a `.cpp` file
- [ ] Implement `LogError` in the same file
- [ ] Implement `LogInfo` in the same file
- [ ] All three functions must be in the `tanim` namespace
- [ ] Test entity lookup works correctly
- [ ] Verify error messages appear in your logs
- [ ] Populate `ComponentData::m_user_data` before using Tanim

## File Structure

```cpp
// YourEngine_TanimIntegration.cpp

#include <tanim/tanim.hpp>
#include "YourEngine/Logger.h"
#include "YourEngine/EntitySystem.h"

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    // Your implementation
}

void tanim::LogError(const std::string& message)
{
    YourEngine::Logger::Error("[TANIM] " + message);
}

void tanim::LogInfo(const std::string& message)
{
    YourEngine::Logger::Info("[TANIM] " + message);
}
```

---

## Next Steps

- See [Example Implementation](../example-implementation.md) for complete working examples
- Review [Data Structures](data-structures.md) for `ComponentData::m_user_data` patterns
- Check [Lifecycle Functions](lifecycle.md) for when these overrides are called
