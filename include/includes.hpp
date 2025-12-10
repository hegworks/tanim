#pragma once

#include <imgui/imgui.h>                  // tested with version 1.92.3
#include <imgui/imgui_internal.h>         // tested with version 1.92.3
#include <visit_struct/visit_struct.hpp>  // minimum version 1.2.0
#include <entt/entt.hpp>                  // tested with version 3.15.0
#include <magic_enum/magic_enum.hpp>      // tested with version 0.9.7

#define USE_GLM

#ifdef USE_GLM
#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#endif
