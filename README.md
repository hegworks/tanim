> [!WARNING]  
> This project is a WIP and is being developed during my studies as a 2nd year Engine & Tools student @ [Breda University of Applied Sciences](https://www.buas.nl/en/programmes/creative-media-and-game-technologies/programming).

# Tanim

**T**imeline **Anim**ation Library for C++ projects based on ImGui & ENTT.

## About

Tanim is a timeline animation library that provides keyframe animation for C++ projects using ImGui and ENTT. It includes a timeline editor with cubic Bezier curve interpolation for the editor interface and runtime playback for animated scenes.

The library provides curve editing, keyframe manipulation, runtime sampling, and serialization. Animation data can be shared across multiple entities for animating hierarchies and creating reusable animation clips.

## Showcase

TODOVISUAL Add showcase video/gif demonstrating timeline editor, curve editing, and runtime playback

## Features

- **Timeline Editor**: ImGui-based interface for creating and editing animation timelines
- **Cubic Bezier Curves**: Industry-standard curve interpolation with configurable tangent modes (Auto, Smooth, Broken, Weighted)
- **Multi-Entity Animation**: Animate entity hierarchies with a single timeline asset
- **Component Reflection**: Simple macro-based system to make components with supported types animatable
- **Keyframe Editing**: Create, move, and delete keyframes with multi-selection support
- **Handle Manipulation**: Direct control over curve tangents with visual feedback
- **Curve Constraints**: Automatic monotonicity enforcement to prevent invalid animations
- **Runtime Playback**: Efficient sampling system with configurable playback modes (Loop, Once)
- **Serialization**: Save and load system for timeline data
- **Type Support**: Built-in support for common types (float, int, bool, glm::vec2, glm::vec3, glm::vec4, glm::quat)
- **Performance**: O(n) time complexity, capable of animating 15,000+ entities at 60 FPS

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

## Documentation

**[📖 Full Documentation Wiki](docs/wiki.md)** - Complete documentation index with links to all guides and references.

### [Getting Started](docs/getting-started.md)

Quick start guide with a minimal working example to get your first animation running.

### [Core Concepts](docs/core-concepts.md)

Understand the architecture and data flow: Components, Timelines, Sequences, Curves, and Keyframes.

### [API Reference](docs/api-reference/overview.md)

Complete API documentation covering:

- [Lifecycle Functions](docs/api-reference/lifecycle.md) - Init, Draw, Update, Play/Pause/Stop
- [Data Structures](docs/api-reference/data-structures.md) - EntityData, TimelineData, ComponentData
- [User Overrides](docs/api-reference/user-overrides.md) - Required function implementations
- [Reflection System](docs/api-reference/reflection.md) - Making components animatable

### [Example Implementation](docs/example-implementation.md)

Complete integration example showing component setup, editor integration, and runtime playback.

### [Supported Types](docs/supported-types.md)

List of built-in animatable types and their behaviors.

### [UI & Shortcuts](docs/ui-shortcuts.md)

Editor controls, keyboard shortcuts, and mouse interactions.

### [Performance](docs/performance.md)

Profiling results, scalability data, and optimization opportunities.

## Quick Example

```cpp
// 1. Initialize Tanim once at startup
tanim::Init();

// 2. Reflect your components (in global namespace)
TANIM_REFLECT(MyComponent, position, rotation, scale);

// 3. Call Draw and UpdateEditor each frame
void OnImGuiRender() {
    tanim::Draw();
    tanim::UpdateEditor(delta_time);
}

// 4. Update timelines during play mode
void OnUpdate() {
    tanim::UpdateTimeline(registry, entity_datas, tdata, cdata, delta_time);
}
```

See [Getting Started](docs/getting-started.md) for the complete setup process.

## Future

- Quality of life improvements for the editor
- Custom type support system
- Event system for animation callbacks
- Standalone Bezier curve editor widget
- Potential Flecs ECS support

## License

MIT
