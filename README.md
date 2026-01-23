> [!WARNING]  
> This project is a WIP and is being developed during my study as a 2nd year Engine & Tools student @ Breda University of Applied Sciences.

# Brief

Tanim is a **T**imeline **Anim**ation Library based on ImGui & Entt.

TODO explain more.

# Prerequisites

Minimum C++ 17.

Tanim is made for projects that are using ImGui for editor GUI, and ENTT for their ECS. So your project must already be using:

|  Library   |      Version       |                  GitHub                  |
| :--------: | :----------------: | :--------------------------------------: |
|    ENTT    | tested with 3.15.0 | [LINK](https://github.com/skypjack/entt) |
| Dear ImGui | tested with 1.92.3 | [LINK](https://github.com/ocornut/imgui) |

Tanim also depends on the libraries below, but just internally; You must add the ones you don't have to your project. Either put them where your other external libraries are, or just add everything inside [the external folder](https://github.com/hegworks/tanim/tree/main/external), individually to your include paths:

|    Library    |       Version       |                     GitHub                      |
| :-----------: | :-----------------: | :---------------------------------------------: |
|      glm      | tested with 0.9.9.9 |      [LINK](https://github.com/g-truc/glm)      |
| visit_struct  |  **minimum** 1.2.0  | [LINK](https://github.com/cbeck88/visit_struct) |
|  magic_enum   |  tested with 0.9.7  |  [LINK](https://github.com/Neargye/magic_enum)  |
| nlohmann/json | tested with 3.12.0  |    [LINK](https://github.com/nlohmann/json)     |

In the end, as an **example** for each library, these includes must be accessible in the code like this:

```
#include <entt/entt.hpp>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <visit_struct/visit_struct.hpp>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
```

> [!NOTE]  
> CMake support is on the way.
