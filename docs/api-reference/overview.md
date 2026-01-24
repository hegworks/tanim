# API Reference Overview

This section provides complete documentation for all Tanim API functions, data structures, and macros.

## API Organization

Tanim's API is organized into several categories:

### [Lifecycle Functions](lifecycle.md)
Core functions that manage Tanim's state and animation playback:
- Initialization and cleanup
- Editor integration
- Timeline editing
- Runtime playback control
- Serialization

### [Data Structures](data-structures.md)
The three main data types that bridge your engine and Tanim:
- `EntityData` - Identifies animatable entities
- `TimelineData` - Stores animation data
- `ComponentData` - Holds runtime playback state

### [User Overrides](user-overrides.md)
Three required functions you must implement in your project:
- `FindEntityOfUID` - Entity lookup
- `LogError` - Error reporting
- `LogInfo` - Info reporting

### [Reflection System](reflection.md)
Macros and functions for making components animatable:
- `TANIM_REFLECT` - Register components with auto-registration
- `TANIM_REFLECT_NO_REGISTER` - Register without auto-registration
- `RegisterComponent` - Manual component registration

## Typical Workflows

### Initial Setup

```cpp
// 1. Initialize Tanim (once at startup)
tanim::Init();

// 2. Reflect your components (in global namespace, typically in header files)
TANIM_REFLECT(Transform, position, rotation, scale);
TANIM_REFLECT(Light, color, intensity);

// 3. Implement user overrides (in a .cpp file)
entt::entity tanim::FindEntityOfUID(const ComponentData& cdata, const std::string& uid) { /*...*/ }
void tanim::LogError(const std::string& message) { /*...*/ }
void tanim::LogInfo(const std::string& message) { /*...*/ }
```

### Editor Integration

```cpp
// Each frame in your editor loop
void OnEditorFrame(float delta_time)
{
    ImGui::NewFrame();
    
    // Update and draw Tanim editor
    tanim::UpdateEditor(delta_time);
    tanim::Draw();
    
    ImGui::Render();
}
```

### Opening a Timeline for Editing

```cpp
// Prepare entity data
std::vector<tanim::EntityData> entity_datas = BuildEntityList(root_entity);

// Open the editor
tanim::OpenForEditing(registry, entity_datas, timeline_data, component_data);

// Later, when done editing or changing scenes
tanim::CloseEditor();
```

### Saving and Loading Timelines

```cpp
// Serialize timeline to string
std::string serialized = tanim::Serialize(timeline_data);
SaveToFile("animation.tanim", serialized);

// Deserialize timeline from string
std::string loaded = LoadFromFile("animation.tanim");
tanim::Deserialize(timeline_data, loaded);
```

### Runtime Playback

```cpp
// Enter play mode
void OnEnterPlayMode()
{
    tanim::EnterPlayMode();
    
    // For each entity with animations
    for (auto entity : animated_entities)
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::StartTimeline(anim.timeline_data, anim.component_data);
    }
}

// Update each frame
void OnUpdate(float delta_time)
{
    for (auto entity : animated_entities)
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        std::vector<tanim::EntityData> entity_datas = BuildEntityList(entity);
        
        tanim::UpdateTimeline(registry, entity_datas, 
                            anim.timeline_data, anim.component_data, delta_time);
    }
}

// Exit play mode
void OnExitPlayMode()
{
    // For each entity with animations
    for (auto entity : animated_entities)
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::StopTimeline(anim.component_data);
    }
    
    tanim::ExitPlayMode();
}
```

### Manual Playback Control

```cpp
// Check playback state
if (tanim::IsPlaying(component_data))
{
    // Animation is currently playing
}

// Control playback
tanim::Play(component_data);   // Start or resume
tanim::Pause(component_data);  // Pause at current time
tanim::Stop(component_data);   // Stop and reset to beginning
```

## Function Call Order

Tanim functions must be called in specific orders for correct behavior:

### Startup
```
1. ImGui::CreateContext()
2. tanim::Init()
3. Component reflection (TANIM_REFLECT macros execute automatically)
```

### Editor Frame
```
1. ImGui::NewFrame()
2. tanim::UpdateEditor(dt)
3. tanim::Draw()
4. ImGui::Render()
```

### Entering Play Mode
```
1. tanim::EnterPlayMode()
2. tanim::StartTimeline() for each animated entity
```

### Play Mode Frame
```
1. tanim::UpdateTimeline() for each animated entity
```

### Exiting Play Mode
```
1. tanim::StopTimeline() for each animated entity
2. tanim::ExitPlayMode()
```

## Threading Considerations

Tanim is **not thread-safe**. All Tanim API functions must be called from the same thread, typically your main thread. This includes:
- All lifecycle functions
- User override implementations
- Component registration

## Error Handling

Tanim reports errors through your `LogError` implementation. Common error scenarios:

**Entity not found**: When `FindEntityOfUID` returns `entt::null`, Tanim logs:
```
FindEntityOfUID with the uid of [uid] returned entt::null
```
Animation for that sequence will be skipped.

**Component missing on entity**: When an entity doesn't have the component being animated, Tanim logs:
```
entity [entity_id] does not have a component named [ComponentName]
```
Occurs during sampling, inspection, or recording keyframes.

**Sequence target not found**: When the entity or component specified in a sequence cannot be found, Tanim logs:
```
Couldn't find any entity with matching details: [sequence_full_name]
```

**Unsupported serialization version**: When deserializing timeline data from an older format, Tanim logs:
```
Versions prior to 2 are not supported. Can not deserialize.
```
Deserialization will fail and timeline data won't be loaded.

## Performance Considerations

**EntityData caching**: Build your `entity_datas` vector once and reuse it across multiple `UpdateTimeline` calls when possible.

**Component lookup**: Your `FindEntityOfUID` implementation is called frequently during playback. Consider caching entity lookups in `ComponentData::m_user_data`.

**Timeline sharing**: Share `TimelineData` across multiple entities when they use the same animation to reduce memory usage.

**Serialization**: `Serialize` and `Deserialize` are relatively expensive operations. Only call them when actually saving or loading, not every frame.

## Next Steps

Explore the detailed API documentation:
- [Lifecycle Functions](lifecycle.md) - Complete function reference
- [Data Structures](data-structures.md) - Data type details
- [User Overrides](user-overrides.md) - Implementation requirements
- [Reflection System](reflection.md) - Component registration
