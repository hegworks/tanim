# Core Concepts

This document explains Tanim's architecture, data structures, and how animation data flows through the system.

## Architecture Overview

Tanim follows a hierarchical data structure for organizing animations:

```
Component (on entity)
    └─ Timeline
        └─ Sequence (per animated field)
            └─ Curve (per field component, e.g., x, y, z)
                └─ Keyframe
                    ├─ In Handle
                    └─ Out Handle
```

### Component
A component attached to an entity that holds references to a `TimelineData` and its runtime `ComponentData`. In your engine, this might be called `AnimationComponent`, `Tanimatable`, or any name you choose.

### Timeline
Contains all the sequences that animate different component fields on entities. A timeline can be shared across multiple entities, allowing the same animation to be reused. For example, multiple enemies can reference the same "patrol" animation timeline.

### Sequence
Represents a single animated property on a component. For example, if you want to animate a `Transform` component's `position` and `rotation`, you would have two sequences: one for `position` and one for `rotation`.

Each sequence targets a specific component on a specific entity (identified by UID) and a specific field within that component.

### Curve
A sequence contains one or more curves depending on the field type. For example:
- `float` field has 1 curve
- `glm::vec3` field has 3 curves (x, y, z)

Each curve contains keyframes that define the animation over time. Some types like `glm::quat` have special handling that is explained in the [Supported Types](supported-types.md) documentation.

### Keyframe
A point in time that defines a value on a curve. Keyframes are connected by curve segments that interpolate between them.

### Handle
Each keyframe has two handles (in and out) that control the shape of the Bezier curve segment. Handles define the tangent at the keyframe, affecting how smoothly the animation transitions.

## Data Structures

Tanim uses three main data structures to bridge your engine and the animation system:

### EntityData

```cpp
struct EntityData
{
    std::string m_uid;      // Unique identifier for the entity
    std::string m_display;  // Display name in the editor
};
```

**Purpose**: Identifies entities that can be animated in a timeline.

**Usage**: When opening a timeline for editing or updating it at runtime, you provide a list of `EntityData` for all entities that should be animatable. This typically includes the entity with the animation component and all its children.

**UID vs Display**:
- `m_uid`: A string that your `FindEntityOfUID` function can use to retrieve the actual `entt::entity`. This could be an entity name, a stringified UUID, or any identifier you choose. Must be unique within the animated hierarchy.
- `m_display`: A user-friendly name shown in the editor. Can be the same as `m_uid` or something more descriptive.

**Example**:
```cpp
std::vector<tanim::EntityData> entity_datas;
entity_datas.push_back({
    .m_uid = "player_root",
    .m_display = "Player"
});
entity_datas.push_back({
    .m_uid = "123456789",
    .m_display = "Player Head"
});
```

### TimelineData

```cpp
struct TimelineData
{
    // All fields handled internally by Tanim
};
```

**Purpose**: Contains all the animation data for a timeline. This includes the curves, keyframes, timeline settings, and editor state.

**Lifetime**: Should persist as long as you want to keep the animation data. Can be serialized to disk and loaded back. Can be shared across multiple entities by having their components reference the same `TimelineData`.

**Serialization**: Tanim handles serialization and deserialization internally. Use `tanim::Serialize()` to get a string representation of the timeline data, and `tanim::Deserialize()` to restore it. See [Lifecycle Functions](api-reference/lifecycle.md) for details on these functions.

**Important**: You should not directly edit `TimelineData` fields. Tanim manages this data internally. Your responsibility is to keep it in memory and pass it to Tanim API functions.

### ComponentData

```cpp
struct ComponentData
{
    std::any m_user_data;
    
private:
    // Runtime playback state handled internally by Tanim
};
```

**Purpose**: Holds runtime playback state for a specific entity's timeline. Each entity that plays a timeline needs its own `ComponentData`.

**Lifetime**: Exists for as long as the entity exists. Does not need to be serialized.

**m_user_data**: This is a field you can use to store any custom data that helps your `FindEntityOfUID` implementation. Common uses:
- Store the root entity
- Cache a list of all animatable entities
- Store a reference to your scene or entity manager

**Example**:
```cpp
struct MyUserData
{
    entt::entity root_entity;
    std::vector<entt::entity> cached_children;
};

// When opening for editing or starting playback:
anim_comp.component_data.m_user_data = MyUserData{
    .root_entity = entity,
    .cached_children = GetAllChildren(entity)
};
```

Then in your `FindEntityOfUID` implementation, you can access this data to quickly find the entity.

## Animation Data Flow

### Editor Mode

