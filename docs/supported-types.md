# Supported Types

Complete reference for all animatable field types in Tanim.

## Overview

Tanim supports the following field types for animation:

- `float` - Single floating-point value
- `int` - Integer value
- `bool` - Boolean value (on/off)
- `glm::vec2` - 2D vector
- `glm::vec3` - 3D vector
- `glm::vec4` - 4D vector / RGBA color
- `glm::quat` - Quaternion rotation

Each type has specific behaviors and curve representations in the editor.

---

## float

**Curve Count**: 1

**Usage**: Single floating-point values like intensity, radius, speed, FOV, etc.

**Example**:

```cpp
struct Light
{
    float intensity{1.0f};
    float radius{10.0f};
};
TANIM_REFLECT(Light, intensity, radius);
```

**Editor Representation**: Single curve in the curve editor.

**Interpolation**: Smooth cubic Bezier interpolation between keyframes.

---

## int

**Curve Count**: 1

**Usage**: Integer values like counts, indices, or discrete states.

**Example**:

```cpp
struct Character
{
    int health{100};
    int level{1};
};
TANIM_REFLECT(Character, health, level);
```

**Editor Representation**: Single curve in the curve editor.

**Interpolation**: Cubic Bezier interpolation with values floored to integers during sampling.

**Special Behavior**: While the curve itself is smooth, the sampled values are always floored to integers. The curve can still have smooth tangents for visual clarity.

---

## bool

**Curve Count**: 1

**Usage**: Boolean flags or toggles.

**Example**:

```cpp
struct Light
{
    bool enabled{true};
};
TANIM_REFLECT(Light, enabled);
```

**Editor Representation**: Single curve in the curve editor.

**Values**: Can only be 0 (false) or 1 (true).

**Interpolation**: Constant - no interpolation between keyframes. The value changes immediately at keyframe positions.

**Special Behavior**:

- Keyframes can only have Y values of 0 or 1
- Handles are always constant (horizontal)
- No smooth interpolation is possible

---

## glm::vec2

**Curve Count**: 2 (x, y)

**Usage**: 2D positions, sizes, or texture coordinates.

**Example**:

```cpp
struct Sprite
{
    glm::vec2 position{0.0f};
    glm::vec2 size{1.0f};
};
TANIM_REFLECT(Sprite, position, size);
```

**Editor Representation**: Two separate curves (x and y) that can be edited independently.

**Interpolation**: Each component interpolates independently using cubic Bezier curves.

---

## glm::vec3

**Curve Count**: 3 (x, y, z)

**Usage**: 3D positions, scales, velocities, or RGB colors.

**Example**:

```cpp
struct Transform
{
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
};
TANIM_REFLECT(Transform, position, scale);
```

**Editor Representation**: Three separate curves (x, y, z) that can be edited independently.

**Interpolation**: Each component interpolates independently using cubic Bezier curves.

### Representation Types

`glm::vec3` can represent different data types. Tanim provides two representations:

**VECTOR** (default): Standard 3D vector with separate x, y, z controls.

**COLOR**: RGB color with a color picker in the inspector.

You can change the representation in the editor when inspecting a sequence. This affects how the values are displayed and edited in the inspector, but doesn't change the underlying animation curves.

**Note**: Color interpolation happens per-component (R, G, B separately), not in perceptual color space. This can produce unexpected hues during interpolation, but allows precise control by adding keyframes where needed.

---

## glm::vec4

**Curve Count**: 4 (x, y, z, w)

**Usage**: 4D data or RGBA colors.

**Example**:

```cpp
struct Light
{
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};
TANIM_REFLECT(Light, color);
```

**Editor Representation**: Four separate curves (x, y, z, w) that can be edited independently.

**Interpolation**: Each component interpolates independently using cubic Bezier curves.

### Representation Types

Similar to `glm::vec3`, `glm::vec4` supports two representations:

**VECTOR** (default): Standard 4D vector with separate x, y, z, w controls.

**COLOR**: RGBA color with a color picker in the inspector (includes alpha channel).

---

## glm::quat

**Curve Count**: 5 (w, x, y, z, spins)

**Usage**: 3D rotations.

**Example**:

```cpp
struct Transform
{
    glm::quat rotation{glm::identity<glm::quat>()};
};
TANIM_REFLECT(Transform, rotation);
```

**Editor Representation**: Five curves displayed in order: w, x, y, z, spins.

**Interpolation**: Spherical Linear Interpolation (slerp) at runtime for proper quaternion blending.

### Why Quaternions Are Different

