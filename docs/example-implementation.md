# Example Implementation

Complete integration example showing how to use Tanim in a real application with ImGui and ENTT.

## Overview

This example demonstrates:

- Component setup and resource management
- User override implementations
- Editor integration
- Runtime playback integration
- Component reflection
- Entity data management

---

## Project Structure

```
MyEngine/
├── Components/
│   ├── AnimationComponent.h      // Component with TimelineData
│   ├── Transform.h                // Example animated component
│   └── Light.h                    // Example animated component
├── Resources/
│   └── TimelineResource.h         // Resource management for timelines
├── TanimIntegration.cpp           // User overrides implementation
├── Editor/
│   └── InspectorPanel.cpp         // Editor UI for animation
└── Systems/
    └── AnimationSystem.cpp        // Runtime playback system
```

---

## 1. Component Setup

### Animation Component

```cpp
// AnimationComponent.h
#pragma once
#include <tanim/tanim.hpp>

namespace MyEngine
{

struct AnimationComponent
{
    // Reference to shared timeline data (can be a resource handle, shared_ptr, etc.)
    ResourceHandle<TimelineResource> timeline_resource;

    // Runtime playback state (unique per entity)
    tanim::ComponentData component_data;
};

}  // namespace MyEngine

// Reflect for engine serialization (not Tanim)
REFLECT(MyEngine::AnimationComponent, timeline_resource);
```

### Timeline Resource

```cpp
// TimelineResource.h
#pragma once
#include <tanim/tanim.hpp>
#include "Resources/Resource.h"

namespace MyEngine
{

class TimelineResource : public Resource
{
public:
    tanim::TimelineData timeline_data;

    TimelineResource(const std::string& name, const std::string& filepath)
        : Resource(name, filepath)
    {
    }
};

}  // namespace MyEngine
```

---

## 2. Example Animated Components

### Transform Component

```cpp
// Transform.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <tanim/tanim.hpp>

namespace MyEngine
{

struct Transform
{
    glm::vec3 position{0.0f};
    glm::quat rotation{glm::identity<glm::quat>()};
    glm::vec3 scale{1.0f};
};

}  // namespace MyEngine

// Reflect to Tanim
TANIM_REFLECT(MyEngine::Transform, position, rotation, scale);
```

### Light Components

```cpp
// Light.h
#pragma once
#include <glm/glm.hpp>
#include <tanim/tanim.hpp>

namespace MyEngine
{

struct PointLight
{
    bool enabled{true};
    float radius{10.0f};
    glm::vec4 color{1.0f};
    float intensity{1.0f};
};

struct DirectionalLight
{
    bool enabled{true};
    glm::vec4 color{1.0f};
    float intensity{1.0f};
};

}  // namespace MyEngine

TANIM_REFLECT(MyEngine::PointLight, enabled, radius, color, intensity);
TANIM_REFLECT(MyEngine::DirectionalLight, enabled, color, intensity);
```

---

## 3. User Overrides Implementation

```cpp
// TanimIntegration.cpp
#include <tanim/tanim.hpp>
#include "Engine.h"

namespace MyEngine
{

// User data structure for FindEntityOfUID
struct TanimUserData
{
    entt::entity root_entity{entt::null};
    std::vector<tanim::EntityData> cached_entities_data;
    std::vector<entt::entity> cached_entities;
};

// Helper function to convert entity to UID string
std::string EntityToUID(entt::entity entity)
{
    // Use entity names as UIDs
    auto& name_component = Engine::Registry().get<NameComponent>(entity);
    return name_component.name;
}

}  // namespace MyEngine

// Implement required user overrides
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid_to_find)
{
    using namespace MyEngine;

    if (!cdata.m_user_data.has_value())
    {
        LogError("m_user_data had no value when finding uid " + uid_to_find);
        return entt::null;
    }

    const auto* user_data = std::any_cast<TanimUserData>(&cdata.m_user_data);
    if (!user_data)
    {
        LogError("m_user_data could not cast to TanimUserData when finding uid " + uid_to_find);
        return entt::null;
    }

    if (user_data->root_entity == entt::null)
    {
        LogError("root_entity in m_user_data was entt::null when finding uid " + uid_to_find);
        return entt::null;
    }

    // Check root entity
    const std::string root_uid = EntityToUID(user_data->root_entity);
    if (root_uid == uid_to_find)
    {
        return user_data->root_entity;
    }

    // Check cached children
    for (const auto& child_entity : user_data->cached_entities)
    {
        if (EntityToUID(child_entity) == uid_to_find)
        {
            return child_entity;
        }
    }

    LogError("no entity was found when finding uid " + uid_to_find);
    return entt::null;
}

void tanim::LogError(const std::string& message)
{
    MyEngine::Logger::Error("[TANIM] " + message);
}

void tanim::LogInfo(const std::string& message)
{
    MyEngine::Logger::Info("[TANIM] " + message);
}
```

---

## 4. Entity Data Management

Helper functions for building entity lists:

