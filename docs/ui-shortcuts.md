# UI & Shortcuts

Guide to using the Tanim timeline editor interface, keyboard shortcuts, and mouse interactions.

> [!CAUTION]
> This file is a WIP & and the information it provides is not correct. This will be resolved soon.

## Overview

The Tanim editor provides a timeline-based interface for creating and editing animations. This guide covers the UI layout, controls, and shortcuts for efficient workflow.

TODOVISUAL Add screenshot showing the full Tanim editor window with labeled sections

---

## Editor Layout

The Tanim editor window consists of several main sections:

**Timeline Header**: Shows timeline name, frame range, and playback controls.

**Sequence List**: Left panel listing all animated sequences (properties). Each sequence can be expanded to show its curves.

**Curve Editor**: Main area displaying animation curves where you can edit keyframes and handles.

**Inspector**: Shows detailed values and settings for selected keyframes or sequences.

**Playhead**: Red vertical line indicating the current time position.

---

## Timeline Controls

### Playback Controls

TODOVISUAL Add screenshot of playback controls

**Play/Pause**: Start or pause timeline playback in the editor.

**Stop**: Stop playback and reset to the beginning or last stopped position.

**Frame Navigation**:

- Click on the timeline ruler to jump to a specific frame
- Drag the playhead to scrub through the animation

### Timeline Settings

**Frame Range**: Set the first and last frames of the timeline's active range.

**Samples Per Second**: Controls playback speed (typically 60).

**Playback Type**:

- **HOLD**: Stops at the last frame and holds the final values
- **RESET**: Stops and resets to the beginning
- **LOOP**: Continuously loops from start to end

**Play Immediately**: When enabled, the timeline starts playing automatically when entering play mode.

---

## Sequence Management

### Creating Sequences

1. Click the **+** button in the sequence list
2. Select the entity you want to animate
3. Select the component on that entity
4. Select the field within the component
5. The sequence is created with curves for each component of the field type

TODOVISUAL Add screenshot of sequence creation menu

### Sequence Options

**Right-click** on a sequence to access:

- **Remove Sequence**: Delete the sequence and all its curves
- **Change Representation**: For vec3/vec4, switch between VECTOR and COLOR representation

**Expand/Collapse**: Click the arrow icon to show or hide the sequence's curves.

**Select**: Click on a sequence to select it and show its curves in the curve editor.

---

## Curve Editor

### Viewing Curves

**Zoom**:

- Scroll wheel to zoom in/out
- Hold Ctrl and drag to zoom to a specific region

**Pan**:

- Middle mouse button drag to pan the view
- Arrow keys to pan in small increments

**Frame All**: Press **F** to frame all keyframes in the view.

**Frame Selection**: Press **Shift+F** to frame only selected keyframes.

TODOVISUAL Add screenshot of curve editor with multiple curves

### Curve Display

**Curve Colors**: Each curve has a distinct color for easy identification.

**Keyframe Markers**:

- **Circle**: Regular keyframe
- **Diamond**: Keyframe at current playhead position

**Curve Legend**: Shows which curve corresponds to which component (x, y, z, w, etc.).

---

## Keyframe Operations

### Creating Keyframes

**Double-click**: Double-click on a curve to create a keyframe at that position.

**+keyframe Button**: Creates keyframes at the current playhead position on the selected sequence's curves.

- For regular types: Creates keyframes on all component curves
- For quaternions: Creates synchronized keyframes on all 5 curves (w, x, y, z, spins)

**Record Button**:

1. Position the playhead where you want the keyframe
2. Click the record button
3. Modify the component values in your engine's inspector or through other means
4. The keyframe is created with the current values

TODOVISUAL Add screenshot showing keyframe creation methods

### Selecting Keyframes

**Click**: Click on a keyframe to select it.

**Ctrl+Click**: Add or remove keyframes from the selection.

**Shift+Click**: Select a range of keyframes between the last selected and clicked keyframe.

**Box Selection**: Click and drag in empty space to draw a selection box around keyframes.

**Select All**: Press **Ctrl+A** to select all keyframes on visible curves.

### Moving Keyframes

**Drag**: Click and drag selected keyframes to move them in time (X axis) and value (Y axis).

**Constrain to X**: Hold **Shift** while dragging to constrain movement to time only.

**Constrain to Y**: Hold **Alt** while dragging to constrain movement to value only.

**Nudge**: Use arrow keys to nudge selected keyframes by small amounts.

### Deleting Keyframes

**Delete Key**: Press **Del** to delete selected keyframes.

**-keyframe Button**: Removes keyframes at the current playhead position.

- For regular types: Removes keyframes on all component curves
- For quaternions: Removes synchronized keyframes from all 5 curves

**Right-click Menu**: Right-click on selected keyframes and choose "Delete".

---

## Handle Manipulation

Handles control the tangent (slope) of the curve at a keyframe, affecting the interpolation shape.

### Selecting Handles

**Click**: Click on a handle (small circle at the end of tangent lines) to select it.

**Both Handles**: Selecting a keyframe also selects its handles for easy tangent adjustment.

### Moving Handles

**Drag**: Click and drag a handle to adjust the tangent angle and weight.

**Symmetrical**: In SMOOTH mode, moving one handle moves the opposite handle symmetrically.

**Independent**: In BROKEN mode, handles can be moved independently.

TODOVISUAL Add screenshot showing handle manipulation

### Handle Constraints

