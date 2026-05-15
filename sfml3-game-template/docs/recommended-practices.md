# Recommended Practices

This document captures patterns and conventions observed in the template codebase, along with general advice for keeping your game loop performant and maintainable.

## Table of Contents

1. [Performance: update vs render](#1-performance-update-vs-render-and-heavy-calculations)
   1. [Keep render() Lean](#keep-render-lean)
   2. [Avoid Per-Frame GPU Copies](#avoid-per-frame-gpu-copies)
   3. [Cache Config Reads](#cache-config-reads)
   4. [Be Mindful of System Queries](#be-mindful-of-system-queries)
   5. [Process Events, Don't Poll in Render](#process-events-dont-poll-in-render)
2. [AppContext & Function Signatures](#2-appcontext--function-signatures)
   1. [Use AppContext as a Service Locator](#use-appcontext-as-a-service-locator)
   2. [Systems Take What They Actually Use](#systems-take-what-they-actually-use)
   3. [EntityFactory Prefab Functions](#entityfactory-prefab-functions)
3. [Common Practices in the Codebase](#3-common-practices-in-the-codebase)
   1. [Deferred State Changes](#deferred-state-changes)
   2. [Null-Check Resources](#null-check-resources)
   3. [Centralized String Constants](#centralized-string-constants)
   4. [RAII Entity Cleanup in State Destructors](#raii-entity-cleanup-in-state-destructors)
   5. [Logger with std::format](#logger-with-stdformat)
   6. [Animation State Machine](#animation-state-machine)
   7. [Service Locator Pattern](#service-locator-pattern)
   8. [Component Queries Use view.get()](#component-queries-use-viewget)
   9. [Flipping Sprites via Negative Scale](#flipping-sprites-via-negative-scale)
   10. [Group-Tag Cleanup](#group-tag-cleanup)
   11. [Sound Playback via AppData::activeSounds](#sound-playback-via-appdataactivesounds)
   12. [Per-Entity Config Files](#per-entity-config-files)
   13. [ECS-Based HUD](#ecs-based-hud)
4. [See Also](#see-also)

---

## 1. Performance: `update()` vs `render()`, and Heavy Calculations

### Keep `render()` Lean

The `render()` phase should **only draw things**. Expensive calculations, physics, AI, and game logic belong in `update()`. Look at how the template separates them:

| State | `update()` does | `render()` does |
|-------|----------------|-----------------|
| `PlayState` | Input handling, movement, animation, facing | Only `CoreSystems::renderSystem()` |
| `MenuState` | `uiHoverSystem()` (mouse hit-testing) | Only `uiRenderSystem()` + title text |

### Avoid Per-Frame GPU Copies

`getSpritePadding()` calls `texture.copyToImage()`, which transfers data from GPU to RAM. This is called **once** in `createPlayer()` during entity construction — never in a system that runs every frame:

```cpp
// GOOD: called once in EntityFactory
SpritePadding padding = utils::getSpritePadding(spriteComp.sprite);

// BAD: never call this inside update() or a system
void mySystem(entt::registry& registry) {
    auto view = registry.view<SpriteComponent>();
    for (auto entity : view) {
        auto padding = utils::getSpritePadding(spriteComp.sprite); // BAD
    }
}
```

### Cache Config Reads

Config file access involves TOML lookups. Read values **once** during initialization and store them in components or local variables rather than querying `ConfigManager` every frame:

```cpp
// GOOD: read in factory, store in component
registry.emplace<MovementSpeed>(entity, moveSpeed);

// BAD: query config every frame
void handlePlayerInput(AppContext& ctx) {
    float speed = ctx.m_ConfigManager->getConfigValue<float>(
        "player", "player", "movementSpeed").value_or(350.0f); // BAD
}
```

### Be Mindful of System Queries

Systems use `registry.view<A, B>()` which is efficient, but if you have a system that iterates thousands of entities, keep the body lightweight. If a system doesn't need to run every frame (e.g., a slow timer or a periodic check), consider adding a frame counter or cooldown:

```cpp
static sf::Time elapsed;
elapsed += deltaTime;
if (elapsed.asSeconds() < 0.5f) return; // Only run twice per second
elapsed = sf::Time::Zero;
```

### Process Events, Don't Poll in Render

Event handling happens in `processEvents()`, not in `render()`. The `handleEvents` SFML 3 API is called once per frame. Real-time input (like WASD) is polled via `sf::Keyboard::isKeyPressed()` inside `update()` — this is correct. Don't move input polling into `render()`.

---

## 2. AppContext & Function Signatures

### Use AppContext as a Service Locator

`AppContext` exists precisely so you don't need to pass a dozen individual pointers to every function. A state or system that needs access to multiple managers, the registry, or the window receives a single `AppContext&`:

```cpp
// GOOD: single reference to the hub
void MySystem::doStuff(AppContext& context) {
    auto& registry = *context.m_Registry;
    auto* font = context.m_ResourceManager->getResource<sf::Font>(...);
    auto& window = *context.m_MainWindow;
}

// BAD: passing individual dependencies
void MySystem::doStuff(
    entt::registry& registry,
    ResourceManager* resMgr,
    sf::RenderWindow& window,
    ConfigManager* cfgMgr,
    ...
);
```

### Systems Take What They Actually Use

While states receive the full `AppContext&`, individual systems should be more specific. A system should only request the data it actually operates on:

```cpp
// GOOD: systems take minimal, specific parameters
void CoreSystems::movementSystem(
    entt::registry& registry,
    sf::Time deltaTime,
    sf::RenderWindow& window
);

void UISystems::uiClickSystem(
    entt::registry& registry,
    const sf::Event::MouseButtonPressed& event
);
```

- If a system only needs the registry, pass only the registry.
- If a system needs the registry plus one other thing, pass those two.
- Avoid giving systems the full `AppContext&` unless they genuinely need multiple managers.

### EntityFactory Prefab Functions

Factory functions follow a consistent pattern:

```cpp
entt::entity createPlayer(AppContext& context, sf::Vector2f position);
entt::entity createButton(AppContext& context, sf::Font& font,
    const std::string& text, sf::Vector2f position,
    std::function<void()> action, ...);
```

- The first parameter is always `AppContext&` (gives access to the registry and resource manager).
- Remaining parameters are the **entity-specific data** that distinguishes this instance.
- They return `entt::entity` (or `entt::null` on failure).

---

## 3. Common Practices in the Codebase

### Deferred State Changes

State changes are never applied immediately. They are queued as `PendingChange` structs and processed at the start of the next frame via `processPending()`. This prevents:

- Mid-frame stack corruption if a state pushes/removes itself during `update()`.
- Recursive state transitions.

```cpp
// In any state:
m_AppContext.m_StateManager->pushState(std::make_unique<PauseState>(m_AppContext));
// The push won't take effect until the next frame's processPending().
```

### Null-Check Resources

Every resource lookup is checked before use. `getResource<T>()` returns `nullptr` on failure — never assume it succeeded:

```cpp
auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
if (!font)
{
    logger::Error("Couldn't load MainFont. Can't draw text.");
    return;  // or fall back gracefully
}
```

This pattern is used consistently in `EntityFactory`, `State` constructors, and `UISystems`.

### Centralized String Constants

Resource IDs and config IDs are never written as inline string literals. They are defined in [`AssetKeys.hpp`](../include/AssetKeys.hpp) as `constexpr std::string_view`:

```cpp
namespace Assets::Fonts
{
    constexpr std::string_view MainFont  = "MainFont";
    constexpr std::string_view ScoreFont = "ScoreFont";
}
namespace Assets::Configs
{
    constexpr std::string_view Window = "WindowConfig";
}
```

Always add your keys here rather than writing strings directly. This catches typos at compile time and makes renaming a resource a single-point change.

### RAII Entity Cleanup in State Destructors

Each state is responsible for cleaning up the entities it created. The destructor iterates over entities with a matching `UITagID` (for UI states) or a specific tag component (for gameplay states) and destroys them:

```cpp
PlayState::~PlayState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<PlayerTag>();
    registry.destroy(view.begin(), view.end());
}
```

```cpp
MenuState::~MenuState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Menu)
            registry.destroy(entity);
    }
}
```

This prevents entity/component leaks when switching between states.

### Logger with `std::format`

Rather than string concatenation, the codebase uses `std::format` for log messages:

```cpp
logger::Info(std::format("Player health: {}", playerHealth));
logger::Error(std::format("Failed to load: {}", filepath));
```

All `logger` functions accept a single `std::string_view`. Use `std::format` (or `std::println`) to build the message string.

### Animation State Machine

Animation switching follows a **guard pattern** — only reset when the animation name changes:

```cpp
if (velocity.value.x != 0.0f || velocity.value.y != 0.0f)
{
    if (animator.currentAnimationName != "walk")  // guard
    {
        animator.currentAnimationName = "walk";
        animator.currentFrame = 0;
        animator.elapsedTime = sf::Time::Zero;
    }
}
```

This prevents resetting the animation to frame 0 every frame while the entity is moving. Without this guard, a 4-frame animation would never advance past frame 0.

### Service Locator Pattern

`AppContext` is passed by reference through the entire call chain. It is never a global or a singleton. This keeps dependencies explicit and makes testing easier — you can construct an `AppContext` with mock managers if needed.

### Component Queries Use `view.get<>()`

When iterating with `registry.view<A, B>()`, access components via `view.get<A>(entity)` rather than `registry.get<A>(entity)`. The view caches the component arrays, making this faster:

```cpp
auto view = registry.view<SpriteComponent, Velocity>();
for (auto entity : view)
{
    auto& sprite = view.get<SpriteComponent>(entity);  // faster
    auto& vel    = registry.get<Velocity>(entity);     // unnecessary lookup
}
```

### Flipping Sprites via Negative Scale

Instead of having left-facing and right-facing sprite assets, the template flips the sprite horizontally with a negative X scale:

```cpp
if (facing.dir == FacingDirection::Left)
    spriteComp.sprite.setScale({ -baseScale.value.x, baseScale.value.y });
else
    spriteComp.sprite.setScale(baseScale.value);
```

The `BaseScale` component stores the original positive scale so it can be toggled cleanly. This pattern is used in `CoreSystems::facingSystem()`.

### Group-Tag Cleanup

Rather than cleaning entities one tag at a time, use a **group tag** to mark all entities of a category. In [Breakdown](https://github.com/nantr0nic/breakdown), every game-renderable entity gets `RenderableTag` and every HUD entity gets `HUDTag`. The state destructor then destroys them in two bulk calls ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/State.cpp)):

```cpp
PlayState::~PlayState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto gameView = registry.view<RenderableTag>();
    registry.destroy(gameView.begin(), gameView.end());

    auto hudView = registry.view<HUDTag>();
    registry.destroy(hudView.begin(), hudView.end());
}
```

### Sound Playback via `AppData::activeSounds`

Short sound effects (like collision sounds) are managed through a `std::list<sf::Sound>` stored in `AppData`. The `playSound()` utility ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/Systems.cpp)) loads a `SoundBuffer`, creates a new `sf::Sound`, plays it, and cleans up finished sounds each call:

```cpp
void CoreSystems::playSound(AppContext& context, std::string_view soundID)
{
    if (context.m_AppSettings.sfxMuted) return;

    // Remove finished sounds
    context.m_AppData.activeSounds.remove_if([](const sf::Sound& s) {
        return s.getStatus() == sf::Sound::Status::Stopped;
    });

    auto* buffer = context.m_ResourceManager->getResource<sf::SoundBuffer>(soundID);
    if (!buffer) return;

    context.m_AppData.activeSounds.emplace_back(*buffer);
    context.m_AppData.activeSounds.back().setVolume(context.m_AppSettings.sfxVolume);
    context.m_AppData.activeSounds.back().play();
}
```

This keeps sound management out of the ECS registry and lets short-lived sounds clean themselves up automatically.

### Per-Entity Config Files

Instead of putting all configuration in one file, split by entity type. [Breakdown](https://github.com/nantr0nic/breakdown) has separate TOML files for the player paddle, ball, bricks, and levels:

- [`config/Ball.toml`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Ball.toml) — radius, speed, color
- [`config/Bricks.toml`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Bricks.toml) — per-brick-type score, health, colors
- [`config/Levels.toml`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Levels.toml) — level layouts as ASCII grids

Each file is loaded on demand by the factory that needs it:
```cpp
context.m_ConfigManager->loadConfig(Assets::Configs::Ball, "config/Ball.toml");
float radius = context.m_ConfigManager->getConfigValue<float>(
    Assets::Configs::Ball, "ball", "ballRadius").value_or(25.0f);
```

### ECS-Based HUD

Instead of raw SFML draw calls for HUD elements, represent them as ECS entities. In Breakdown the score display ([factory source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/EntityFactory.cpp)) has components `HUDTag + ScoreHUDTag + CurrentScore + UIText`. Systems update the text ([collision system source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/Systems.cpp)) by modifying the `UIText` component directly:

```cpp
auto scoreView = registry.view<ScoreHUDTag, UIText>();
for (auto entity : scoreView)
    scoreView.get<UIText>(entity).text.setString(
        std::format("Score: {}", newScore));
```

This keeps HUD rendering inside `UISystems::uiRenderSystem()` and avoids special-case draw code in states.

---

## See Also

- [Game Loop](game-loop.md) — how the frame is structured
- [ECS](ecs.md) — system architecture and entity lifecycle
- [Managers](managers.md) — how AppContext and the managers work
- [Utilities](utilities.md) — details on logger, random, and helpers
