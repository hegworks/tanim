# Getting Started

This guide walks you through integrating Tanim into your project and creating your first animation.

## Step 1: Include Tanim

Add the Tanim header to your project:

```cpp
#include <tanim/tanim.hpp>
```

This single header includes all the API functions and data structures you need.

## Step 2: Initialize Tanim

Call `tanim::Init()` once at application startup, before any other Tanim functions:

```cpp
void OnApplicationStart()
{
    // Initialize ImGui first
    ImGui::CreateContext();
    // ... your ImGui setup

    // Then initialize Tanim
    tanim::Init();
}
```

## Step 3: Implement Required User Overrides

Tanim requires you to implement three functions. These allow Tanim to communicate with your engine without directly depending on it.

Create a `.cpp` file in your project and implement these:

```cpp
#include <tanim/tanim.hpp>

// Convert a UID string back to an entt::entity
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    // Your implementation here
    // Return the entity that matches the uid_to_find
    // Return entt::null if not found
}

// Error logging
void tanim::LogError(const std::string& message)
{
    // Your logging system here
    // Example: std::cerr << "[TANIM ERROR] " << message << std::endl;
}

// Info logging
void tanim::LogInfo(const std::string& message)
{
    // Your logging system here
    // Example: std::cout << "[TANIM INFO] " << message << std::endl;
}
```

See [User Overrides](api-reference/user-overrides.md) for detailed explanations and examples of these functions.

## Step 4: Reflect Your Components

For each component you want to animate, use the `TANIM_REFLECT` macro in the global namespace:

```cpp
// Example component
struct Transform
{
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

// Reflect it to Tanim (in global namespace)
TANIM_REFLECT(Transform, position, rotation, scale);
```

Only include fields with supported types. See [Supported Types](supported-types.md) for the full list.

## Step 5: Create a Component to Hold Timeline Data

Create a component that can be attached to entities you want to animate:

```cpp
struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};
```

- `TimelineData`: Holds the animation curves, keyframes, and timeline settings. Can be shared across entities or serialized to disk.
- `ComponentData`: Holds runtime playback state. Unique per entity.

## Step 6: Integrate with Your Editor Loop

Call Tanim's editor functions in your ImGui render loop:

```cpp
void OnEditorFrame(float delta_time)
{
    ImGui::NewFrame();

    // Your editor panels here

    // Update and draw Tanim editor
    tanim::UpdateEditor(delta_time);
    tanim::Draw();

    ImGui::Render();
}
```

`UpdateEditor` handles time-based editor updates, and `Draw` renders the Tanim window.

## Step 7: Open a Timeline for Editing

When you want to edit a timeline, prepare entity data and open the editor:

```cpp
void OpenTimelineEditor(entt::registry& registry,
                       entt::entity entity_with_animation,
                       AnimationComponent& anim_comp)
{
    // Build list of entities that can be animated in this timeline
    std::vector<tanim::EntityData> entity_datas;

    // Add the root entity
    entity_datas.push_back({
        .m_uid = "RootEntity",      // Unique identifier for this entity
        .m_display = "Root Entity"   // Display name in the editor
    });

    // Add any child entities you want to animate
    // entity_datas.push_back({ ... });

    // Open the editor
    tanim::OpenForEditing(registry,
                         entity_datas,
                         anim_comp.timeline_data,
                         anim_comp.component_data);
}
```

The Tanim editor window will now appear, allowing you to create sequences and keyframes.

TODOVISUAL Add screenshot of Tanim editor window

## Step 8: Play Animations at Runtime

To play animations during your game/application loop:

```cpp
void OnPlayMode()
{
    // Call once when entering play mode
    tanim::EnterPlayMode();

    // For each entity with animations, start its timeline
    tanim::StartTimeline(anim_comp.timeline_data, anim_comp.component_data);
}

void OnUpdate(float delta_time)
{
    // For each entity with animations, update its timeline
    tanim::UpdateTimeline(registry,
                         entity_datas,  // Same list from OpenForEditing
                         anim_comp.timeline_data,
                         anim_comp.component_data,
                         delta_time);
}

void OnExitPlayMode()
{
    // For each entity with animations, stop its timeline
    tanim::StopTimeline(anim_comp.component_data);

    // Call once when exiting play mode
    tanim::ExitPlayMode();
}
```

## Minimal Complete Example

Here's a minimal working example:

```cpp
#include <tanim/tanim.hpp>
#include <entt/entt.hpp>
#include <imgui/imgui.h>

// Component to animate
struct Transform
{
    glm::vec3 position{0.0f};
};
TANIM_REFLECT(Transform, position);

// Component to hold animation data
struct AnimationComponent
{
    tanim::TimelineData timeline_data;
    tanim::ComponentData component_data;
};

int main()
{
    // Initialize
    ImGui::CreateContext();
    tanim::Init();

    entt::registry registry;
    entt::entity entity = registry.create();
    registry.emplace<Transform>(entity);
    registry.emplace<AnimationComponent>(entity);

    // Game loop
    while (running)
    {
        float delta_time = GetDeltaTime();

        ImGui::NewFrame();
        tanim::UpdateEditor(delta_time);
        tanim::Draw();
        ImGui::Render();

        // Render, swap buffers, etc.
    }

    return 0;
}

// User override implementations
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    // Implementation here
    return entt::null;
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

## Next Steps

- Read [Core Concepts](core-concepts.md) to understand Tanim's architecture
- Check [API Reference](api-reference/overview.md) for detailed function documentation
- See [Example Implementation](example-implementation.md) for a complete integration example
- Learn about [Supported Types](supported-types.md) and their special behaviors

## Common Issues

**Tanim editor window doesn't appear**: Make sure you called `tanim::Init()` and `tanim::Draw()` between `ImGui::NewFrame()` and `ImGui::Render()`.

**Animations don't play**: Verify you called `tanim::EnterPlayMode()` and `tanim::StartTimeline()` before calling `tanim::UpdateTimeline()`.

**Components not showing in editor**: Check that you used `TANIM_REFLECT` in the global namespace and included only supported types.

**Entity not found errors**: Verify your `FindEntityOfUID` implementation correctly maps UIDs to entities.
