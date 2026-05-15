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
   1. [API](#api-1)
   2. [Data Members](#data-members)
5. [ResourceManager](#resourcemanager)
   1. [API](#api-2)
   2. [Data Members](#data-members-1)
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

Each config file is identified by a string ID (e.g., `"WindowConfig"`) and the parsed `toml::table` is stored in an internal map.

### API

| Method | Description |
|--------|-------------|
| `loadConfig(configID, filepath)` | Parses a TOML file and stores it under the given string ID. Logs an error and returns early if parsing fails. |
| `getConfigTable(configID)` | Returns a `const toml::table*` for direct TOML access, or `nullptr` if the ID is not found. |
| `getConfigValue<T>(configID, key)` | Looks up a top-level key in the config table. Returns `std::optional<T>` — empty if the key is missing or the type doesn't match. |
| `getConfigValue<T>(configID, section, key)` | Looks up a nested `section → key` in the config table. Returns `std::optional<T>` — empty if the section, key, or type is wrong. |
| `getStringArray(configID, section, key)` | Reads a TOML array of strings from `section → key`. Returns `std::vector<std::string>` (empty if the section, key, or array is missing). |
| `getConfigFiles()` | Returns `const` reference to the internal map of all loaded config tables (`std::map<std::string, toml::table>`). |

All lookup methods log a warning (with the caller's `std::source_location`) on failure, making it easy to find which code path is missing a config value.

#### Usage Examples

```cpp
// Simple key lookup:
auto width = m_ConfigManager->getConfigValue<unsigned int>(
    Assets::Configs::Window, "mainWindow", "X").value_or(800u);

// Nested section/key lookup:
auto speed = m_ConfigManager->getConfigValue<float>(
    "player", "player", "movementSpeed").value_or(350.0f);
```

> **Config IDs are up to you.** Using `Assets::Configs::*` constants (like `Assets::Configs::Window`) is recommended for safety against typos, but **entirely optional**. You can pass raw string IDs and direct filepaths to `loadConfig()` instead.
>
> For example, `EntityFactory::createPlayer()` loads its config with a plain string ID and filepath ([`EntityFactory.cpp`](../src/ECS/EntityFactory.cpp:67)):
>
> ```cpp
> context.m_ConfigManager->loadConfig("player", "config/Player.toml");
> float moveSpeed = context.m_ConfigManager->getConfigValue<float>(
>     "player", "player", "movementSpeed").value_or(350.0f);
> ```
>
> This also means you can load config files at any point — not just during startup — making it easy to keep config data close to where it's used.

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `m_ConfigFiles` | `std::map<std::string, toml::table, std::less<>>` | The underlying storage. Keys are config ID strings (e.g. `"WindowConfig"`), values are the parsed TOML tables. `std::less<>` enables heterogeneous lookup with `std::string_view` keys. |

---

## ResourceManager

**Header:** [`ResourceManager.hpp`](../include/Managers/ResourceManager.hpp)  
**Source:** [`ResourceManager.cpp`](../src/Managers/ResourceManager.cpp)

A typed, exception-free asset manager. It stores resources in per-type `std::map`s keyed by string IDs. Supports `sf::Font`, `sf::Texture`, `sf::SoundBuffer`, and `sf::Music`.

### API

| Method | Description |
|--------|-------------|
| `loadAssetsFromManifest(filepath)` | Parses a TOML manifest file and loads all assets declared in its `[[fonts]]`, `[[textures]]`, `[[soundbuffers]]`, and `[[musics]]` arrays. |
| `loadResource<T>(id, filepath)` | Loads a single resource of type `T` from disk. Logs an error and returns early if the file can't be loaded. |
| `getResource<T>(id)` | Returns `T*` to the cached resource, or `nullptr` if the ID is not found (non-const overload). |
| `getResource<T>(id) const` | Returns `const T*` to the cached resource, or `nullptr` if the ID is not found (const overload). |

If a resource cannot be loaded, the function logs an error and returns `nullptr` — no exceptions are thrown. **Always check the return value** before using a resource.

Resource IDs are centralized in [`AssetKeys.hpp`](../include/AssetKeys.hpp) as compile-time string constants. Always use these constants rather than raw strings to avoid typo bugs:

```cpp
namespace Assets::Fonts
{
    constexpr std::string_view MainFont  = "MainFont";
    constexpr std::string_view ScoreFont = "ScoreFont";
}
```

#### Usage Examples

```cpp
// Load everything from the TOML manifest
m_AppContext.m_ResourceManager->loadAssetsFromManifest("config/AssetsManifest.toml");

// Load a single resource directly
m_AppContext.m_ResourceManager->loadResource<sf::Font>(
    Assets::Fonts::MainFont, "resources/fonts/CaesarDressing-Regular.ttf");

// Retrieve and use a resource
auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
if (!font) { /* handle gracefully */ }
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

### Data Members

| Member | Type | Description |
|--------|------|-------------|
| `m_Fonts` | `std::map<std::string, std::unique_ptr<sf::Font>>` | Stores loaded fonts keyed by string ID. |
| `m_Textures` | `std::map<std::string, std::unique_ptr<sf::Texture>>` | Stores loaded textures keyed by string ID. |
| `m_SoundBuffers` | `std::map<std::string, std::unique_ptr<sf::SoundBuffer>>` | Stores loaded sound buffers keyed by string ID. |
| `m_Musics` | `std::map<std::string, std::unique_ptr<sf::Music>>` | Stores loaded music streams keyed by string ID. |

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
