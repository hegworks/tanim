# Lifecycle Functions

Complete reference for all Tanim lifecycle and control functions.

## Initialization

### tanim::Init

```cpp
void Init();
```

**Purpose**: Initializes the Tanim system. Must be called once at application startup before any other Tanim functions.

**When to call**: After ImGui initialization, before any other Tanim API calls.

**Example**:

```cpp
void OnApplicationStart()
{
    ImGui::CreateContext();
    // ... ImGui setup

    tanim::Init();  // Initialize Tanim after ImGui
}
```

**Notes**:

- Only call this once during your application's lifetime

---

## Editor Integration

### tanim::Draw

```cpp
void Draw();
```

**Purpose**: Renders the Tanim editor window and all its UI elements.

**When to call**: Every frame between `ImGui::NewFrame()` and `ImGui::Render()`.

**Example**:

```cpp
void OnEditorFrame()
{
    ImGui::NewFrame();

    // Your other ImGui panels
    RenderInspector();
    RenderHierarchy();

    tanim::Draw();  // Render Tanim editor

    ImGui::Render();
}
```

**Notes**:

- The Tanim window will only appear if a timeline has been opened with `OpenForEditing()`
- Safe to call even when no timeline is open (does nothing in that case)

### tanim::UpdateEditor

```cpp
void UpdateEditor(float dt);
```

**Purpose**: Updates time-based editor operations and handles per-frame editor logic.

**Parameters**:

- `dt`: Delta time in seconds since the last frame

**When to call**: Every frame in your editor loop, before `Draw()`.

**Example**:

```cpp
void OnEditorFrame(float delta_time)
{
    ImGui::NewFrame();

    tanim::UpdateEditor(delta_time);  // Update first
    tanim::Draw();                    // Then draw

    ImGui::Render();
}
```

**Notes**:

- Must be called before `Draw()` in the same frame
- Handles editor playback scrubbing and other time-based features

---

## Timeline Editing

### tanim::OpenForEditing

```cpp
void OpenForEditing(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata);
```

**Purpose**: Opens the Tanim editor window to edit a specific timeline.

**Parameters**:

- `registry`: Your ENTT registry containing the entities being animated
- `entity_datas`: List of all entities that can be animated in this timeline (usually the root entity and all children)
- `tdata`: Reference to the TimelineData to edit
- `cdata`: Reference to the ComponentData associated with the entity

**When to call**: When the user wants to edit a timeline, typically from an editor button or menu.

**Example**:

```cpp
void OnEditButtonClicked(entt::entity entity)
{
    auto& anim_comp = registry.get<AnimationComponent>(entity);

    // Build entity list
    std::vector<tanim::EntityData> entity_datas;
    entity_datas.push_back({
        .m_uid = GetEntityUID(entity),
        .m_display = GetEntityName(entity)
    });

    // Add children recursively
    AddChildrenToList(entity, entity_datas);

    // Open editor
    tanim::OpenForEditing(registry,
                         entity_datas,
                         anim_comp.timeline_data,
                         anim_comp.component_data);
}
```

**Notes**:

- Only one timeline can be open for editing at a time
- Opening a new timeline automatically closes the previously opened one
- The `entity_datas` vector determines which entities appear in the sequence creation menu
- Store any necessary data in `cdata.m_user_data` to help your `FindEntityOfUID` implementation

### tanim::CloseEditor

```cpp
void CloseEditor();
```

**Purpose**: Closes the currently open Tanim editor window.

**When to call**:

- When switching scenes or unloading the timeline data
- When removing the animation component from an entity
- When the timeline data is about to be destroyed

**Example**:

```cpp
void OnSceneUnload()
{
    tanim::CloseEditor();  // Close before unloading scene
    UnloadCurrentScene();
}

void OnRemoveAnimationComponent(entt::entity entity)
{
    tanim::CloseEditor();  // Close before removing component
    registry.remove<AnimationComponent>(entity);
}
```

**Notes**:

- Safe to call even if no timeline is currently open
- Must be called before the TimelineData or ComponentData is destroyed, otherwise might cause a crash due to null pointer access

---

## Play Mode Control

### tanim::EnterPlayMode

```cpp
void EnterPlayMode();
```

**Purpose**: Signals to Tanim that the application is entering play mode.

**When to call**: Once when your application/game transitions from editor mode to play mode.

**Example**:

```cpp
void OnPlayButtonPressed()
{
    tanim::EnterPlayMode();

    // Start all timelines
    for (auto entity : animated_entities)
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::StartTimeline(anim.timeline_data, anim.component_data);
    }
}
```

**Notes**:

- Must be called before `StartTimeline()` for any entity
- In a release build (non-editor), call this once at startup

### tanim::ExitPlayMode

```cpp
void ExitPlayMode();
```

**Purpose**: Signals to Tanim that the application is exiting play mode.

**When to call**: Once when your application/game transitions from play mode back to editor mode.