**Monotonicity**: Handles are automatically clamped to neighboring keyframes to ensure the curve remains monotonic in time (doesn't loop back).

**Visual Indicators**: Red lines or markers indicate when a handle is constrained by neighboring keyframes.

---

## Tangent Modes

Each keyframe can have different tangent modes that control its curve shape:

**AUTO**:

- Tanim automatically calculates smooth tangents
- Uses Fritsch-Carlson monotonic Catmull-Rom algorithm
- Good for smooth, natural motion

**SMOOTH**:

- Both handles move together symmetrically
- Ensures smooth C1 continuity at the keyframe
- Handles can have adjustable weight

**BROKEN**:

- Handles can be moved independently
- Allows sharp changes in curve direction
- Useful for impacts or sudden stops

**WEIGHTED**:

- Handle length affects the influence region
- Longer handles create gentler curves
- Shorter handles create tighter curves

**FLAT**:

- Horizontal tangents at the keyframe
- Creates ease-in/ease-out effect

**LINEAR**:

- Straight line interpolation between keyframes
- No curve smoothing

**CONSTANT**:

- No interpolation, immediate value change
- Required for bool types

### Changing Tangent Modes

**Per Keyframe**: Right-click on a keyframe and select the desired tangent mode.

**All Curves (Quaternions)**: For quaternion sequences, use "All Curves' Handles' Type" to set the mode for all component curves simultaneously.

TODOVISUAL Add screenshot of tangent mode menu

---

## Inspector Panel

When a keyframe or sequence is selected, the inspector shows:

**Keyframe Time**: The frame number where the keyframe is located.

**Keyframe Value**: The value at the keyframe (can be edited directly).

**Tangent Settings**: Handle angles and weights for WEIGHTED mode.

**Representation Type**: Switch between VECTOR and COLOR for vec3/vec4 sequences.

**All Curves' Handles' Type**: For quaternions, set the global handle type.

---

## Keyboard Shortcuts

### General

| Shortcut    | Action                                 |
| ----------- | -------------------------------------- |
| **F**       | Frame all keyframes in view            |
| **Shift+F** | Frame selected keyframes               |
| **Ctrl+A**  | Select all keyframes on visible curves |
| **Del**     | Delete selected keyframes              |
| **Ctrl+Z**  | Undo (if implemented in your engine)   |
| **Ctrl+Y**  | Redo (if implemented in your engine)   |

### Navigation

| Shortcut                        | Action                       |
| ------------------------------- | ---------------------------- |
| **Scroll Wheel**                | Zoom in/out                  |
| **Middle Mouse**                | Pan view                     |
| **Arrow Keys**                  | Pan view in small increments |
| **Arrow Keys** (with selection) | Nudge keyframes              |

### Selection

| Shortcut        | Action                    |
| --------------- | ------------------------- |
| **Click**       | Select single keyframe    |
| **Ctrl+Click**  | Toggle keyframe selection |
| **Shift+Click** | Range select keyframes    |
| **Box Drag**    | Select multiple keyframes |

### Movement

| Shortcut       | Action                         |
| -------------- | ------------------------------ |
| **Drag**       | Move selected keyframes freely |
| **Shift+Drag** | Constrain to horizontal (time) |
| **Alt+Drag**   | Constrain to vertical (value)  |

---

## Mouse Interactions

### Timeline Ruler

**Left Click**: Jump playhead to clicked position.

**Drag**: Scrub through the animation by dragging the playhead.

### Curve Editor

**Left Click**: Select keyframe or handle.

**Double Click**: Create keyframe on curve.

**Right Click**: Open context menu for keyframe, handle, or curve options.

**Middle Mouse**: Pan the view.

**Scroll Wheel**: Zoom in/out at mouse position.

**Drag (empty space)**: Box selection for multiple keyframes.

**Drag (on keyframe)**: Move selected keyframes.

**Drag (on handle)**: Adjust tangent angle and weight.

---

## Tips & Best Practices

**Frame Your Work**: Use **F** frequently to frame all keyframes and see the full animation.

**Zoom to Details**: Zoom in when making precise timing adjustments to keyframes.

**Use Auto Tangents**: Start with AUTO mode for smooth curves, then switch to BROKEN for specific control points.

**Box Selection**: Use box selection for selecting multiple keyframes quickly across different curves.

**Constrained Movement**: Hold **Shift** when you only want to adjust timing, **Alt** when you only want to adjust values.

**Record for Precision**: Use the record button when you need exact values from your scene's current state.

**Check Quaternion Sync**: When animating rotations, remember that all quaternion keyframes must stay synchronized.

**Color vs Vector**: Use COLOR representation when adjusting colors visually, VECTOR when you need precise component control.

---

## Common Workflows

### Creating a Simple Animation

1. Open timeline for editing
2. Click **+** to create a sequence
3. Select entity, component, and field
4. Position playhead at frame 0
5. Click **+keyframe** button
6. Position playhead at frame 60
7. Click **record** button
8. Adjust the component value in your inspector
9. Press **F** to frame the animation
10. Click **Play** to preview

### Adjusting Timing

1. Select the keyframes you want to move
2. Hold **Shift** and drag horizontally to adjust timing
3. Release to commit the change

### Creating Smooth Easing

1. Select the keyframes at the transition
2. Right-click and choose **AUTO** or **SMOOTH** tangent mode
3. Adjust handles if needed for the exact curve shape

### Adding Overshoot

1. Create keyframes at start and end positions
2. Create an intermediate keyframe beyond the end position
3. Adjust tangents to create smooth acceleration and deceleration

---

## Next Steps

- [API Reference](api-reference/overview.md) - Integrate Tanim into your editor
- [Supported Types](supported-types.md) - Learn type-specific behaviors
- [Performance](performance.md) - Optimize your animations
