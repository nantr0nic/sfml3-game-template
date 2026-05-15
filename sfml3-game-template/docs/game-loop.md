# Game Loop

This document explains how the application starts, how the main loop works, and how events are dispatched.

## Table of Contents

1. [Entry Point](#entry-point)
2. [Application Construction](#application-construction)
3. [The Main Loop](#the-main-loop)
   1. [processPending()](#1-processpending)
   2. [processEvents()](#2-processevents)
   3. [update(deltaTime)](#3-updatedeltatime)
   4. [render()](#4-render)
4. [Delta Time](#delta-time)
5. [Event Merging Detail](#event-merging-detail)
6. [See Also](#see-also)

---

## Entry Point

The program begins in [`Main.cpp`](../src/Main.cpp):

```cpp
int main()
{
    Application app;
    app.run();
    return 0;
}
```

`Application` is constructed once, then `run()` is called — that's the entire lifecycle.

---

## Application Construction

[`Application::Application()`](../src/Application.cpp#L13-L26) does four things in order:

1. **Construct `AppContext`** — This builds all managers internally (see [Managers](managers.md)). The config manager loads `WindowConfig.toml` first, then the window manager uses those values.
2. **`initMainWindow()`** — Creates the SFML render window using the config's width, height, and title. Sets a 60 FPS framerate limit.
3. **`initResources()`** — Loads all assets from `config/AssetsManifest.toml` (fonts, textures, sound buffers, music).
4. **Push the initial state** — A `MenuState` is created and pushed onto the state stack.

---

## The Main Loop

[`Application::run()`](../src/Application.cpp#L36-L51) is a tight loop that runs until the window closes:

```
while window is open:
    deltaTime = clock.restart()
    stateManager.processPending()
    processEvents()
    update(deltaTime)
    render()
```

### 1. `processPending()`

State changes (push, pop, replace) are **deferred** — they don't take effect immediately. Instead, the `StateManager` queues them as `PendingChange` structs. At the start of each frame, `processPending()` applies all queued changes at once. This prevents bugs where a state change mid-update could invalidate iterators or leave the stack in an inconsistent state.

### 2. `processEvents()`

The method first guards against a missing state — if `getCurrentState()` returns `nullptr`, it logs an error and drains the event queue before returning.

Events are then dispatched in a layered fashion:

- **Global events** (from `GlobalEventManager`) run first — `onClose` closes the window, and `onGlobalKeyPress` maps Escape → close window.
- **State-specific events** run second. `onKeyPress` is **merged** with the global handler: the global one fires first, then the state handler. `onMouseButtonPress` is passed as a standalone callback.
- **Window resize** is handled inline: the view is recalculated with `utils::boxView()` to maintain the target aspect ratio (letterboxing/pillarboxing).

The `handleEvents` call uses SFML 3's event-handling API where each event type has a dedicated callback.

### 3. `update(deltaTime)`

Delegates to `StateManager::update()`, which calls `update()` on the **topmost state** only. States lower on the stack are not updated (they are effectively suspended).

### 4. `render()`

The window is cleared to black, then `StateManager::render()` is called. Unlike update, render iterates **all** states in the stack — this allows overlay states (e.g., a pause menu) to draw on top of the state beneath them. Finally `window.display()` swaps the buffers.

---

## Delta Time

The main clock (`sf::Clock`) lives in `AppContext`. Each frame `restart()` returns a `sf::Time` delta that is passed through the entire update chain:

```
Application::run()
  → StateManager::update(deltaTime)
    → PlayState::update(deltaTime)
      → CoreSystems::movementSystem(registry, deltaTime, window)
      → CoreSystems::animationSystem(registry, deltaTime)
```

All systems that depend on frame timing receive `deltaTime` explicitly. This keeps them deterministic and testable.

---

## Event Merging Detail

The event pipeline in [`Application::processEvents()`](../src/Application.cpp#L53-L88) merges global and state-specific handlers:

```
onClose               → passed directly (global only)
onKeyPressMerged      → first call globalEvents.onGlobalKeyPress,
                        then call stateEvents.onKeyPress
onMouseButtonPress    → passed directly (state only)
onResized            → handled inline with boxView
```

The `onResized` handler recalculates the SFML view so the game's target resolution is letterboxed inside the actual window. This keeps rendering resolution-independent.

> **Note:** Line numbers in this doc reference the current source and may drift over time.

---

## See Also

- [Managers](managers.md) — details on StateManager, WindowManager, etc.
- [ECS](ecs.md) — how systems inside states work
- [Getting Started](getting-started.md) — adding your own states and game logic
