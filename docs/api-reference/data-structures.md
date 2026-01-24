# Data Structures

Complete reference for Tanim's data structures that bridge your engine and the animation system.

## Overview

Tanim uses three main data structures:

- **EntityData**: Identifies entities that can be animated
- **TimelineData**: Stores animation curves, keyframes, and timeline settings
- **ComponentData**: Holds runtime playback state for each entity

These structures allow Tanim to remain independent of your engine's implementation while providing full animation functionality.

---

## EntityData

```cpp
struct EntityData
{
    std::string m_uid;
    std::string m_display;
};
```

### Purpose

Identifies an entity that can be animated in a timeline. You provide a vector of `EntityData` when opening a timeline for editing or updating it at runtime.

### Fields

#### m_uid

**Type**: `std::string`

**Purpose**: Unique identifier that your `FindEntityOfUID` function uses to retrieve the actual `entt::entity`.

**Usage**: Can be any string that uniquely identifies the entity within the animated hierarchy. The UID must persist across application sessions - it should not be something that changes when scenes are loaded or reloaded.

**Important Considerations**:

**Session Persistence**: The UID must remain the same between application sessions. For example, `entt::entity` IDs are not suitable because they can change when a scene is loaded. The same scene with the same entities might have different `entt::entity` IDs after reloading, which would break animation references.

**Reusability vs Uniqueness**: The UID scheme affects animation reusability:
- **Universally unique IDs (UUIDs)**: If you use UUIDs that are unique across the entire scene, you lose the ability to reuse the same `TimelineData` on multiple entities with identical hierarchies. Even if two entities have the same structure, their unique UUIDs will prevent the animation from playing on both.
- **Name-based IDs**: Using entity names or hierarchical names (like Unity and Godot do) allows animation reuse across similar hierarchies. For example, multiple characters with the same bone structure can share one walk cycle animation if they use consistent naming.

**Recommended Approach**: Entity names or hierarchical names are common and practical. Ensure you have proper error handling for cases where names aren't found or are duplicated. See the [Example Implementation](../example-implementation.md) for a complete implementation.

**Common approaches**:
- Entity name (simple but requires unique names)
- Stringified UUID (unique but less readable, prevents animation reuse)
- Custom identifier system from your engine

**Example**:
```cpp
// Using entity names (allows animation reuse)
m_uid = "PlayerHead"

// Using UUIDs (unique but prevents reuse)
m_uid = "123e4567-e89b-12d3-a456-426614174000"

// Using custom IDs
m_uid = "entity_42"
```

**Important**: The UID must be unique within the entity hierarchy being animated. Two entities with the same UID will cause ambiguous behavior. Consider caching the `entity_datas` vector and rebuilding only when the hierarchy changes, and store the cached list in `ComponentData::m_user_data` so both `OpenForEditing` and `UpdateTimeline` can use it.

#### m_display

**Type**: `std::string`

**Purpose**: User-friendly name shown in the Tanim editor when creating sequences.

**Usage**: Can be the same as `m_uid` or a more descriptive name. Useful when `m_uid` uses non-human-readable identifiers like UUIDs or numeric IDs.

**Example**:
```cpp
// Same as UID (when UID is readable)
EntityData{ .m_uid = "Player", .m_display = "Player" }

// More descriptive (when UID is not readable)
EntityData{ .m_uid = "123456789", .m_display = "Player Head" }

// Hierarchical display names for better readability
EntityData{ .m_uid = "head_bone", .m_display = "Player > Head > Bone" }
```

**Best Practice**: Make `m_display` readable and descriptive for better editor UX, especially when `m_uid` uses non-human-readable identifiers.

### Usage Example

```cpp
std::vector<tanim::EntityData> BuildEntityList(entt::entity root)
{
    std::vector<tanim::EntityData> entity_datas;
    
    // Add root entity
    entity_datas.push_back({
        .m_uid = GetEntityUID(root),
        .m_display = GetEntityName(root)
    });
    
    // Add all children recursively
    for (entt::entity child : GetAllChildren(root))
    {
        entity_datas.push_back({
            .m_uid = GetEntityUID(child),
            .m_display = GetEntityName(child)
        });
    }
    
    return entity_datas;
}

// Use in OpenForEditing and UpdateTimeline
auto entity_list = BuildEntityList(animated_entity);
tanim::OpenForEditing(registry, entity_list, tdata, cdata);
```

---

## TimelineData

```cpp
struct TimelineData
{
    // All fields handled internally by Tanim
};
```