1. User opens a timeline for editing via `tanim::OpenForEditing()`
2. User creates sequences, adds keyframes, and edits curves in the Tanim editor
3. Changes are automatically stored in the `TimelineData`
4. User can serialize the timeline via `tanim::Serialize()` to save to disk
5. User can deserialize later via `tanim::Deserialize()` to load from disk

### Play Mode

1. Application calls `tanim::EnterPlayMode()` to signal play mode start
2. For each animated entity, call `tanim::StartTimeline()` to begin playback
3. Each frame, call `tanim::UpdateTimeline()` which:
   - Advances the playback time based on delta time
   - Samples the curves at the current time
   - Writes the sampled values directly to the component fields on your entities
4. When stopping, call `tanim::StopTimeline()` and `tanim::ExitPlayMode()`

### How Sampling Works

When `UpdateTimeline` runs, Tanim:
1. Calculates which frame to sample based on the current playback time
2. For each sequence in the timeline:
   - Finds the entity using your `FindEntityOfUID` implementation
   - Gets the component from the entity
   - Samples the curve at the current time
   - Writes the value to the component field
3. ENTT handles the actual component data, Tanim just reads and writes to it

## Curve Types and Interpolation

Tanim uses cubic Bezier curves for interpolation between keyframes. This is the same system used by Unity, Unreal, and other professional animation tools.

### Why Bezier Curves?

Bezier curves allow precise control over animation timing and easing. Unlike simple linear interpolation, Bezier curves can create smooth acceleration and deceleration, making animations feel more natural.

### Tangent Modes

Each keyframe can have different tangent modes that control the curve shape:

**Auto**: Tanim automatically calculates smooth tangents using a monotonic Catmull-Rom algorithm. This creates smooth curves that pass through all keyframes without overshooting.

**Smooth**: Both handles move together symmetrically, ensuring smooth continuity at the keyframe. Used for smooth, flowing animations.

**Broken**: Handles can be adjusted independently, allowing sharp changes in curve direction. Used for impacts, sudden stops, or intentionally non-smooth motion.

**Weighted**: Handles have adjustable length (weight) that affects the influence region of the keyframe. Longer handles create gentler curves, shorter handles create tighter curves.

### Curve Constraints

Tanim enforces monotonicity in time - the curve always moves forward in time and never loops back. This prevents invalid animations where a single time value would map to multiple values.

Handles are automatically clamped to neighboring keyframes to maintain this constraint.

## Quaternion Animation

Quaternions require special handling because their components (w, x, y, z) are interdependent. You cannot interpolate each component independently.

When animating `glm::quat` fields:
- Tanim shows 5 curves in the editor: w, x, y, z, and spins
- Use the **+keyframe** button or **record** button to create keyframes on all curves simultaneously at the same time
- Use the **-keyframe** button to remove keyframes from all curves at once
- Individual curve editing is restricted to maintain quaternion validity
- At runtime, Tanim uses spherical linear interpolation (slerp) for proper quaternion blending

For more details on quaternion animation and the spins curve, see [Supported Types](supported-types.md).

## Component Reflection

Tanim uses the `visit_struct` library to reflect component fields at compile-time. The `TANIM_REFLECT` macro makes a component's fields accessible to Tanim.

```cpp
TANIM_REFLECT(ComponentName, field1, field2, field3);
```

This macro:
1. Registers the component with visit_struct for field iteration
2. Automatically registers the component with Tanim's type system
3. Makes the fields available in the Tanim editor for creating sequences

Only fields with supported types can be included in the reflection. See [Supported Types](supported-types.md) for details.

## Timeline Sharing and Reusability

One `TimelineData` can be referenced by multiple entities. This is useful for:

**Reusable animations**: Create one "walk cycle" timeline and use it on multiple characters.

**Memory efficiency**: Store one copy of animation data instead of duplicating it per entity.

**Synchronized vs Independent Playback**: Each entity has its own `ComponentData` which contains runtime playback state. This means:
- **Synchronized**: Call `tanim::Play()`, `tanim::Pause()`, and `tanim::Stop()` on all entities sharing a timeline to control them together
- **Independent**: Call these functions on individual entities to control them separately. One entity can be playing while another is paused, or they can be at different times in the same animation

You can control playback state through code using:
```cpp
tanim::Play(component_data);   // Start/resume playback
tanim::Pause(component_data);  // Pause at current time
tanim::Stop(component_data);   // Stop and reset to beginning
tanim::IsPlaying(component_data);  // Check current state
```

See [Lifecycle Functions](api-reference/lifecycle.md) for complete API documentation.

## Next Steps

Now that you understand the architecture, explore:
- [API Reference](api-reference/overview.md) for detailed function documentation
- [Example Implementation](example-implementation.md) for complete integration code
- [UI & Shortcuts](ui-shortcuts.md) to learn the editor interface
