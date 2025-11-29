# tanim

Timeline Animation Library based on ImGui & ImGuizmo's ImSequencer.

# How to setup

## Dependencies

- Dear ImGui (TODO: specify version)
- [VisitStruct 1.2.0](https://github.com/cbeck88/visit_struct/releases/tag/1.2.0)

## Adding the library

1. download a snapshot.
2. move the "tanim" folder to where your external libraries are. don't move the files inside the "tanim" folder around.
3. include all the files inside "tanim/src" in your project.
4. open "tanim/include/tanimgui_includes.hpp" and change the address of imgui includes based on your project setup.

## Using the library

- in one of your .cpp files, include tanim like this: `#include <tanim/include/tanim.hpp>`
- call `tanim::Tanim::Init();` once, before other tanim calls; e.g. in your initialization phase.
- call `tanim::Tanim::Draw();` where you call your own imgui draw functions (every frame).
- call `tanim::Tanim::Update(m_raw_delta_time);` in your systems update phase (every frame).
