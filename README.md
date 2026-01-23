> [!WARNING]  
> This project is a WIP and is being developed during my studies as a 2nd year Engine & Tools student @ [Breda University of Applied Sciences](https://www.buas.nl/en/programmes/creative-media-and-game-technologies/programming).

# About

## Showcase

## Brief

Tanim is a **T**imeline **Anim**ation Library based on ImGui & Entt.

TODO explain more.

# Features

# Installation

> [!NOTE]  
> CMake support is on the way, until then, please follow the steps to manually integrate Tanim into your project.

clone, download zip, or download the latest release

## Prerequisites

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

# Getting Started

# Core concepts

- Architecture:
  - Component -> Timeline -> Sequence -> Curve -> Keyframe -> Handle
- EntityData, TimelineData & ComponentData
-

# API docs

# Example implementation

# Supported types

# UI & Shortcuts

# Performance

# Future

- QoL improvements
- custom types
- events
- separated bezier editor
- flecs support (maybe)

# License

MIT