Quaternions require special handling because their four components (w, x, y, z) are interdependent. You cannot animate each component independently like you can with vectors. Editing one component without considering the others would result in invalid rotations.

### Synchronized Keyframe Operations

Because quaternion components must remain synchronized, keyframe creation and deletion works differently:

**Adding Keyframes**: Use the **+keyframe** button or **record** button to create keyframes on all five curves (w, x, y, z, spins) simultaneously at the same time.

**Removing Keyframes**: Use the **-keyframe** button to remove keyframes from all five curves at once.

**Individual Operations Restricted**: Creating or deleting a keyframe on one individual curve is restricted by the Tanim editor to prevent invalid quaternions.

### Curve Editing Restrictions

To maintain quaternion validity:

**No Individual Tangent Editing**: You cannot adjust tangents on individual component curves.

**Global Handle Types**: You can change the handle type for all curves together by setting "All Curves' Handles' Type" to:

- **FLAT**: Smooth interpolation with automatic tangents
- **LINEAR**: Straight line interpolation between keyframes
- **CONSTANT**: No interpolation, immediate value changes

### The Spins Curve

The fifth curve, "spins", controls how many full 360-degree rotations occur when interpolating between keyframes.

**Why Spins Are Needed**: A quaternion at 0 degrees rotation and 360 degrees rotation are identical. When you slerp between two identical quaternions, no rotation occurs. The spins curve solves this by specifying how many full rotations should happen during interpolation.

**How to Use Spins**:

1. At keyframe `a`, set your object to the starting rotation (e.g., 0 degrees)
2. At keyframe `b`, set your object to nearly the same rotation but very slightly rotated in the direction you want to spin
3. Set the spins value at keyframe `b` to the number of full rotations you want (e.g., 1 for one full spin, 2 for two spins)

**Important**: The rotation at `b` must be slightly different from `a`. If the quaternions are exactly identical, slerp cannot determine which direction to rotate, even with a non-zero spins value.

**Direction Control**:

- Positive spins values rotate in one direction
- Negative spins values rotate in the opposite direction

**Example**: To make an object spin twice while moving from point A to point B:

- Keyframe at time 0: rotation = 0 degrees, spins = 0
- Keyframe at time 1: rotation = 1 degree (slightly rotated), spins = 2

The object will complete two full rotations between these keyframes.

---

## Type Metadata System

Tanim uses two internal metadata systems to handle type-specific behaviors:

### RepresentationMeta

Controls how certain types are displayed in the inspector:

- **VECTOR**: Display as separate numeric inputs (x, y, z, w)
- **COLOR**: Display as a color picker
- **QUAT**: Special quaternion handling

Applicable to: `glm::vec3`, `glm::vec4`, `glm::quat`

You can change the representation in the editor when inspecting a sequence. This is purely a display preference and doesn't affect the animation data.

### TypeMeta

Controls type-specific restrictions and behaviors:

- **INT**: Values are floored to integers during sampling
- **BOOL**: Values restricted to 0 or 1, constant interpolation only

These are automatically detected by Tanim based on the field type and cannot be changed.

---

## Unsupported Types

The following types are **not supported** for animation:

**Strings**: Cannot meaningfully interpolate between text values.

**Custom Structs**: Planned for future versions.

**Pointers/References**: No storage location to animate.

**Static Fields**: Animation is per-entity, static fields are shared across all instances.

**Enums**: No meaningful interpolation between enum values (use `int` if needed).

**Matrices**: Use separate position, rotation, scale components instead.

If you try to reflect an unsupported type, you'll get a compile error.

---

## Best Practices

**Use Appropriate Types**: Use `float` for smooth continuous values and `int` for discrete counts or indices.

**Quaternions for Rotation**: Always use `glm::quat` for 3D rotations, not Euler angles. Quaternions avoid gimbal lock and interpolate smoothly with slerp.

**Color Choice**: When animating colors, decide if you want precise component control (VECTOR representation) or color picker convenience (COLOR representation).

**Bool for States**: Use `bool` for binary states that should change instantly, not for values that should interpolate smoothly.

**Vector Components**: Remember that vector components animate independently. Each axis follows its own curve.

**Spins for Full Rotations**: When you need an object to rotate multiple times (like a spinning coin or rotating platform), use the spins curve with quaternions.

---

## Future Support

Planned for future versions:

- Custom type support through user-defined interpolation functions

See the project roadmap for details.

---

## Next Steps

- [Reflection System](api-reference/reflection.md) - Learn how to reflect these types
- [Core Concepts](core-concepts.md) - Understand how types map to curves
- [Example Implementation](example-implementation.md) - See types in use