**Example**:

```cpp
void OnStopButtonPressed()
{
    // Stop all timelines
    for (auto entity : animated_entities)
    {
        auto& anim = registry.get<AnimationComponent>(entity);
        tanim::StopTimeline(anim.component_data);
    }

    tanim::ExitPlayMode();
}
```

**Notes**:

- Must be called after `StopTimeline()` for all entities
- In a release build, typically called at shutdown

---

## Timeline Playback

### tanim::StartTimeline

```cpp
void StartTimeline(const TimelineData& tdata, ComponentData& cdata);
```

**Purpose**: Prepares a timeline for playback. Must be called after `EnterPlayMode()` and before `UpdateTimeline()`.

**Parameters**:

- `tdata`: The TimelineData to start
- `cdata`: The ComponentData for this entity's playback

**When to call**: Once for each entity after calling `EnterPlayMode()`.

**Example**:

```cpp
void OnEnterPlayMode()
{
    tanim::EnterPlayMode();

    // Start each timeline
    auto view = registry.view<AnimationComponent>();
    for (auto entity : view)
    {
        auto& anim = view.get<AnimationComponent>(entity);
        tanim::StartTimeline(anim.timeline_data, anim.component_data);
    }
}
```

**Notes**:

- Does not start playback automatically
- If `m_play_immediately` is enabled in TimelineData, the timeline will start playing
- Otherwise, call `Play()` to begin playback

### tanim::UpdateTimeline

```cpp
void UpdateTimeline(entt::registry& registry,
                    const std::vector<EntityData>& entity_datas,
                    TimelineData& tdata,
                    ComponentData& cdata,
                    float delta_time);
```

**Purpose**: Advances timeline playback and samples animation curves, writing values to component fields.

**Parameters**:

- `registry`: Your ENTT registry containing the animated entities
- `entity_datas`: List of all entities that can be animated (same as used in `OpenForEditing`)
- `tdata`: The TimelineData being played
- `cdata`: The ComponentData for this entity's playback
- `delta_time`: Time delta in seconds since the last update

**When to call**: Every frame during play mode for each entity with an active timeline.

**Example**:

```cpp
void OnUpdate(float delta_time)
{
    auto view = registry.view<AnimationComponent>();
    for (auto entity : view)
    {
        auto& anim = view.get<AnimationComponent>(entity);

        // Get entity list (consider caching this)
        std::vector<tanim::EntityData> entity_datas = BuildEntityList(entity);

        tanim::UpdateTimeline(registry,
                            entity_datas,
                            anim.timeline_data,
                            anim.component_data,
                            delta_time);
    }
}
```

**Notes**:

- This function does the actual animation work: sampling curves and writing to components
- Only updates if the timeline is playing (check with `IsPlaying()`)
- The `entity_datas` should match what was used in `OpenForEditing()`
- Consider caching the `entity_datas` vector for performance

### tanim::StopTimeline

```cpp
void StopTimeline(ComponentData& cdata);
```

**Purpose**: Stops timeline playback and prepares it for shutdown. Must be called before `ExitPlayMode()`.

**Parameters**:

- `cdata`: The ComponentData for the timeline to stop

**When to call**: Once for each entity before calling `ExitPlayMode()`.

**Example**:

```cpp
void OnExitPlayMode()
{
    // Stop all timelines first
    auto view = registry.view<AnimationComponent>();
    for (auto entity : view)
    {
        auto& anim = view.get<AnimationComponent>(entity);
        tanim::StopTimeline(anim.component_data);
    }

    tanim::ExitPlayMode();
}
```

**Notes**:

- Resets the playback time to the beginning

---

## Playback Control

### tanim::IsPlaying

```cpp
bool IsPlaying(const ComponentData& cdata);
```

**Purpose**: Checks if a timeline is currently playing.

**Parameters**:

- `cdata`: The ComponentData to check

**Returns**: `true` if the timeline is playing, `false` if paused or stopped.

**Example**:

```cpp
void UpdateUI()
{
    auto& anim = registry.get<AnimationComponent>(selected_entity);

    if (tanim::IsPlaying(anim.component_data))
    {
        ShowPauseButton();
    }
    else
    {
        ShowPlayButton();
    }
}
```

### tanim::Play

```cpp
void Play(ComponentData& cdata);
```

**Purpose**: Starts or resumes timeline playback.

**Parameters**:

- `cdata`: The ComponentData to control

**When to call**: Anytime during play mode to begin or resume playback.

**Example**:

```cpp
void OnPlayButtonClicked()
{
    auto& anim = registry.get<AnimationComponent>(selected_entity);
    tanim::Play(anim.component_data);
}
```

**Notes**:

- If paused, continues from the current time
- If stopped, starts from the beginning (or wherever the timeline was stopped)
- Can be called on any entity at any time to control its playback state independently, or called on multiple entities to synchronize their playback

### tanim::Pause