### Purpose

Stores all animation data for a timeline, including sequences, curves, keyframes, and editor settings. This is the data that gets serialized when saving animations.

### Lifetime

- Created when you create a new animation component
- Persists as long as you want to keep the animation data
- Can be serialized to disk and loaded back
- Can be shared across multiple entities

### Memory Management

**Sharing**: Multiple entities can reference the same `TimelineData`. How you implement this sharing is up to you - Tanim does not prescribe a specific approach. You might use:
- A resource management system with reference counting
- Smart pointers like `std::shared_ptr`
- Your existing asset/resource sharing system

**Example using a custom resource system** (this is just one possible approach):

```cpp
// Your resource management system (not part of Tanim)
struct TimelineResource
{
    tanim::TimelineData timeline_data;
    std::string name;
    std::string filepath;
};

// Multiple entities reference the same resource
struct AnimationComponent
{
    ResourceHandle<TimelineResource> timeline_handle;  // Shared (your system)
    tanim::ComponentData component_data;                // Unique per entity (Tanim)
};
```

**Example using shared pointers**:

```cpp
struct AnimationComponent
{
    std::shared_ptr<tanim::TimelineData> timeline_data;  // Shared
    tanim::ComponentData component_data;                  // Unique per entity
};
```

See [Example Implementation](../example-implementation.md) for a complete resource management example.

**Benefits of sharing**:
- Memory efficiency: Store one copy instead of duplicating per entity
- Reusability: Create one animation and use it on multiple entities with similar hierarchies
- Consistency: Edit once, affects all entities using it

### Serialization

Tanim provides functions to save and load `TimelineData`:

```cpp
// Serialize to string
std::string serialized = tanim::Serialize(timeline_data);
SaveToFile("animation.tanim", serialized);

// Deserialize from string
std::string loaded = LoadFromFile("animation.tanim");
tanim::Deserialize(timeline_data, loaded);
```

**Internal Format**: Tanim currently uses JSON for serialization. Always use `Serialize()` and `Deserialize()` rather than parsing the string directly.

**Version Handling**: Tanim handles versioning internally and tries to maintain backward compatibility. If you try to load an old format that is no longer supported, you'll get an error through `LogError()`.

**Best Practice**: Only serialize when actually saving (this is an expensive operation), not every frame.

### Direct Access

**Warning**: You should not directly access or modify fields in `TimelineData`. All data is managed internally by Tanim through the editor UI and API functions.

The only time you interact with `TimelineData` is:
- Passing it to Tanim API functions
- Serializing/deserializing it
- Storing it in your resource/asset system

### Usage Example

```cpp
struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};

// Creating a new animation
void CreateNewAnimation(entt::entity entity)
{
    auto& anim = registry.emplace<AnimationComponent>(entity);
    // timeline_data is default-constructed and ready to use
}

// Saving animation
void SaveAnimation(entt::entity entity, const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    std::string data = tanim::Serialize(anim.timeline_data);
    WriteFile(filepath, data);
}

// Loading animation
void LoadAnimation(entt::entity entity, const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    std::string data = ReadFile(filepath);
    tanim::Deserialize(anim.timeline_data, data);
}
```

---

## ComponentData

```cpp
struct ComponentData
{
    std::any m_user_data;
    
private:
    // Runtime playback state handled internally by Tanim
};
```

### Purpose

Holds runtime playback state for a specific entity's timeline. Each entity playing a timeline needs its own `ComponentData`, even if they share the same `TimelineData`.

### Lifetime

- Created when you add an animation component to an entity
- Exists for as long as the entity exists
- Does not need to be serialized (runtime state only)
- Destroyed when the entity is destroyed

### Fields

#### m_user_data

**Type**: `std::any`

**Purpose**: Storage for custom data that helps your `FindEntityOfUID` implementation look up entities efficiently.

**Common Uses**:

1. **Store the root entity**:
```cpp
cdata.m_user_data = root_entity;

// In FindEntityOfUID:
entt::entity root = std::any_cast<entt::entity>(cdata.m_user_data);
```

2. **Cache entity list**:
```cpp
struct AnimUserData
{
    entt::entity root_entity;
    std::vector<entt::entity> cached_children;
};

cdata.m_user_data = AnimUserData{
    .root_entity = entity,
    .cached_children = GetAllChildren(entity)
};

// In FindEntityOfUID:
auto* user_data = std::any_cast<AnimUserData>(&cdata.m_user_data);
for (entt::entity child : user_data->cached_children)
{
    if (GetEntityUID(child) == uid_to_find)
        return child;
}
```

