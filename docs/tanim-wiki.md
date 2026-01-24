# Tanim Documentation

## Table of Contents

1. [Installation](#installation)
2. [Getting Started](#getting-started)
3. [Architecture Overview](#architecture-overview)
4. [Data Structures](#data-structures)
5. [Reflection System](#reflection-system)
6. [User Overrides](#user-overrides)
7. [API Reference](#api-reference)
8. [Supported Types](#supported-types)
9. [Editor UI & Shortcuts](#editor-ui--shortcuts)
10. [Performance](#performance)
11. [Example Implementation](#example-implementation)
12. [Future](#future)

---

## Installation

> [!NOTE]  
> CMake support is on the way. Until then, please follow the steps to manually integrate Tanim into your project.

Clone, download zip, or download the latest release.

### Prerequisites

Minimum C++17.

Tanim is designed for projects using ImGui for editor GUI and ENTT for their ECS. Your project must already include:

| Library    | Version            | GitHub                                   |
| ---------- | ------------------ | ---------------------------------------- |
| ENTT       | tested with 3.15.0 | [LINK](https://github.com/skypjack/entt) |
| Dear ImGui | tested with 1.92.3 | [LINK](https://github.com/ocornut/imgui) |

Tanim also depends on the libraries below, but only internally. You must add any you don't already have to your project. Either place them where your other external libraries are, or add everything inside [the external folder](https://github.com/hegworks/tanim/tree/main/external) individually to your include paths:

| Library       | Version             | GitHub                                          |
| ------------- | ------------------- | ----------------------------------------------- |
| glm           | tested with 0.9.9.9 | [LINK](https://github.com/g-truc/glm)           |
| visit_struct  | **minimum** 1.2.0   | [LINK](https://github.com/cbeck88/visit_struct) |
| magic_enum    | tested with 0.9.7   | [LINK](https://github.com/Neargye/magic_enum)   |
| nlohmann/json | tested with 3.12.0  | [LINK](https://github.com/nlohmann/json)        |

As an **example** for each library, these includes must be accessible in your code like this:

```cpp
#include <entt/entt.hpp>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <visit_struct/visit_struct.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
```

---

## Getting Started

This section walks you through integrating Tanim into your project and creating your first animation.

### Step 1: Include Tanim

```cpp
#include <tanim/tanim.hpp>
```

This single header includes all the API functions and data structures you need.

### Step 2: Initialize Tanim

Call `tanim::Init()` once at application startup, before any other Tanim functions:

```cpp
void OnApplicationStart()
{
    ImGui::CreateContext();
    // ... your ImGui setup

    tanim::Init();
}
```

### Step 3: Implement Required User Overrides

Tanim requires you to implement three functions. See [User Overrides](#user-overrides) for detailed explanations.

```cpp
#include <tanim/tanim.hpp>

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    // Your implementation - return the entity matching uid_to_find
    // Return entt::null if not found
}

void tanim::LogError(const std::string& message)
{
    // Your logging system
}

void tanim::LogInfo(const std::string& message)
{
    // Your logging system
}
```

### Step 4: Reflect Your Components

For each component you want to animate, use `TANIM_REFLECT` in the global namespace. See [Reflection System](#reflection-system) for details.

```cpp
struct Transform
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

TANIM_REFLECT(Transform, position, rotation, scale);
```

### Step 5: Create a Component to Hold Timeline Data

```cpp
struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};
```

See [Data Structures](#data-structures) for detailed information about these types.

### Step 6: Integrate with Your Editor Loop

```cpp
void OnEditorFrame(float delta_time)
{
    ImGui::NewFrame();
    tanim::UpdateEditor(delta_time);
    tanim::Draw();
    ImGui::Render();
}
```

### Step 7: Open a Timeline for Editing

```cpp
void OpenTimelineEditor(entt::registry& registry,
                       entt::entity entity_with_animation,
                       AnimationComponent& anim_comp)
{
    std::vector<tanim::EntityData> entity_datas;
    entity_datas.push_back({
        .m_uid = "RootEntity",
        .m_display = "Root Entity"
    });

    tanim::OpenForEditing(registry, entity_datas,
                         anim_comp.timeline_data,
                         anim_comp.component_data);
}
```

TODOVISUAL Add screenshot of Tanim editor window

### Step 8: Play Animations at Runtime

```cpp
void OnPlayMode()
{
    tanim::EnterPlayMode();
    tanim::StartTimeline(anim_comp.timeline_data, anim_comp.component_data);
}

void OnUpdate(float delta_time)
{
    tanim::UpdateTimeline(registry, entity_datas,
                         anim_comp.timeline_data,
                         anim_comp.component_data,
                         delta_time);
}

void OnExitPlayMode()
{
    tanim::StopTimeline(anim_comp.component_data);
    tanim::ExitPlayMode();
}
```

### Minimal Complete Example

```cpp
#include <tanim/tanim.hpp>
#include <entt/entt.hpp>
#include <imgui/imgui.h>

struct Transform
{
    glm::vec3 position{0.0f};
};
TANIM_REFLECT(Transform, position);

struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};

int main()
{
    ImGui::CreateContext();
    tanim::Init();

    entt::registry registry;
    entt::entity entity = registry.create();
    registry.emplace<Transform>(entity);
    registry.emplace<AnimationComponent>(entity);

    while (running)
    {
        float delta_time = GetDeltaTime();
        ImGui::NewFrame();
        tanim::UpdateEditor(delta_time);
        tanim::Draw();
        ImGui::Render();
    }
    return 0;
}

// User override implementations
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    return entt::null; // Your implementation
}

void tanim::LogError(const std::string& message)
{
    std::cerr << "[TANIM] " << message << std::endl;
}

void tanim::LogInfo(const std::string& message)
{
    std::cout << "[TANIM] " << message << std::endl;
}
```

### Common Issues

| Issue                            | Solution                                                                                                 |
| -------------------------------- | -------------------------------------------------------------------------------------------------------- |
| Editor window doesn't appear     | Ensure `tanim::Init()` and `tanim::Draw()` are called between `ImGui::NewFrame()` and `ImGui::Render()`  |
| Animations don't play            | Verify `tanim::EnterPlayMode()` and `tanim::StartTimeline()` are called before `tanim::UpdateTimeline()` |
| Components not showing in editor | Check that `TANIM_REFLECT` is in the global namespace with only supported types                          |
| Entity not found errors          | Verify your `FindEntityOfUID` implementation correctly maps UIDs to entities                             |

---

## Architecture Overview

Tanim follows a hierarchical data structure for organizing animations:

```
Component (on entity)
    └─ Timeline
        └─ Sequence (per animated field)
            └─ Curve (per field component, e.g., x, y, z)
                └─ Keyframe
                    ├─ In Handle
                    └─ Out Handle
```

### Component

A component attached to an entity that holds references to a `TimelineData` and its runtime `ComponentData`. In your engine, this might be called `AnimationComponent`, `Tanimatable`, or any name you choose.

### Timeline

Contains all the sequences that animate different component fields on entities. A timeline can be shared across multiple entities, allowing the same animation to be reused (e.g., multiple enemies can reference the same "patrol" animation timeline).

### Sequence

Represents a single animated property on a component. Each sequence targets a specific component on a specific entity (identified by UID) and a specific field within that component.

### Curve

A sequence contains one or more curves depending on the field type:

- `float` field → 1 curve
- `glm::vec3` field → 3 curves (x, y, z)
- `glm::quat` field → 5 curves (w, x, y, z, spins)

Each curve contains keyframes that define the animation over time.

### Keyframe

A point in time that defines a value on a curve. Keyframes are connected by curve segments that interpolate between them.

### Handle

Each keyframe has two handles (in and out) that control the shape of the Bezier curve segment, defining the tangent at the keyframe and affecting how smoothly the animation transitions.

### Animation Data Flow

**Editor Mode:**

1. User opens a timeline via `OpenForEditing()`
2. User creates sequences, keyframes, and edits curves
3. Changes are stored in `TimelineData`
4. User serializes to disk via `Serialize()`

**Play Mode:**

1. Application calls `EnterPlayMode()`
2. For each animated entity, call `StartTimeline()`
3. Each frame, `UpdateTimeline()` samples curves and writes to components
4. When stopping, call `StopTimeline()` and `ExitPlayMode()`

---

## Data Structures

Tanim uses three main data structures to bridge your engine and the animation system.

### EntityData

```cpp
struct EntityData
{
    std::string m_uid;      // Unique identifier for entity lookup
    std::string m_display;  // Display name in the editor
};
```

**Purpose**: Identifies entities that can be animated in a timeline.

**m_uid considerations:**

| Approach     | Pros                                                        | Cons                                   |
| ------------ | ----------------------------------------------------------- | -------------------------------------- |
| Entity names | Readable, allows animation reuse across similar hierarchies | Requires unique names                  |
| UUIDs        | Guaranteed unique                                           | Not readable, prevents animation reuse |
| Custom IDs   | Flexible                                                    | Depends on your implementation         |

**Important**: The UID must persist across application sessions. `entt::entity` IDs are not suitable because they can change when scenes are reloaded.

**Example:**

```cpp
std::vector<tanim::EntityData> BuildEntityList(entt::entity root)
{
    std::vector<tanim::EntityData> entity_datas;
    entity_datas.push_back({
        .m_uid = GetEntityUID(root),
        .m_display = GetEntityName(root)
    });

    for (entt::entity child : GetAllChildren(root))
    {
        entity_datas.push_back({
            .m_uid = GetEntityUID(child),
            .m_display = GetEntityName(child)
        });
    }
    return entity_datas;
}
```

### TimelineData

```cpp
struct TimelineData
{
    // All fields handled internally by Tanim
};
```

**Purpose**: Stores all animation data (sequences, curves, keyframes, settings). This is serialized when saving animations.

**Lifetime**: Persists as long as you want to keep the animation data. Can be serialized and shared across multiple entities.

**Sharing**: Multiple entities can reference the same `TimelineData`. Implementation approaches include resource management systems, `std::shared_ptr`, or your existing asset system.

**Serialization:**

```cpp
// Save
std::string serialized = tanim::Serialize(timeline_data);
SaveToFile("animation.tanim", serialized);

// Load
std::string loaded = LoadFromFile("animation.tanim");
tanim::Deserialize(timeline_data, loaded);
```

**Warning**: Do not directly access or modify `TimelineData` fields. All data is managed internally through the editor UI and API functions.

### ComponentData

```cpp
struct ComponentData
{
    std::any m_user_data;  // Your custom data for FindEntityOfUID
private:
    // Runtime playback state handled internally
};
```

**Purpose**: Holds runtime playback state for a specific entity's timeline. Each entity playing a timeline needs its own `ComponentData`, even when sharing `TimelineData`.

**Lifetime**: Exists as long as the entity exists. Does not need to be serialized.

**m_user_data**: Store data to help your `FindEntityOfUID` implementation run efficiently:

```cpp
struct TanimUserData
{
    entt::entity root_entity;
    std::vector<entt::entity> cached_children;
};

anim.component_data.m_user_data = TanimUserData{
    .root_entity = entity,
    .cached_children = GetAllChildren(entity)
};
```

**Independent Playback**: Each entity has its own `ComponentData`, so entities can be playing, paused, or stopped independently, or be at different times in the same animation.

---

## Reflection System

Tanim uses compile-time reflection to access component fields for animation.

### TANIM_REFLECT

```cpp
TANIM_REFLECT(STRUCT_NAME, field1, field2, field3, ...);
```

Reflects a component and automatically registers it with Tanim. Call this in the **global namespace**:

```cpp
namespace MyEngine
{
struct Transform
{
    glm::vec3 position{0.0f};
    glm::quat rotation{glm::identity<glm::quat>()};
    glm::vec3 scale{1.0f};
};
}

// Global namespace - include full namespace path
TANIM_REFLECT(MyEngine::Transform, position, rotation, scale);
```

**Important Notes:**

- Always include the full namespace path in `STRUCT_NAME`
- Only include fields with [supported types](#supported-types)
- You don't need to reflect all fields—only the ones you want to animate

### TANIM_REFLECT_NO_REGISTER

Use only if `TANIM_REFLECT` causes initialization order issues for a specific component:

```cpp
TANIM_REFLECT_NO_REGISTER(MyEngine::ProblematicComponent, value);

// Then manually register during initialization:
void InitializeEngine()
{
    tanim::Init();
    tanim::RegisterComponent<MyEngine::ProblematicComponent>();
}
```

### Reflecting Private Fields

Use `BEFRIEND_VISITABLE()` inside your component:

```cpp
struct CTransform
{
    glm::vec3 m_pos{0.0f};
private:
    BEFRIEND_VISITABLE();
    glm::quat m_rot{glm::identity<glm::quat>()};
};

TANIM_REFLECT(CTransform, m_pos, m_rot);
```

### Reflection Requirements

- **Supported types only**: See [Supported Types](#supported-types)
- **No static fields**: Animation is per-entity; static fields would affect all entities
- **No functions**: Animation requires data storage to write values to

### Troubleshooting

| Error                                                            | Cause                                          | Solution                                                                          |
| ---------------------------------------------------------------- | ---------------------------------------------- | --------------------------------------------------------------------------------- |
| "No matching function for call to 'visit_struct::apply_visitor'" | Missing or incorrect reflection macro          | Ensure `TANIM_REFLECT` is in global namespace with correct struct name and fields |
| Component not appearing in editor                                | Not registered or unsupported types            | Verify macro usage and field types                                                |
| Multiple definition errors                                       | Macro called more than once for same component | Use include guards; keep macro in header file                                     |

---

## User Overrides

Three functions you must implement in your project for Tanim to integrate with your engine.

### FindEntityOfUID

```cpp
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find);
```

Converts a UID string back to an `entt::entity`. Called frequently during playback and editing.

**Return**: The matching `entt::entity`, or `entt::null` if not found.

**Performance**: Use `ComponentData::m_user_data` to cache entity lookups.

**Recommended Implementation Pattern:**

```cpp
struct TanimUserData
{
    entt::entity root_entity;
    std::vector<entt::entity> cached_entities;
};

entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
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

    if (GetEntityName(user_data->root_entity) == uid_to_find)
        return user_data->root_entity;

    for (entt::entity entity : user_data->cached_entities)
    {
        if (GetEntityName(entity) == uid_to_find)
            return entity;
    }

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

### LogError / LogInfo

```cpp
void tanim::LogError(const std::string& message);
void tanim::LogInfo(const std::string& message);
```

Route messages through your logging system:

```cpp
void tanim::LogError(const std::string& message)
{
    MyEngine::Logger::Error("[TANIM] " + message);
}

void tanim::LogInfo(const std::string& message)
{
    MyEngine::Logger::Info("[TANIM] " + message);
}
```

**Common Error Messages:**

- `"FindEntityOfUID with the uid of [uid] returned entt::null"` - Entity not found
- `"entity [id] does not have a component named [ComponentName]"` - Component missing
- `"Versions prior to 2 are not supported. Can not deserialize."` - Old serialization format

---

## API Reference

### Initialization

#### tanim::Init

```cpp
void Init();
```

Initializes the Tanim system. Call once at startup after ImGui initialization.

### Editor Integration

#### tanim::Draw

```cpp
void Draw();
```

Renders the Tanim editor window. Call every frame between `ImGui::NewFrame()` and `ImGui::Render()`.

#### tanim::UpdateEditor

```cpp
void UpdateEditor(float dt);
```

Updates time-based editor operations. Call every frame before `Draw()`.

### Timeline Editing

#### tanim::OpenForEditing

```cpp
void OpenForEditing(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata);
```

Opens the Tanim editor window. Only one timeline can be open at a time.

#### tanim::CloseEditor

```cpp
void CloseEditor();
```

Closes the editor window. **Must be called before destroying TimelineData or ComponentData.**

### Play Mode Control

#### tanim::EnterPlayMode / tanim::ExitPlayMode

```cpp
void EnterPlayMode();
void ExitPlayMode();
```

Signal play mode transitions. Call once when your application transitions between editor and play modes.

### Timeline Playback

#### tanim::StartTimeline

```cpp
void StartTimeline(const TimelineData& tdata, ComponentData& cdata);
```

Prepares a timeline for playback. Call after `EnterPlayMode()`.

#### tanim::UpdateTimeline

```cpp
void UpdateTimeline(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata,
                    float delta_time);
```

Advances playback, samples curves, and writes values to components. Call every frame during play mode.

#### tanim::StopTimeline

```cpp
void StopTimeline(ComponentData& cdata);
```

Stops playback and resets time. Call before `ExitPlayMode()`.

### Playback Control

```cpp
void Play(ComponentData& cdata);     // Start or resume
void Pause(ComponentData& cdata);    // Pause at current time
void Stop(ComponentData& cdata);     // Stop and reset to beginning
bool IsPlaying(const ComponentData& cdata);  // Check state
```

### Serialization

```cpp
std::string Serialize(TimelineData& tdata);
void Deserialize(TimelineData& tdata, const std::string& serialized_string);
```

### Component Registration

```cpp
template <typename T>
void RegisterComponent();
```

Manually registers a component. Only needed for components using `TANIM_REFLECT_NO_REGISTER`.

### Function Call Order

**Startup:**

```
ImGui::CreateContext() → tanim::Init()
```

**Editor Frame:**

```
ImGui::NewFrame() → tanim::UpdateEditor(dt) → tanim::Draw() → ImGui::Render()
```

**Entering Play Mode:**

```
tanim::EnterPlayMode() → tanim::StartTimeline() for each entity
```

**Play Mode Frame:**

```
tanim::UpdateTimeline() for each entity
```

**Exiting Play Mode:**

```
tanim::StopTimeline() for each entity → tanim::ExitPlayMode()
```

### Threading

Tanim is **not thread-safe**. All API functions must be called from the same thread.

---

## Performance

Tanim has **O(n) linear time complexity** where n is the number of animated entities.

- **Per entity**: ~1ms per 1000 entities
- **60 FPS limit**: ~15,000 animated entities

### Best Practices

| Practice                             | Benefit                                |
| ------------------------------------ | -------------------------------------- |
| Cache entity lists in `m_user_data`  | Avoid rebuilding lists every frame     |
| Share `TimelineData` across entities | Reduce memory, maintain consistency    |
| Optimize `FindEntityOfUID`           | Called frequently during playback      |
| Limit animated entities              | Only animate visible/relevant entities |
| Use simpler types when possible      | `float` is faster than `glm::quat`     |
| Serialize only when saving           | Expensive operation                    |
