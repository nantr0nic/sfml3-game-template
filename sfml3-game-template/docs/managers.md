# Managers

This document covers the five manager classes that form the template's service layer, and the `AppContext` struct that bundles them together.

## Table of Contents

1. [AppContext — The Service Locator](#appcontext--the-service-locator)
2. [StateManager](#statemanager)
   1. [API](#api)
   2. [Deferred Changes Pattern](#deferred-changes-pattern)
   3. [Example: Pausing](#example-pausing)
3. [WindowManager](#windowmanager)
4. [ConfigManager](#configmanager)
   1. [Loading](#loading)
   2. [Access](#access)
5. [ResourceManager](#resourcemanager)
   1. [Loading](#loading-1)
   2. [Access](#access-1)
6. [GlobalEventManager](#globaleventmanager)
7. [See Also](#see-also)

---

## AppContext — The Service Locator

[`AppContext`](../include/AppContext.hpp) is the central hub of the application. Every game state and system receives a reference to it. It stores:

- **Managers** (as `unique_ptr`): `ConfigManager`, `WindowManager`, `ResourceManager`, `GlobalEventManager`
- **`StateManager`** — owned directly by `Application` as a member variable; `AppContext` holds a **pointer** to it (`StateManager*`)
- **ECS registry** (`unique_ptr<entt::registry>`)
- **Main clock** (`unique_ptr<sf::Clock>`)
- Application settings and runtime data (`AppSettings` + `AppData`)
- **Pointer** to the main window (`sf::RenderWindow*`)

The construction order inside `AppContext` is intentional:

```
ConfigManager   ← loaded first (so other managers can read config)
WindowManager   ← needs config
ResourceManager
GlobalEventManager
MainClock         ← just an sf::Clock, not a manager class
ECS Registry
```

This avoids initialization-order problems — `WindowManager`, for example, reads window dimensions from config files that `ConfigManager` has already parsed.

---

## StateManager

**Header:** [`StateManager.hpp`](../include/Managers/StateManager.hpp)  
**Source:** [`StateManager.cpp`](../src/Managers/StateManager.cpp)

A stack-based finite state machine. States are `unique_ptr<State>` stored in a vector.

### API

| Method | Description |
|--------|-------------|
| `pushState(state)` | Queue a state to be added on top of the stack. |
| `popState()` | Queue the top state to be removed. |
| `replaceState(state)` | Queue a state to replace the top (pop then push). |
| `processPending()` | Apply all queued changes. Called once per frame. |
| `getCurrentState()` | Returns the topmost state, or `nullptr`. |
| `update(deltaTime)` | Calls `update()` on the top state only. |
| `render()` | Calls `render()` on **all** states (bottom to top). |

### Deferred Changes Pattern

State changes are never applied immediately — they are stored in `m_PendingChanges`. At the start of each frame, `processPending()` moves them into the live stack. This avoids:

- **Iterator invalidation** — systems iterating the registry or the state vector aren't disrupted mid-frame.
- **Recursive state changes** — a state's `update()` can safely call `pushState()` or `popState()` without corrupting the stack.

### Example: Pausing

In `PlayState`, pressing `P` calls:

```cpp
m_AppContext.m_StateManager->pushState(std::make_unique<PauseState>(m_AppContext));
```

The `PauseState` renders on top of `PlayState` (because `StateManager::render()` draws all states) but only the pause state receives `update()` calls.

---

## WindowManager

**Header:** [`WindowManager.hpp`](../include/Managers/WindowManager.hpp)  
**Source:** [`WindowManager.cpp`](../src/Managers/WindowManager.cpp)

Wraps `sf::RenderWindow` creation. The main window can be created either from config file values or from explicit parameters.

| Method | Description |
|--------|-------------|
| `createMainWindow()` | Reads width, height, and title from `WindowConfig.toml`. |
| `createMainWindow(w, h, title)` | Creates a window with explicit dimensions. |
| `getMainWindow()` | Returns a reference to the internal `sf::RenderWindow`. |

If `createMainWindow()` is called when a window already exists, it logs an error and returns `false`.

---

## ConfigManager

**Header:** [`ConfigManager.hpp`](../include/Managers/ConfigManager.hpp)  
**Source:** [`ConfigManager.cpp`](../src/Managers/ConfigManager.cpp)

Manages TOML configuration files using [toml++](https://github.com/marzer/tomlplusplus).

### Loading

```cpp
m_ConfigManager->loadConfig(Assets::Configs::Window, "config/WindowConfig.toml");
```

Each config file is identified by a string ID (e.g., `"WindowConfig"`) and stored as a `toml::table`.

### Access

```cpp
// Simple key lookup:
auto width = m_ConfigManager->getConfigValue<unsigned int>(
    Assets::Configs::Window, "mainWindow", "X").value_or(800u);

// Nested section/key lookup:
auto speed = m_ConfigManager->getConfigValue<float>(
    "player", "player", "movementSpeed").value_or(350.0f);
```

The `getStringArray()` method handles TOML arrays of strings. Every failed lookup logs a warning with the caller's source location (using `std::source_location`), making it easy to find which code path is missing a config value.

---

## ResourceManager

**Header:** [`ResourceManager.hpp`](../include/Managers/ResourceManager.hpp)  
**Source:** [`ResourceManager.cpp`](../src/Managers/ResourceManager.cpp)

A typed, exception-free asset manager. It stores resources in separate `std::map`s keyed by string IDs:

- `sf::Font`
- `sf::Texture`
- `sf::SoundBuffer`
- `sf::Music`

### Loading

Assets are loaded from a TOML manifest:

```cpp
m_AppContext.m_ResourceManager->loadAssetsFromManifest("config/AssetsManifest.toml");
```

The manifest file has sections for each resource type:

```toml
[[fonts]]
id = "MainFont"
path = "resources/fonts/CaesarDressing-Regular.ttf"

[[textures]]
id = "PlayerSpriteSheet"
path = "resources/sprites/knight.png"

[[musics]]
id = "MainSong"
path = "resources/music/VideoGameAm.ogg"
```

### Access

```cpp
auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
if (!font) { /* handle gracefully */ }
```

Resource IDs are centralized in [`AssetKeys.hpp`](../include/AssetKeys.hpp) as compile-time string constants. Always use these constants rather than raw strings to avoid typo bugs:

```cpp
namespace Assets::Fonts
{
    constexpr std::string_view MainFont  = "MainFont";
    constexpr std::string_view ScoreFont = "ScoreFont";
}
```

If a resource cannot be loaded, the function logs an error and returns `nullptr` — no exceptions are thrown. Always check the return value before using a resource.

---

## GlobalEventManager

**Header:** [`GlobalEventManager.hpp`](../include/Managers/GlobalEventManager.hpp)  
**Source:** [`GlobalEventManager.cpp`](../src/Managers/GlobalEventManager.cpp)

Holds `ApplicationEvents` — a set of `std::function` delegates for application-wide events:

| Delegate | Default Behavior |
|----------|------------------|
| `onClose` | Close the main window. |
| `onGlobalKeyPress` | Escape key → close the window. |

These are merged with state-specific handlers inside `Application::processEvents()` (see [Game Loop](game-loop.md#event-merging-detail)). The global handlers run **before** the state handler, so global behavior (e.g., Escape to quit) can be overridden by a state's handler if needed.

---

## See Also

- [Game Loop](game-loop.md) — how managers are used each frame
- [ECS](ecs.md) — how the registry inside AppContext is used
- [Getting Started](getting-started.md) — configuration files and asset setup
