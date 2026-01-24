# Reflection System

Documentation for making components animatable using Tanim's reflection macros.

## Overview

Tanim uses compile-time reflection to access component fields for animation. You must explicitly mark which components and fields should be animatable using reflection macros.

The reflection system is based on the `visit_struct` library and provides two macros:

- **TANIM_REFLECT**: Reflects a component and automatically registers it with Tanim
- **TANIM_REFLECT_NO_REGISTER**: Reflects a component without automatic registration (requires manual registration)

---

## TANIM_REFLECT

```cpp
TANIM_REFLECT(STRUCT_NAME, field1, field2, field3, ...);
```

### Purpose

Reflects a component to Tanim and automatically registers it during application startup. This is the recommended way to make components animatable.

### Parameters

- `STRUCT_NAME`: The fully qualified name of the component struct, including namespaces
- `field1, field2, ...`: Names of the fields you want to make animatable (must be supported types)

### Usage

Call this macro in the **global namespace**, typically in your component header files after the struct definition:

```cpp
namespace MyEngine
{
struct Transform
{
    glm::vec3 position{0.0f};
    glm::quat rotation{glm::identity<glm::quat>()};
    glm::vec3 scale{1.0f};
};
}  // namespace MyEngine

// In global namespace
TANIM_REFLECT(MyEngine::Transform, position, rotation, scale);
```

### Important Notes

**Namespace Qualification**: Always include the full namespace path in `STRUCT_NAME`. Don't use `using namespace` before the macro.

**Global Namespace Only**: The macro must be called in the global namespace, not inside your namespace block.

**Supported Types Only**: Only include fields with types supported by Tanim. See [Supported Types](../supported-types.md) for the complete list.

**Automatic Registration**: The component is automatically registered with Tanim when the application starts. No need to call `RegisterComponent()`.

### Examples

#### Simple Component

```cpp
namespace MyGame
{
struct Light
{
    glm::vec4 color{1.0f};
    float intensity{1.0f};
    bool enabled{true};
};
}

TANIM_REFLECT(MyGame::Light, color, intensity, enabled);
```

#### Subset of Fields

You don't need to reflect all fields - only the ones you want to animate:

```cpp
namespace MyGame
{
struct Camera
{
    float fov{45.0f};           // Will be animatable
    float near_plane{0.1f};     // Will be animatable
    float far_plane{1000.0f};   // Will be animatable

    glm::mat4 projection;       // Not reflected, won't be animatable
    glm::mat4 view;             // Not reflected, won't be animatable
};
}

TANIM_REFLECT(MyGame::Camera, fov, near_plane, far_plane);
```

#### Nested Namespaces

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

### What Happens Behind the Scenes

`TANIM_REFLECT` does two things:

1. Calls `VISITABLE_STRUCT_IN_CONTEXT` to make the component's fields accessible via `visit_struct`
2. Automatically registers the component with Tanim's type system during static initialization

The registration happens before `main()` is called, so by the time you call `tanim::Init()`, the component is already available.

---

## TANIM_REFLECT_NO_REGISTER

```cpp
TANIM_REFLECT_NO_REGISTER(STRUCT_NAME, field1, field2, field3, ...);
```

### Purpose

Reflects a component to Tanim without automatic registration. You must manually call `tanim::RegisterComponent<T>()` later during initialization.

### When to Use

Use this macro only if `TANIM_REFLECT` causes initialization order issues for a specific component. This can happen if:

- The component's constructor accesses global variables that aren't initialized yet
- The component has static initialization dependencies
- You need explicit control over when registration occurs

**Important**: This is a per-component decision. You can use `TANIM_REFLECT` for most components and `TANIM_REFLECT_NO_REGISTER` only for problematic ones.

### Parameters

Same as `TANIM_REFLECT`:

- `STRUCT_NAME`: The fully qualified name of the component struct, including namespaces
- `field1, field2, ...`: Names of the fields you want to make animatable

### Usage

```cpp
namespace MyEngine
{
struct ProblematicComponent
{
    float value{0.0f};

    ProblematicComponent()
    {
        // Constructor accesses some global that might not be initialized yet
        value = SomeGlobalConfig::GetDefaultValue();
    }
};
}

// Reflect without auto-registration
TANIM_REFLECT_NO_REGISTER(MyEngine::ProblematicComponent, value);
```

Then manually register during your initialization:

```cpp
void InitializeEngine()
{
    // Initialize your globals/config first
    SomeGlobalConfig::Initialize();

    // Initialize ImGui
    ImGui::CreateContext();

    // Initialize Tanim
    tanim::Init();

    // Manually register the component
    tanim::RegisterComponent<MyEngine::ProblematicComponent>();
}
```

### Important Notes

**Manual Registration Required**: You must call `tanim::RegisterComponent<T>()` for each component that uses `TANIM_REFLECT_NO_REGISTER`.

**Registration Timing**: Call `RegisterComponent()` after `tanim::Init()` but before trying to create sequences for that component.

**Per-Component**: You only need to use this for specific components that have initialization issues, not for all components.

---

## RegisterComponent

```cpp
template <typename T>
void RegisterComponent();
```

### Purpose

Manually registers a component with Tanim's type system. Only needed for components that use `TANIM_REFLECT_NO_REGISTER`.

### Template Parameters

- `T`: The component type to register

### When to Call

- After `tanim::Init()`
- Before creating sequences for this component type
- During your application's initialization phase

### Usage Example