3. **Store entity data list**:
```cpp
struct AnimUserData
{
    entt::entity root_entity;
    std::vector<tanim::EntityData> entity_datas;
    std::vector<entt::entity> entities;
};

cdata.m_user_data = AnimUserData{
    .root_entity = entity,
    .entity_datas = BuildEntityDataList(entity),
    .entities = GetAllChildren(entity)
};

// Reuse the same list for both OpenForEditing and UpdateTimeline
auto* user_data = std::any_cast<AnimUserData>(&cdata.m_user_data);
tanim::OpenForEditing(registry, user_data->entity_datas, tdata, cdata);
tanim::UpdateTimeline(registry, user_data->entity_datas, tdata, cdata, dt);
```

**Best Practice**: Set `m_user_data` before calling `OpenForEditing()` or `UpdateTimeline()` so your `FindEntityOfUID` can access it. Store whatever data helps your `FindEntityOfUID` implementation run efficiently.

### Independent Playback State

Each entity has its own `ComponentData`, which means:

**Independent control**: Each entity can be playing, paused, or stopped independently:
```cpp
// Entity 1 is playing
tanim::Play(entity1_cdata);

// Entity 2 is paused
tanim::Pause(entity2_cdata);

// Entity 3 is stopped
tanim::Stop(entity3_cdata);
```

**Independent timing**: Entities can be at different times in the same animation:
```cpp
// Both share the same TimelineData but are at different times
// This happens naturally as they have separate ComponentData
```

**Synchronized control**: To synchronize entities, call the same function on all of them:
```cpp
// Synchronize playback for all entities using this timeline
for (auto entity : entities_sharing_timeline)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    tanim::Play(anim.component_data);
}
```

**Best Practice**: One `ComponentData` per entity, even when sharing `TimelineData`. Don't serialize `ComponentData` - it contains runtime state only.

### Usage Example

```cpp
struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};

// Initialize component data when opening for editing
void OpenTimelineForEditing(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    
    // Set user data for FindEntityOfUID
    struct UserData
    {
        entt::entity root;
        std::vector<entt::entity> children;
    };
    
    anim.component_data.m_user_data = UserData{
        .root = entity,
        .children = GetAllChildren(entity)
    };
    
    auto entity_list = BuildEntityList(entity);
    tanim::OpenForEditing(registry, entity_list, 
                         anim.timeline_data, 
                         anim.component_data);
}

// FindEntityOfUID implementation
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, 
                                   const std::string& uid_to_find)
{
    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data is empty");
        return entt::null;
    }
    
    auto* user_data = std::any_cast<UserData>(&cdata.m_user_data);
    if (!user_data)
    {
        LogError("m_user_data is wrong type");
        return entt::null;
    }
    
    // Check root
    if (GetEntityUID(user_data->root) == uid_to_find)
        return user_data->root;
    
    // Check children
    for (entt::entity child : user_data->children)
    {
        if (GetEntityUID(child) == uid_to_find)
            return child;
    }
    
    return entt::null;
}
```

---

## Data Flow Summary

### Editor Workflow

```
User creates AnimationComponent with TimelineData and ComponentData
    ↓
User calls OpenForEditing with EntityData list
    ↓
User edits in Tanim editor (creates sequences, keyframes)
    ↓
Changes stored in TimelineData
    ↓
User calls Serialize to save TimelineData to disk
```

### Runtime Workflow

```
Load TimelineData from disk with Deserialize
    ↓
Attach AnimationComponent to entities
    ↓
Call StartTimeline to initialize playback
    ↓
Each frame: UpdateTimeline samples curves and writes to components
    ↓
Tanim uses FindEntityOfUID (which uses ComponentData.m_user_data) to find entities
    ↓
Component fields are updated with animated values
```

### Data Relationships

```
TimelineData (shared)
    ├── Entity 1: ComponentData (unique)
    ├── Entity 2: ComponentData (unique)
    └── Entity 3: ComponentData (unique)

Each entity has:
- Reference to shared TimelineData (animation data)
- Own ComponentData (playback state, user data)
```

---

## Next Steps

- [Lifecycle Functions](lifecycle.md) - How to use these data structures in API calls
- [User Overrides](user-overrides.md) - Implement FindEntityOfUID to use ComponentData.m_user_data
- [Example Implementation](../example-implementation.md) - See complete integration examples
