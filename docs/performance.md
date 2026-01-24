# Performance

Profiling results, scalability analysis, and optimization considerations for Tanim.

## Overview

This document presents performance testing results from stress tests and scalability analysis. All tests were conducted using Superluminal profiler on a release configuration with debug information.

---

## Time Complexity

Tanim's animation system has **O(n) linear time complexity** where n is the number of animated entities.

**Constant time per entity**: Approximately 1ms per 1000 entities in typical scenarios.

**60 FPS limit**: Up to approximately 15,000 animated entities can be updated at 60 FPS.

---

## Scalability Test Results

Tests were conducted by increasing the number of animated entities and measuring the time spent in the animation update system.

### Raw Data

| Animated Entity Count | Time Spent (ms) |
| :-------------------: | :-------------: |
|          10           |      0.024      |
|          100          |      0.094      |
|         1,000         |      0.987      |
|        10,000         |     10.152      |
|        100,000        |      107.8      |

### Scalability Graph

TODOVISUAL Add logarithmic line graph showing scalability (from ILO3)

### Analysis

The profiling data confirms linear O(n) time complexity:

- Performance scales predictably with entity count
- No exponential growth or performance cliffs
- Consistent ~1ms per 1000 entities

**60 FPS Budget**: At 60 FPS, you have ~16.67ms per frame. With Tanim using ~1ms per 1000 entities, the practical limit is around 15,000 animated entities before animation alone consumes the full frame budget.

---

## Performance Best Practices

### For Users

**Cache Entity Lists**: Use the `UpdateCachedDataIfEmpty` pattern shown in the example implementation to avoid rebuilding entity lists every frame.

**Share Timeline Data**: When multiple entities use the same animation, share the `TimelineData` rather than duplicating it. This reduces memory usage and maintains consistency.

**Optimize FindEntityOfUID**: Your `FindEntityOfUID` implementation is called frequently. Use cached lookups in `ComponentData::m_user_data` rather than searching the entire registry.

**Limit Animated Entities**: For performance-critical scenarios, only animate entities that are visible or relevant to gameplay.

**Use Simpler Types**: Animating a `float` is faster than animating a `glm::quat` with 5 curves. Use the simplest type that meets your needs.

---

## Next Steps

- [API Reference](api-reference/overview.md) - See API functions and their performance characteristics
- [Example Implementation](example-implementation.md) - Follow caching patterns for optimal performance
- [Core Concepts](core-concepts.md) - Understand the data structures that affect performance