```cpp
void Pause(ComponentData& cdata);
```

**Purpose**: Pauses timeline playback at the current time.

**Parameters**:

- `cdata`: The ComponentData to control

**Example**:

```cpp
void OnPauseButtonClicked()
{
    auto& anim = registry.get<AnimationComponent>(selected_entity);
    tanim::Pause(anim.component_data);
}
```

**Notes**:

- Calling `Play()` after `Pause()` will resume from where it was paused
- The current playback time is preserved

### tanim::Stop

```cpp
void Stop(ComponentData& cdata);
```

**Purpose**: Stops timeline playback and resets to the beginning.

**Parameters**:

- `cdata`: The ComponentData to control

**Example**:

```cpp
void OnStopButtonClicked()
{
    auto& anim = registry.get<AnimationComponent>(selected_entity);
    tanim::Stop(anim.component_data);
}
```

**Notes**:

- Resets playback time to the beginning
- Calling `Play()` after `Stop()` will start from the beginning

---

## Serialization

### tanim::Serialize

```cpp
std::string Serialize(TimelineData& tdata);
```

**Purpose**: Converts TimelineData to a string representation for saving.

**Parameters**:

- `tdata`: The TimelineData to serialize

**Returns**: A string containing all timeline data.

**When to call**: When you want to save timeline data to disk, network, or memory.

**Example**:

```cpp
void SaveTimeline(const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    // Serialize to string
    std::string serialized = tanim::Serialize(anim.timeline_data);

    // Save to file (using your file system)
    WriteFile(filepath, serialized);
}
```

**Notes**:

- The returned string format is internal to Tanim (currently JSON-based)
- Only serialize when actually saving, not every frame (this is an expensive operation)
- The string contains all sequences, curves, keyframes, and timeline settings

### tanim::Deserialize

```cpp
void Deserialize(TimelineData& tdata, const std::string& serialized_string);
```

**Purpose**: Restores TimelineData from a serialized string.

**Parameters**:

- `tdata`: The TimelineData to populate
- `serialized_string`: The string previously returned by `Serialize()`

**When to call**: When loading timeline data from disk, network, or memory.

**Example**:

```cpp
void LoadTimeline(const std::string& filepath)
{
    auto& anim = registry.get<AnimationComponent>(entity);

    // Load from file (using your file system)
    std::string serialized = ReadFile(filepath);

    // Deserialize into timeline
    tanim::Deserialize(anim.timeline_data, serialized);
}
```

**Notes**:

- Will log an error if the serialization version is not supported (version handling is internal to Tanim)
- Any existing data in `tdata` will be replaced
- The string must have been created by `Serialize()`
- This is an expensive operation, only call when actually loading

---

## Component Registration

### tanim::RegisterComponent

```cpp
template <typename T>
void RegisterComponent();
```

**Purpose**: Manually registers a component with Tanim's type system.

**Template Parameters**:

- `T`: The component type to register

**When to call**: Only needed if you used `TANIM_REFLECT_NO_REGISTER` instead of `TANIM_REFLECT`. Call once during application initialization.

**Example**:

```cpp
// In header (global namespace)
TANIM_REFLECT_NO_REGISTER(MyComponent, position, rotation);

// In initialization code
void OnApplicationStart()
{
    ImGui::CreateContext();
    tanim::Init();

    // Manual registration needed
    tanim::RegisterComponent<MyComponent>();
}
```

**Notes**:

- Not needed if you use `TANIM_REFLECT` (which auto-registers)
- Only use this for a specific component if `TANIM_REFLECT` causes initialization order issues for that component
- See [Reflection System](reflection.md) for more details

---

## Function Call Sequence Summary

### Startup

```cpp
ImGui::CreateContext();
tanim::Init();
// Component reflection happens automatically with TANIM_REFLECT
```

### Editor Frame

```cpp
ImGui::NewFrame();
tanim::UpdateEditor(delta_time);
tanim::Draw();
ImGui::Render();
```

### Timeline Editing

```cpp
// Open
tanim::OpenForEditing(registry, entity_datas, tdata, cdata);

// ... user edits in the UI ...

// Save
std::string data = tanim::Serialize(tdata);
SaveToFile(data);

// Close
tanim::CloseEditor();
```

### Play Mode Lifecycle

```cpp
// Enter play mode
tanim::EnterPlayMode();
for (each entity) {
    tanim::StartTimeline(tdata, cdata);
}

// Each frame
for (each entity) {
    tanim::UpdateTimeline(registry, entity_datas, tdata, cdata, dt);
}

// Exit play mode
for (each entity) {
    tanim::StopTimeline(cdata);
}
tanim::ExitPlayMode();
```

### Manual Playback Control

```cpp
tanim::Play(cdata);    // Start/resume
tanim::Pause(cdata);   // Pause
tanim::Stop(cdata);    // Stop and reset
bool playing = tanim::IsPlaying(cdata);  // Check state
```