```cpp
// EntityHelpers.cpp
#include "EntityHelpers.h"

namespace MyEngine
{

std::string GetEntityName(entt::entity entity)
{
    auto& name = Engine::Registry().get<NameComponent>(entity).name;
    return name;
}

std::vector<entt::entity> GetAllChildren(entt::entity root)
{
    std::vector<entt::entity> children;

    // Get transform hierarchy
    auto& transform = Engine::Registry().get<Transform>(root);

    // Recursively add all children
    // (Implementation depends on your hierarchy system)
    // ...

    return children;
}

std::vector<tanim::EntityData> BuildEntityDataList(entt::entity root)
{
    std::vector<tanim::EntityData> entity_datas;

    // Add root
    entity_datas.push_back({
        .m_uid = GetEntityName(root),
        .m_display = GetEntityName(root)
    });

    // Add all children
    for (entt::entity child : GetAllChildren(root))
    {
        entity_datas.push_back({
            .m_uid = GetEntityName(child),
            .m_display = GetEntityName(child)
        });
    }

    return entity_datas;
}

TanimUserData& UpdateCachedDataIfEmpty(tanim::ComponentData& cdata, entt::entity root_entity)
{
    if (!cdata.m_user_data.has_value())
    {
        TanimUserData user_data;
        user_data.root_entity = root_entity;
        user_data.cached_entities_data = BuildEntityDataList(root_entity);
        user_data.cached_entities = GetAllChildren(root_entity);

        cdata.m_user_data = user_data;
    }

    return std::any_cast<TanimUserData&>(cdata.m_user_data);
}

}  // namespace MyEngine
```

---

## 5. Editor Integration

### Inspector Panel - Timeline Editing

```cpp
// InspectorPanel.cpp
void InspectorPanel::Render()
{
    ImGui::Begin("Inspector");

    if (m_selected_entity == entt::null)
    {
        ImGui::Text("No entity selected.");
        ImGui::End();
        return;
    }

    // ... other component UI ...

    // Animation component UI
    if (registry.all_of<AnimationComponent>(m_selected_entity))
    {
        RenderAnimationComponent(m_selected_entity);
    }

    ImGui::End();
}

void InspectorPanel::RenderAnimationComponent(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Timeline resource selection
        bool has_timeline = ResourceManager::Exists(anim.timeline_resource);

        if (ImGui::Button("Select Timeline"))
        {
            // Show resource picker
            // ...
        }

        if (has_timeline)
        {
            auto& timeline = ResourceManager::Get(anim.timeline_resource);
            ImGui::Text("Timeline: %s", timeline.name.c_str());

            // Create new timeline
            if (ImGui::Button("Create Timeline"))
            {
                std::string filepath = FileDialog::SaveFile("Timeline (*.tanim)|*.tanim");
                if (!filepath.empty())
                {
                    CreateTimeline(entity, filepath);
                }
            }

            // Load existing timeline
            if (ImGui::Button("Load Timeline"))
            {
                std::string filepath = FileDialog::OpenFile("Timeline (*.tanim)|*.tanim");
                if (!filepath.empty())
                {
                    LoadTimeline(entity, filepath);
                }
            }

            // Edit timeline
            if (ImGui::Button("Edit Timeline"))
            {
                OpenTimelineEditor(entity);
            }

            // Save timeline
            if (ImGui::Button("Save Timeline"))
            {
                SaveTimeline(entity);
            }
        }
    }
}

void InspectorPanel::CreateTimeline(entt::entity entity, const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    // Create resource
    auto timeline_resource = ResourceManager::Create<TimelineResource>(filepath);
    anim.timeline_resource = timeline_resource;

    // Save empty timeline
    SaveTimeline(entity);
}

void InspectorPanel::LoadTimeline(entt::entity entity, const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    // Load or get existing resource
    anim.timeline_resource = ResourceManager::Load<TimelineResource>(filepath);

    auto& timeline = ResourceManager::Get(anim.timeline_resource);

    // Deserialize timeline data
    std::string serialized = FileSystem::ReadFile(filepath);
    tanim::Deserialize(timeline.timeline_data, serialized);
}

void InspectorPanel::SaveTimeline(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    auto& timeline = ResourceManager::Get(anim.timeline_resource);

    // Serialize timeline data
    std::string serialized = tanim::Serialize(timeline.timeline_data);

    // Save to file
    FileSystem::WriteFile(timeline.filepath, serialized);
}

void InspectorPanel::OpenTimelineEditor(entt::entity entity)
{
    auto& anim = registry.get<AnimationComponent>(entity);
    auto& timeline = ResourceManager::Get(anim.timeline_resource);

    // Update cached user data
    TanimUserData& user_data = UpdateCachedDataIfEmpty(anim.component_data, entity);

    // Open editor
    tanim::OpenForEditing(registry,
                         user_data.m_cached_entities_data,
                         timeline.timeline_data,
                         anim.component_data);
}
```

### Handling Scene Changes

```cpp
// SceneManager.cpp
void SceneManager::UnloadScene()
{
    // Close Tanim editor before unloading
    tanim::CloseEditor();

    // Unload scene resources
    // ...
}

void SceneManager::OnComponentRemoved(entt::entity entity, AnimationComponent& anim)
{
    // Close editor if this component was being edited
    tanim::CloseEditor();
}
```

