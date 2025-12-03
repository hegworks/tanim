# tanim

Timeline Animation Library based on ImGui & ImGuizmo's ImSequencer.

> [!WARNING]  
> This project is a WIP and is being developed during my study as a 2nd year Engine & Tools student @ Breda University of Applied Sciences.

# How to setup

## Dependencies

|   Library   |           Version           |                                 Link                                  |
| :---------: | :-------------------------: | :-------------------------------------------------------------------: |
| Dear ImGui  | Tested with version: 1.92.3 | [LINK](https://github.com/ocornut/imgui/releases/tag/v1.92.3-docking) |
| VisitStruct |   Minimum version: 1.2.0    |  [LINK](https://github.com/cbeck88/visit_struct/releases/tag/1.2.0)   |
|    ENTT     | Tested with version: 3.15.0 |     [LINK](https://github.com/skypjack/entt/releases/tag/v3.15.0)     |

## Adding the library

1. download a snapshot.
2. move the "tanim" folder to where your external libraries are. don't move the files inside the "tanim" folder around.
3. include all the files inside "tanim/src" in your project.
4. open "tanim/include/incluldes.hpp" and change the address of includes based on your project setup.

## Using the library

- in one of your .cpp files, include tanim like this: `#include <tanim/include/tanim.hpp>`
- call `tanim::Tanim::Init();` once, before other tanim calls; e.g. in your initialization phase.
- call `tanim::Tanim::Draw();` where you call your own imgui draw functions (every frame).
- call `tanim::Tanim::Update(m_raw_delta_time);` in your systems update phase (every frame).
- TODO...

### Component

- TODO explain inspection
