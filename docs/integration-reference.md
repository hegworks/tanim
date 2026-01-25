# Tanim Documentation

## Table of Contents

1. [Installation](#installation)
2. [Getting Started](#getting-started)
3. [Architecture Overview](#architecture-overview)
4. [Data Structures](#data-structures)
5. [Reflection System](#reflection-system)
6. [User Overrides](#user-overrides)
7. [API Reference](#api-reference)
8. [Performance](#performance)

**See Also:**

- [Supported Types](supported-types.md) - Complete type reference with behaviors
- [Editor UI & Shortcuts](ui-shortcuts.md) - Editor controls and keyboard shortcuts
- [Example Implementation](example-implementation.md) - Complete integration example

---

## Installation

> [!NOTE]  
> CMake support is on the way. Until then, please follow the steps to manually integrate Tanim into your project.

### Prerequisites

Minimum C++17.

Tanim is designed for projects using ImGui for editor GUI and ENTT for their ECS. Your project must already include:

| Library    | Version            | GitHub                                   |
| ---------- | ------------------ | ---------------------------------------- |
| ENTT       | tested with 3.15.0 | [LINK](https://github.com/skypjack/entt) |
| Dear ImGui | tested with 1.92.3 | [LINK](https://github.com/ocornut/imgui) |

### Step 1: Get Tanim

Choose one of the following methods:

- **Clone**: `git clone https://github.com/hegworks/tanim.git`
- **Download ZIP**: Download from the [GitHub repository](https://github.com/hegworks/tanim)
- **Release**: Download the latest release from [Releases](https://github.com/hegworks/tanim/releases)

### Step 2: Add Tanim to Your Project

Copy the `tanim` folder into your project's external/third-party libraries directory. For example:

```
YourProject/
├── src/
├── external/
│   ├── tanim/          <-- Place tanim here
│   │   ├── include/
│   │   │   └── tanim/
│   │   │       └── tanim.hpp
│   │   └── external/   <-- Tanim's internal dependencies
│   ├── entt/
│   ├── imgui/
│   └── ...
└── ...
```

### Step 3: Configure Include Paths

Add Tanim's include directory to your project's include paths so you can access:

```cpp
#include <tanim/tanim.hpp>
```

The path to add is the `include` folder inside the `tanim` directory (e.g., `external/tanim/include`).

**Visual Studio**: Project Properties > C/C++ > General > Additional Include Directories

### Step 4: Add Dependencies

Tanim depends on the libraries below internally. You must add any you don't already have to your project. Either place them where your other external libraries are, or add everything inside [the external folder](https://github.com/hegworks/tanim/tree/main/external) individually to your include paths:

| Library       | Version             | GitHub                                          |
| ------------- | ------------------- | ----------------------------------------------- |
| glm           | tested with 0.9.9.9 | [LINK](https://github.com/g-truc/glm)           |
| visit_struct  | **minimum** 1.2.0   | [LINK](https://github.com/cbeck88/visit_struct) |
| magic_enum    | tested with 0.9.7   | [LINK](https://github.com/Neargye/magic_enum)   |
| nlohmann/json | tested with 3.12.0  | [LINK](https://github.com/nlohmann/json)        |

Verify the installation by ensuring these includes work in your code:

```cpp
#include <tanim/tanim.hpp>
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

Tanim requires you to implement three functions. See [User Overrides](#user-overrides) for detailed explanations and implementation patterns.

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
| Components not showing in editor | Check that `TANIM_REFLECT` is in the global namespace with only [supported types](supported-types.md)    |
| Entity not found errors          | Verify your [FindEntityOfUID](#findentityofuid) implementation correctly maps UIDs to entities           |

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

A component attached to an entity that holds references to a [TimelineData](#timelinedata) and its runtime [ComponentData](#componentdata). In your engine, this might be called `AnimationComponent`, `Tanimatable`, or any name you choose.

### Timeline

Contains all the sequences that animate different component fields on entities. A timeline can be shared across multiple entities, allowing the same animation to be reused (e.g., multiple enemies can reference the same "patrol" animation timeline). See [Timeline Sharing and Reusability](#timeline-sharing-and-reusability) for details.

### Sequence

Represents a single animated property on a component. Each sequence targets a specific component on a specific entity (identified by UID) and a specific field within that component.

### Curve

A sequence contains one or more curves depending on the field type. For example, a `float` has 1 curve, a `glm::vec3` has 3 curves (x, y, z), and a `glm::quat` has 5 curves (w, x, y, z, spins). See [Supported Types](supported-types.md) for the complete list with curve counts and type-specific behaviors.

Each curve contains keyframes that define the animation over time.

### Keyframe

A point in time that defines a value on a curve. Keyframes are connected by curve segments that interpolate between them.

### Handle

Each keyframe has two handles (in and out) that control the shape of the Bezier curve segment, defining the tangent at the keyframe and affecting how smoothly the animation transitions. See [Editor UI & Shortcuts](ui-shortcuts.md) for how to manipulate handles in the editor.

### Curve Types and Interpolation

Tanim uses cubic Bezier curves for interpolation between keyframes. This is the same system used by Unity, Unreal, and other professional animation tools.

**Why Bezier Curves?** Bezier curves allow precise control over animation timing and easing. Unlike simple linear interpolation, Bezier curves can create smooth acceleration and deceleration, making animations feel more natural.

### Tangent Modes

Each keyframe can have different tangent modes that control the curve shape. See [Editor UI & Shortcuts](ui-shortcuts.md) for how to change tangent modes in the editor.

### Curve Constraints

Tanim enforces monotonicity in time, meaning the curve always moves forward in time and never loops back. This prevents invalid animations where a single time value would map to multiple values. Handles are automatically clamped to neighboring keyframes to maintain this constraint.

### Quaternion Animation

Quaternions (`glm::quat`) require special handling because their four components (w, x, y, z) are interdependent. You cannot animate each component independently like you can with vectors.

Tanim provides synchronized keyframe operations for quaternions: adding or removing keyframes affects all five curves (w, x, y, z, spins) simultaneously. The "spins" curve controls how many full 360-degree rotations occur during interpolation.

For complete details on quaternion animation, including the spins curve and editing restrictions, see [Supported Types > glm::quat](supported-types.md#glmquat).

### Animation Data Flow

**Editor Mode:**

1. User opens a timeline via [OpenForEditing()](#tanimopenforediting)
2. User creates sequences, keyframes, and edits curves
3. Changes are stored in [TimelineData](#timelinedata)
4. User serializes to disk via [Serialize()](#serialization)

**Play Mode:**

1. Application calls [EnterPlayMode()](#tanimenterplaymode--tanimexitplaymode)
2. For each animated entity, call [StartTimeline()](#tanimstarttimeline)
3. Each frame, [UpdateTimeline()](#tanimupdatetimeline):
   - Advances playback time based on delta time
   - Samples curves at the current time
   - Writes sampled values directly to component fields via your [FindEntityOfUID](#findentityofuid) implementation
4. When stopping, call [StopTimeline()](#tanimstoptimeline) and [ExitPlayMode()](#tanimenterplaymode--tanimexitplaymode)

### Timeline Sharing and Reusability

One [TimelineData](#timelinedata) can be referenced by multiple entities. This is useful for:

- **Reusable animations**: Create one "walk cycle" timeline and use it on multiple characters
- **Memory efficiency**: Store one copy of animation data instead of duplicating per entity

**Synchronized vs Independent Playback**: Each entity has its own [ComponentData](#componentdata) containing runtime playback state, so entities can play, pause, or stop independently, or be at different times in the same animation. See [Playback Control](#playback-control) for the control functions.

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

**Purpose**: Identifies entities that can be animated in a timeline. You provide a vector of `EntityData` when calling [OpenForEditing()](#tanimopenforediting) or [UpdateTimeline()](#tanimupdatetimeline).

The `m_uid` field is used by your [FindEntityOfUID](#findentityofuid) implementation to convert the string back to an `entt::entity`.

#### m_uid Considerations

| Approach     | Pros                                                        | Cons                                   |
| ------------ | ----------------------------------------------------------- | -------------------------------------- |
| Entity names | Readable, allows animation reuse across similar hierarchies | Requires unique names                  |
| UUIDs        | Guaranteed unique                                           | Not readable, prevents animation reuse |
| Custom IDs   | Flexible                                                    | Depends on your implementation         |

**Session Persistence**: The UID must remain the same between application sessions. For example, `entt::entity` IDs are not suitable because they can change when a scene is loaded. The same scene with the same entities might have different `entt::entity` IDs after reloading, which would break animation references.

**Reusability vs Uniqueness**: The UID scheme affects animation reusability:

- **Universally unique IDs (UUIDs)**: If you use UUIDs that are unique across the entire scene, you lose the ability to reuse the same `TimelineData` on multiple entities with identical hierarchies
- **Name-based IDs**: Using entity names (like Unity and Godot do) allows animation reuse across similar hierarchies

**Recommended Approach**: Entity names or hierarchical names are common and practical. Ensure proper error handling in your [FindEntityOfUID](#findentityofuid) implementation for cases where names aren't found or are duplicated.

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

**Performance Tip**: Cache the `entity_datas` vector and rebuild it only when the hierarchy changes. Store the cached list in [ComponentData::m_user_data](#componentdata) so both `OpenForEditing()` and `UpdateTimeline()` can use it. See [Example Implementation](example-implementation.md) for this pattern.

### TimelineData

```cpp
struct TimelineData
{
    // All fields handled internally by Tanim
};
```

**Purpose**: Stores all animation data (sequences, curves, keyframes, settings). This is serialized when saving animations.

**Lifetime**: Persists as long as you want to keep the animation data. Can be serialized and shared across multiple entities.

**Sharing**: Multiple entities can reference the same `TimelineData`. Implementation approaches include resource management systems, `std::shared_ptr`, or your existing asset system. See [Example Implementation](example-implementation.md) for a complete resource management example.

**Serialization:**

```cpp
// Save
std::string serialized = tanim::Serialize(timeline_data);
SaveToFile("animation.tanim", serialized);

// Load
std::string loaded = LoadFromFile("animation.tanim");
tanim::Deserialize(timeline_data, loaded);
```

**Internal Format**: Tanim uses JSON for serialization. Always use [Serialize() and Deserialize()](#serialization) rather than parsing the string directly. Tanim handles versioning internally and will report errors through [LogError](#logerror--loginfo) if an unsupported version is encountered.

**Warning**: Do not directly access or modify `TimelineData` fields. All data is managed internally through the editor UI and [API functions](#api-reference).

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

**Lifetime**: Exists as long as the entity exists. Does not need to be serialized (runtime state only).

**m_user_data**: Store data to help your [FindEntityOfUID](#findentityofuid) implementation run efficiently. See [User Overrides](#user-overrides) for multiple implementation patterns using this field.

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

**Independent Playback**: Each entity has its own `ComponentData`, so entities can be playing, paused, or stopped independently. See [Playback Control](#playback-control) for the control functions.

```cpp
tanim::Play(entity1_cdata);   // Entity 1 is playing
tanim::Pause(entity2_cdata);  // Entity 2 is paused
tanim::Stop(entity3_cdata);   // Entity 3 is stopped
```

### Data Flow Summary

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

## Reflection System

Tanim uses compile-time reflection to access component fields for animation. Only fields with [supported types](supported-types.md) can be reflected.

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

**What Happens Behind the Scenes**: `TANIM_REFLECT` does two things:

1. Calls `VISITABLE_STRUCT_IN_CONTEXT` to make the component's fields accessible via `visit_struct`
2. Automatically registers the component with Tanim's type system during static initialization

The registration happens before `main()` is called, so by the time you call [tanim::Init()](#taniminit), the component is already available.

**Important Notes:**

- Always include the full namespace path in `STRUCT_NAME`
- Only include fields with [supported types](supported-types.md)
- You don't need to reflect all fields, only the ones you want to animate

**Subset of Fields Example:**

```cpp
struct Camera
{
    float fov{45.0f};           // Will be animatable
    float near_plane{0.1f};     // Will be animatable
    glm::mat4 projection;       // Not reflected, won't be animatable
};

TANIM_REFLECT(Camera, fov, near_plane);
```

**Nested Namespaces Example:**

```cpp
namespace MyEngine::Components
{
struct RigidBody
{
    glm::vec3 velocity{0.0f};
    float mass{1.0f};
};
}

TANIM_REFLECT(MyEngine::Components::RigidBody, velocity, mass);
```

### TANIM_REFLECT_NO_REGISTER

Use only if `TANIM_REFLECT` causes initialization order issues for a specific component (e.g., if the component's constructor accesses global variables that aren't initialized yet):

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

- **Supported types only**: See [Supported Types](supported-types.md) for the complete list
- **No static fields**: Animation is per-entity; static fields would affect all entities simultaneously
- **No functions**: Animation requires data storage to write values to; functions are executable code, not data

### Troubleshooting

| Error                                                            | Cause                                          | Solution                                                                          |
| ---------------------------------------------------------------- | ---------------------------------------------- | --------------------------------------------------------------------------------- |
| "No matching function for call to 'visit_struct::apply_visitor'" | Missing or incorrect reflection macro          | Ensure `TANIM_REFLECT` is in global namespace with correct struct name and fields |
| Component not appearing in editor                                | Not registered or unsupported types            | Verify macro usage and check [Supported Types](supported-types.md)                |
| Multiple definition errors (C2766)                               | Macro called more than once for same component | Use include guards; keep macro in header file only                                |
| Initialization order crash                                       | Constructor accesses uninitialized globals     | Use `TANIM_REFLECT_NO_REGISTER` and manually register after globals are ready     |

---

## User Overrides

Three functions you must implement in your project for Tanim to integrate with your engine. Implement these in a `.cpp` file in your project, not in Tanim's code.

### FindEntityOfUID

```cpp
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find);
```

Converts a UID string (from [EntityData::m_uid](#entitydata)) back to an `entt::entity`. Called frequently during playback, when creating sequences in the editor, when recording keyframes, and when inspecting animated values.

**Return**: The matching `entt::entity`, or `entt::null` if not found. Never throw exceptions or crash; Tanim will log the error and skip animation for that sequence.

**Performance**: Use [ComponentData::m_user_data](#componentdata) to cache entity lookups. This function is called every frame per sequence during playback. See [Performance](#performance) for optimization tips.

#### Implementation Pattern 1: Simple Name-Based Lookup

```cpp
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data is empty when finding uid: " + uid_to_find);
        return entt::null;
    }

    entt::entity root = std::any_cast<entt::entity>(cdata.m_user_data);

    if (GetEntityName(root) == uid_to_find)
        return root;

    for (entt::entity child : GetAllChildrenRecursive(root))
    {
        if (GetEntityName(child) == uid_to_find)
            return child;
    }

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

#### Implementation Pattern 2: Cached Entity List (Recommended)

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

#### Implementation Pattern 3: Hash Map for Large Hierarchies

```cpp
struct TanimUserData
{
    entt::entity root_entity;
    std::unordered_map<std::string, entt::entity> uid_to_entity;
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

    auto it = user_data->uid_to_entity.find(uid_to_find);
    if (it != user_data->uid_to_entity.end())
        return it->second;

    LogError("Entity not found for uid: " + uid_to_find);
    return entt::null;
}
```

#### Setting Up User Data

Populate [ComponentData::m_user_data](#componentdata) before calling [OpenForEditing()](#tanimopenforediting) or [UpdateTimeline()](#tanimupdatetimeline):

```cpp
void PrepareAnimationComponent(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    TanimUserData user_data;
    user_data.root_entity = entity;
    for (entt::entity child : GetAllChildrenRecursive(entity))
    {
        user_data.cached_entities.push_back(child);
    }

    anim.component_data.m_user_data = user_data;
}
```

See [Example Implementation](example-implementation.md) for a complete working example with caching patterns.

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

| Message                                                         | Meaning                        |
| --------------------------------------------------------------- | ------------------------------ |
| `"FindEntityOfUID with the uid of [uid] returned entt::null"`   | Entity not found during lookup |
| `"entity [id] does not have a component named [Component]"`     | Component missing on entity    |
| `"Couldn't find any entity with matching details: [name]"`      | Sequence target not found      |
| `"Versions prior to 2 are not supported. Can not deserialize."` | Old serialization format       |

---

## API Reference

### Initialization

#### tanim::Init

```cpp
void Init();
```

Initializes the Tanim system. Call once at startup after ImGui initialization, before any other Tanim functions. Only call this once during your application's lifetime.

### Editor Integration

#### tanim::Draw

```cpp
void Draw();
```

Renders the Tanim editor window. Call every frame between `ImGui::NewFrame()` and `ImGui::Render()`. Safe to call even when no timeline is open. The Tanim window will only appear if a timeline has been opened with [OpenForEditing()](#tanimopenforediting).

#### tanim::UpdateEditor

```cpp
void UpdateEditor(float dt);
```

Updates time-based editor operations and handles per-frame editor logic. Call every frame before [Draw()](#tanimdraw).

### Timeline Editing

#### tanim::OpenForEditing

```cpp
void OpenForEditing(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata);
```

Opens the Tanim editor window to edit a specific timeline. Only one timeline can be open at a time; opening a new one automatically closes the previous.

Store any necessary data in [cdata.m_user_data](#componentdata) before calling so that your [FindEntityOfUID](#findentityofuid) implementation can access it.

The `entity_datas` vector determines which entities appear in the sequence creation menu. See [EntityData](#entitydata) for details.

#### tanim::CloseEditor

```cpp
void CloseEditor();
```

Closes the editor window. **Must be called before destroying TimelineData or ComponentData** to prevent crashes from null pointer access. Safe to call even if no timeline is open.

Call this when switching scenes, unloading timeline data, or removing the animation component from an entity.

### Play Mode Control

#### tanim::EnterPlayMode / tanim::ExitPlayMode

```cpp
void EnterPlayMode();
void ExitPlayMode();
```

Signal play mode transitions. Call once when your application transitions between editor and play modes.

- `EnterPlayMode()` must be called before [StartTimeline()](#tanimstarttimeline) for any entity
- `ExitPlayMode()` must be called after [StopTimeline()](#tanimstoptimeline) for all entities

In a release build (non-editor), call `EnterPlayMode()` once at startup and `ExitPlayMode()` at shutdown.

### Timeline Playback

#### tanim::StartTimeline

```cpp
void StartTimeline(const TimelineData& tdata, ComponentData& cdata);
```

Prepares a timeline for playback. Call after [EnterPlayMode()](#tanimenterplaymode--tanimexitplaymode), before [UpdateTimeline()](#tanimupdatetimeline).

Does not automatically start playback; call [Play()](#playback-control) to begin. If `m_play_immediately` is enabled in TimelineData settings, playback will start automatically.

#### tanim::UpdateTimeline

```cpp
void UpdateTimeline(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata,
                    float delta_time);
```

Advances playback, samples curves, and writes values to components. Call every frame during play mode for each entity with an active timeline.

The `entity_datas` should match what was used in [OpenForEditing()](#tanimopenforediting). Consider caching this vector in [ComponentData::m_user_data](#componentdata) for better performance. See [Performance](#performance) for optimization tips.

#### tanim::StopTimeline

```cpp
void StopTimeline(ComponentData& cdata);
```

Stops playback and resets time to the beginning. Call before [ExitPlayMode()](#tanimenterplaymode--tanimexitplaymode).

### Playback Control

```cpp
void Play(ComponentData& cdata);     // Start or resume playback
void Pause(ComponentData& cdata);    // Pause at current time (Play() resumes from here)
void Stop(ComponentData& cdata);     // Stop and reset to beginning
bool IsPlaying(const ComponentData& cdata);  // Check playback state
```

These functions control playback for individual entities. Since each entity has its own [ComponentData](#componentdata), you can control them independently or call the same function on multiple entities to synchronize their playback.

### Serialization

```cpp
std::string Serialize(TimelineData& tdata);
void Deserialize(TimelineData& tdata, const std::string& serialized_string);
```

Save and load [TimelineData](#timelinedata). The string format is internal to Tanim (currently JSON-based).

**Note**: These are expensive operations. Only call when actually saving/loading, not every frame. Tanim handles versioning internally; if you try to load an unsupported old format, an error will be reported through [LogError](#logerror--loginfo).

### Component Registration

```cpp
template <typename T>
void RegisterComponent();
```

Manually registers a component with Tanim's type system. Only needed for components using [TANIM_REFLECT_NO_REGISTER](#tanim_reflect_no_register). Call after [tanim::Init()](#taniminit), before creating sequences for that component.

### Function Call Order

**Startup:**

```
ImGui::CreateContext() -> tanim::Init()
```

**Editor Frame:**

```
ImGui::NewFrame() -> tanim::UpdateEditor(dt) -> tanim::Draw() -> ImGui::Render()
```

**Entering Play Mode:**

```
tanim::EnterPlayMode() -> tanim::StartTimeline() for each entity
```

**Play Mode Frame:**

```
tanim::UpdateTimeline() for each entity
```

**Exiting Play Mode:**

```
tanim::StopTimeline() for each entity -> tanim::ExitPlayMode()
```

### Threading

Tanim is **not thread-safe**. All API functions must be called from the same thread (typically your main thread). This includes all lifecycle functions, [user override implementations](#user-overrides), and component registration.

### Error Handling

Tanim reports errors through your [LogError](#logerror--loginfo) implementation rather than throwing exceptions or crashing. When errors occur (entity not found, component missing, etc.), Tanim logs the error and skips animation for that sequence. Your application continues running.

---

## Performance

Tanim has **O(n) linear time complexity** where n is the number of animated entities.

### Scalability Test Results

Tests conducted using Superluminal profiler on release configuration:

| Animated Entity Count | Time Spent (ms) |
| :-------------------: | :-------------: |
|          10           |      0.024      |
|          100          |      0.094      |
|         1,000         |      0.987      |
|        10,000         |     10.152      |
|        100,000        |      107.8      |

**Summary:**

- **Per entity**: ~1ms per 1000 entities
- **60 FPS budget** (~16.67ms): ~15,000 animated entities maximum

### Best Practices

| Practice                             | Benefit                                                                                             |
| ------------------------------------ | --------------------------------------------------------------------------------------------------- |
| Cache entity lists in `m_user_data`  | Avoid rebuilding lists every frame; see [User Overrides](#user-overrides) for patterns              |
| Share `TimelineData` across entities | Reduce memory, maintain consistency; see [Timeline Sharing](#timeline-sharing-and-reusability)      |
| Optimize `FindEntityOfUID`           | Called frequently during playback; use cached lookups or hash maps                                  |
| Limit animated entities              | Only animate visible/relevant entities                                                              |
| Use simpler types when possible      | `float` is faster than `glm::quat` (1 curve vs 5 curves); see [Supported Types](supported-types.md) |
| Serialize only when saving           | Expensive operation; don't call every frame                                                         |

See [Example Implementation](example-implementation.md) for complete caching patterns and optimal integration.