```cpp
void InitializeApplication()
{
    ImGui::CreateContext();
    tanim::Init();

    // Manually register components that used TANIM_REFLECT_NO_REGISTER
    tanim::RegisterComponent<MyEngine::ProblematicComponent>();
    tanim::RegisterComponent<MyEngine::AnotherSpecialComponent>();
}
```

### Important Notes

**Not Needed for TANIM_REFLECT**: Components that use `TANIM_REFLECT` are automatically registered. Calling `RegisterComponent()` on them is harmless but unnecessary.

**Call Once**: Only call this once per component type during initialization.

**Order Matters**: Must be called after `tanim::Init()`.

---

## Reflection Requirements

### Field Type Support

Only fields with these types can be reflected:

- `float`
- `int`
- `bool`
- `glm::vec2`
- `glm::vec3`
- `glm::vec4`
- `glm::quat`

See [Supported Types](../supported-types.md) for details on type-specific behaviors.

### Field Access

**Public vs Private Fields**: By default, only public fields can be reflected. However, you can reflect private or protected fields by using the `BEFRIEND_VISITABLE()` macro inside your component:

```cpp
namespace MyEngine::components
{
struct CTransform
{
    glm::vec3 m_pos{0.0f};  // Public field

private:
    BEFRIEND_VISITABLE();  // Allows visit_struct to access private fields
    glm::quat m_rot{glm::identity<glm::quat>()};  // Private but animatable
    glm::vec3 m_scl{1.0f};                        // Private but animatable
};
}  // namespace MyEngine::components

TANIM_REFLECT(MyEngine::components::CTransform, m_pos, m_rot, m_scl);
```

This macro is provided by Tanim in `reflection_macro.hpp` and allows `visit_struct` to access private members for reflection.

**No Static Fields**: Static fields cannot be reflected because they are shared across all instances of a component. Animation works on a per-entity basis. When you animate a component on one entity, it should not affect the same component on other entities. Static fields would be animated globally, affecting all entities simultaneously, which breaks the per-entity animation model.

**No Functions**: Member functions cannot be reflected because animation requires actual data storage (memory locations) to write values to. Functions are executable code, not data. Animation works by sampling curves and writing values directly to component fields. There's no field to write to if you specify a function. Also, VisitStruct doesn't support function reflection.

---

## Common Patterns

### Reflecting Multiple Components

```cpp
namespace MyEngine
{
struct Transform { glm::vec3 position, scale; glm::quat rotation; };
struct Light { glm::vec4 color; float intensity; };
struct Camera { float fov; };
}

TANIM_REFLECT(MyEngine::Transform, position, rotation, scale);
TANIM_REFLECT(MyEngine::Light, color, intensity);
TANIM_REFLECT(MyEngine::Camera, fov);
```

### Mixing Reflected and Non-Reflected Fields

```cpp
struct PlayerController
{
    // Animatable fields
    float move_speed{5.0f};
    float jump_force{10.0f};

    // Not reflected - won't be animatable
    bool is_grounded{false};
    glm::vec3 input_direction{0.0f};
};

TANIM_REFLECT(PlayerController, move_speed, jump_force);
// is_grounded and input_direction are not included, so they can't be animated
```

### Shared Header Files

If your component is defined in a header included by multiple translation units, the reflection macro should also be in the header (in global namespace):

```cpp
// Transform.h
#pragma once
#include <glm/glm.hpp>
#include <tanim/tanim.hpp>

namespace MyEngine
{
struct Transform
{
    glm::vec3 position{0.0f};
    glm::quat rotation{glm::identity<glm::quat>()};
    glm::vec3 scale{1.0f};
};
}

// Macro in global namespace, still in the header
TANIM_REFLECT(MyEngine::Transform, position, rotation, scale);
```

---

## Troubleshooting

### Compile Error: "No matching function for call to 'visit_struct::apply_visitor'"

**Cause**: You forgot to call the reflection macro, or called it incorrectly.

**Solution**: Ensure `TANIM_REFLECT` or `TANIM_REFLECT_NO_REGISTER` is called in the global namespace with the correct struct name and field names.

### Component Not Appearing in Editor

**Cause**: Component not registered, or fields don't have supported types.

**Solution**:

- Verify you used `TANIM_REFLECT` or called `RegisterComponent()`
- Check that all reflected fields have supported types
- Ensure the macro is in the global namespace

### Initialization Order Crash

**Cause**: Component constructor accesses uninitialized globals during static initialization.

**Solution**: Use `TANIM_REFLECT_NO_REGISTER` instead and manually register after your globals are initialized.

### Multiple Definition Errors

**Cause**: Reflection macro called more than once for the same component type.

**Error Message**:

```
Error C2766: explicit specialization; 'visit_struct::traits::visitable<MyEngine::Component,tanim::VSContext>' has already been defined
```

**Solution**: Ensure the reflection macro is only called once per component type. Use include guards and keep the macro in the header file, not in multiple `.cpp` files.

---

## Best Practices

**Use TANIM_REFLECT by default**: Only use `TANIM_REFLECT_NO_REGISTER` when you encounter actual initialization issues.

**Reflect only what you need**: Don't reflect fields you don't plan to animate. It reduces compile time and avoids clutter in the editor.

**Keep macros in headers**: Put reflection macros in the same header file as the component definition.

---

## Next Steps

- [Supported Types](../supported-types.md) - Learn which types can be animated
- [Lifecycle Functions](lifecycle.md) - Use reflected components in animations
- [Example Implementation](../example-implementation.md) - See complete reflection examples