---

## 6. Engine Initialization

```cpp
// Engine.cpp
void Engine::Initialize()
{
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Initialize ImGui backend
    // ...

    // Initialize Tanim
    tanim::Init();

    // Component reflection happens automatically via TANIM_REFLECT macros
}

void Engine::EditorFrame(float delta_time)
{
    ImGui::NewFrame();

    // Update Tanim editor
    tanim::UpdateEditor(delta_time);

    // Render editor panels
    // ...

    // Render Tanim window
    tanim::Draw();

    ImGui::Render();
}
```

---

## 7. Runtime Playback System

```cpp
// AnimationSystem.cpp
namespace MyEngine
{

class AnimationSystem
{
public:
    void OnEnterPlayMode(entt::registry& registry)
    {
        // Notify Tanim we're entering play mode
        tanim::EnterPlayMode();

        // Start all timelines
        auto view = registry.view<AnimationComponent>();
        for (auto entity : view)
        {
            auto& anim = view.get<AnimationComponent>(entity);

            if (!ResourceManager::Exists(anim.timeline_resource))
                continue;

            auto& timeline = ResourceManager::Get(anim.timeline_resource);

            // Reset user data to ensure fresh hierarchy state
            anim.component_data.m_user_data.reset();

            // Start timeline
            tanim::StartTimeline(timeline.timeline_data, anim.component_data);
        }
    }

    void Update(entt::registry& registry, float delta_time)
    {
        auto view = registry.view<AnimationComponent>();

        for (auto entity : view)
        {
            auto& anim = view.get<AnimationComponent>(entity);

            if (!ResourceManager::Exists(anim.timeline_resource))
                continue;

            auto& timeline = ResourceManager::Get(anim.timeline_resource);

            // Update cached data if empty (handles hierarchy changes during play)
            TanimUserData& user_data = UpdateCachedDataIfEmpty(anim.component_data, entity);

            // Update timeline
            tanim::UpdateTimeline(registry,
                                user_data.m_cached_entities_data,
                                timeline.timeline_data,
                                anim.component_data,
                                delta_time);
        }
    }

    void OnExitPlayMode(entt::registry& registry)
    {
        // Stop all timelines
        auto view = registry.view<AnimationComponent>();
        for (auto entity : view)
        {
            auto& anim = view.get<AnimationComponent>(entity);
            tanim::StopTimeline(anim.component_data);
        }

        // Notify Tanim we're exiting play mode
        tanim::ExitPlayMode();
    }
};

}  // namespace MyEngine
```

---

## 8. Manual Playback Control

```cpp
// GameplayController.cpp
void GameplayController::OnTriggerEnter(entt::entity entity)
{
    // Play animation when trigger is entered
    if (registry.all_of<AnimationComponent>(entity))
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::Play(anim.component_data);
    }
}

void GameplayController::OnTriggerExit(entt::entity entity)
{
    // Stop animation when trigger is exited
    if (registry.all_of<AnimationComponent>(entity))
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::Stop(anim.component_data);
    }
}

void GameplayController::CheckAnimationState(entt::entity entity)
{
    if (registry.all_of<AnimationComponent>(entity))
    {
        auto& anim = registry.get<AnimationComponent>(entity);

        if (tanim::IsPlaying(anim.component_data))
        {
            // Animation is playing
        }
        else
        {
            // Animation is paused or stopped
        }
    }
}
```

---

## 9. Complete Main Loop

```cpp
// Main.cpp
int main()
{
    Engine engine;
    engine.Initialize();

    while (engine.IsRunning())
    {
        float delta_time = engine.GetDeltaTime();

        // Editor mode
        if (engine.IsInEditorMode())
        {
            engine.EditorFrame(delta_time);
        }

        // Play mode
        if (engine.IsInPlayMode())
        {
            engine.AnimationSystem().Update(engine.Registry(), delta_time);
            engine.Update(delta_time);
        }

        engine.Render();
    }

    return 0;
}
```

---

## Key Takeaways

**Resource Management**: Timeline data is stored in resources that can be shared across multiple entities. Each entity has its own runtime state (`ComponentData`).

**User Data Pattern**: Cache entity lists and hierarchy information in `ComponentData::m_user_data` for efficient lookup in `FindEntityOfUID`.

**Editor Integration**: Use Tanim's editor functions to provide timeline editing UI in your inspector/property panels.

**Playback System**: Separate play mode initialization (`EnterPlayMode`, `StartTimeline`) from per-frame updates (`UpdateTimeline`).

**Scene Management**: Always call `tanim::CloseEditor()` before unloading scenes or removing animation components to prevent crashes.

**Entity Identification**: Use persistent identifiers (entity names) as UIDs rather than ENTT entity IDs which can change between sessions.

---

## Next Steps

- Review [API Reference](api-reference/overview.md) for detailed function documentation
- Check [Supported Types](supported-types.md) for type-specific behaviors
- See [UI & Shortcuts](ui-shortcuts.md) for editor usage
- Read [Performance](performance.md) for optimization tips
