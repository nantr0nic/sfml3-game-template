# Entity-Component-System (ECS)

This template uses the [EnTT](https://github.com/skypjack/entt) library for its ECS architecture. This document explains how the ECS is structured, how to add new components and prefabs, and how systems operate on entities.

## Table of Contents

1. [The ECS Mindset](#the-ecs-mindset)
2. [About EnTT](#about-entt)
   1. [How This Codebase Uses EnTT](#how-this-codebase-uses-entt)
3. [Common EnTT Calls](#common-entt-calls)
   1. [Creating & Destroying Entities](#creating--destroying-entities)
   2. [Adding, Reading & Removing Components](#adding-reading--removing-components)
   3. [Iterating with Views](#iterating-with-views)
   4. [Full Examples](#full-examples)
4. [Components](#components)
   1. [Game Components](#game-components)
   2. [UI Components](#ui-components)
   3. [Adding a New Component](#adding-a-new-component)
5. [EntityFactory — Creating Prefabs](#entityfactory--creating-prefabs)
   1. [Existing Prefabs](#existing-prefabs)
   2. [Creating Your Own Prefab](#creating-your-own-prefab)
   3. [Key Points About Factories](#key-points-about-factories)
   4. [Real-world Example: HUD Score Display](#real-world-example-hud-score-display)
6. [Systems — Operating on Entities](#systems--operating-on-entities)
   1. [CoreSystems](#coresystems)
   2. [UISystems](#uisystems)
   3. [How Systems Use the Registry](#how-systems-use-the-registry)
   4. [Real-world Example: Collision System](#real-world-example-collision-system)
   5. [Adding a System for Your Own Prefab](#adding-a-system-for-your-own-prefab)
7. [The Separation in Practice](#the-separation-in-practice)
8. [States and Systems](#states-and-systems)
   1. [Cleanup Pattern: Bulk Tag Destruction](#cleanup-pattern-bulk-tag-destruction)
9. [See Also](#see-also)

---

## The ECS Mindset

The core idea is a strict **separation of concerns**:

| Layer | Role | Files |
|-------|------|-------|
| **Component** | Pure data. Simple structs with no logic. | [`Components.hpp`](../include/ECS/Components.hpp) |
| **Entity** | An ID (an `entt::entity`) that bundles components together. Created by factories or directly in the registry. | [`EntityFactory.hpp`](../include/ECS/EntityFactory.hpp) / [`EntityFactory.cpp`](../src/ECS/EntityFactory.cpp) |
| **System** | Logic that operates on entities with a specific set of components. | [`Systems.hpp`](../include/ECS/Systems.hpp) / [`Systems.cpp`](../src/ECS/Systems.cpp) |

- **Components don't know about entities.** A `Velocity` struct just holds an `sf::Vector2f` — it doesn't know which entity it belongs to.
- **Entities don't contain logic.** An entity is just a collection of components.
- **Systems don't own data.** A system queries the registry for component combinations and reads/writes them.

This avoids the deep inheritance hierarchies common in OOP game design and makes it easy to compose new behaviors by adding or removing components at runtime.

---

## About EnTT

The template uses [**EnTT**](https://github.com/skypjack/entt), a header-only C++ ECS library, as its entity-component-system backbone. EnTT is designed around **data-oriented** principles — components are stored in contiguous arrays (one per type), so iterating over entities with a specific set of components is cache-friendly and fast.

### How This Codebase Uses EnTT

- A single **`entt::registry`** lives in `AppContext` (`m_Registry`) and is shared by all states and systems.
- **Entities** are created via `registry.create()` inside `EntityFactory` prefab functions.
- **Components** are plain structs defined in `Components.hpp` and attached via `registry.emplace<Component>(entity, ...)`.
- **Systems** query the registry with `registry.view<ComponentA, ComponentB>()` to iterate over only the entities that have those components.
- **Cleanup** happens in state destructors by destroying entities matching specific tags.

> **Note:** For a thorough introduction to EnTT's API and philosophy, see the [official EnTT ECS crash course](https://github.com/skypjack/entt/wiki/Entity-Component-System#crash-course-entity-component-system).

---

## Common EnTT Calls

Below are the EnTT operations used most frequently in this codebase, grouped by purpose. Each includes a real example drawn from the template source.

### Creating & Destroying Entities

| Call | Purpose | Example (from `EntityFactory.cpp`) |
|------|---------|------------------------------------|
| `registry.create()` | Create a new entity handle | `auto playerEntity = registry.create();` |
| `registry.destroy(entity)` | Destroy a single entity and all its components | `registry.destroy(entity);` |
| `registry.destroy(begin, end)` | Destroy a range of entities in one call | `registry.destroy(view.begin(), view.end());` |
| `entt::null` | Sentinel representing an invalid entity | `return entt::null;` (on resource-load failure) |

### Adding, Reading & Removing Components

| Call | Purpose | Example (from source) |
|------|---------|-----------------------|
| `registry.emplace<C>(entity, args...)` | Attach a new component with constructor args | `registry.emplace<Velocity>(playerEntity);` / `registry.emplace<SpriteComponent>(entity, sf::Sprite(*texture));` |
| `registry.emplace_or_replace<C>(entity, args...)` | Attach or replace a component | `registry.emplace_or_replace<UIHover>(entity);` |
| `registry.get<C>(entity)` | Get a **mutable reference** to an existing component | `auto& buttonBounds = registry.get<UIBounds>(buttonEntity);` |
| `registry.try_get<C>(entity)` | Safely try to read a component (returns `nullptr` if absent) | `if (auto* bounds = registry.try_get<ConfineToWindow>(entity))` |
| `registry.all_of<C>(entity)` | Check if an entity has a component (or multiple) | `if (registry.all_of<UIHover>(entity))` / `if (registry.all_of<UIAction, UIBounds>(textEntity))` |
| `registry.remove<C>(entity)` | Remove a component from an entity | `registry.remove<UIHover>(entity);` |
| `registry.clear<C>()` | Remove a component type from **all** entities | Used for bulk cleanup |

### Iterating with Views

| Call | Purpose | Example (from `Systems.cpp`) |
|------|---------|------------------------------|
| `registry.view<A, B>()` | Get a view of all entities that have **all** listed components | `auto view = registry.view<PlayerTag, Velocity, MovementSpeed, AnimatorComponent, SpriteComponent, Facing>();` |
| `view.get<C>(entity)` | Read a component from within a view (faster than `registry.get`) | `auto& velocity = view.get<Velocity>(entity);` |
| `view.each()` | Iterate with a callback that unpacks components | `for (auto [entity, tag] : view.each())` (used in state destructors) |

> **Performance tip:** When iterating with a view, always use `view.get<C>(entity)` rather than `registry.get<C>(entity)`. The view already has the component storage cached, so the lookup is faster.

### Full Examples

This annotated snippet from `CoreSystems::movementSystem()` shows several EnTT calls working together:

```cpp
void CoreSystems::movementSystem(entt::registry& registry, sf::Time deltaTime, sf::RenderWindow& window)
{
    // Create a view of all entities with both SpriteComponent AND Velocity
    auto view = registry.view<SpriteComponent, Velocity>();

    for (auto entity : view)                     // iterate matching entities
    {
        auto& spriteComp = view.get<SpriteComponent>(entity);  // fast path via view
        const auto& velocity = view.get<Velocity>(entity);

        spriteComp.sprite.move(velocity.value * deltaTime.asSeconds());

        // try_get: safely check for optional component without a full view join
        if (auto* bounds = registry.try_get<ConfineToWindow>(entity))
        {
            // Enforce window confinement using bounds padding
            // ...
        }
    }
}
```

Here is another example of a movement system using EnTT from [Breakdown](https://github.com/nantr0nic/breakdown/blob/ae81e9aabe4c7c673d7d5dcc2e4fa0ed106d0b4f/breakdown/src/ECS/Systems.cpp#L67):

```cpp
void movementSystem(AppContext& context, sf::Time deltaTime)
{
    auto& registry = context.m_Registry;
    bool levelStarted = context.m_AppData.levelStarted;

    auto paddleView = registry->view<Paddle, Velocity>();
    for (auto paddleEntity : paddleView)
    {
        auto& paddleComp = paddleView.get<Paddle>(paddleEntity);
        const auto& velocity = paddleView.get<Velocity>(paddleEntity);

        paddleComp.shape.move(velocity.value * deltaTime.asSeconds());
    }

    if (levelStarted)
    {
        auto ballView = registry->view<Ball, Velocity>();
        for (auto ballEntity : ballView)
        {
            auto& ballShape = ballView.get<Ball>(ballEntity);
            const auto& velocity = ballView.get<Velocity>(ballEntity);

            ballShape.shape.move(velocity.value * deltaTime.asSeconds());
        }
    }
    else
    {
        auto paddleOnlyView = registry->view<Paddle>();
        sf::Vector2f paddlePosition{};
        sf::Vector2f paddleSize{};

        for (auto paddleEntity : paddleOnlyView)
        {
            auto& paddleComp = paddleOnlyView.get<Paddle>(paddleEntity);
            paddlePosition = paddleComp.shape.getPosition();
            paddleSize = paddleComp.shape.getSize();
            break;
        }

        auto ballView = registry->view<Ball>();
        for (auto ballEntity : ballView)
        {
            auto& ballComp = ballView.get<Ball>(ballEntity);
            float ballRadius = ballComp.shape.getRadius();

            float x = paddlePosition.x;
            float y = paddlePosition.y - paddleSize.y / 2.0f - ballRadius;

            ballComp.shape.setPosition({ x, y });
        }
    }

}
```

---

## Components

Components are plain structs defined in [`Components.hpp`](../include/ECS/Components.hpp). They fall into two categories:

### Game Components

| Component | Data | Purpose |
|-----------|------|---------|
| `PlayerTag` | (empty tag) | Marks the player entity. |
| `Velocity` | `sf::Vector2f value` | Movement direction/speed per frame. |
| `MovementSpeed` | `float value` | Base movement speed in pixels/second. |
| `SpriteComponent` | `sf::Sprite sprite` | Drawable sprite. |
| `Facing` | `FacingDirection dir` | Which way the entity faces (Left/Right). |
| `BaseScale` | `sf::Vector2f value` | Base sprite scale (used for flipping). |
| `ConfineToWindow` | Four `float` padding values | Keeps the sprite inside the window bounds. |
| `BoundaryHits` | Four `bool` flags | Tracks which boundaries have been hit. |
| `AnimatorComponent` | Animation map, current frame/timer | Frame-based sprite animation. |
| `Animation` | Row, frame count, duration | A single animation clip definition. |
| `RenderableCircle` | `sf::CircleShape` | Simple circle rendering (non-textured). |
| `RenderableRect` | `sf::RectangleShape` | Simple rectangle rendering (non-textured). |

### UI Components

| Component | Data | Purpose |
|-----------|------|---------|
| `UITagID` | `UITags id` | Tags a UI entity with a group (Menu, Settings, Pause, Transition). |
| `UIShape` | `sf::RectangleShape shape` | Button background shape. |
| `UIText` | `sf::Text text` | Button label text. |
| `UIBounds` | `sf::FloatRect rect` | Click/hover boundary for the UI element. |
| `UIAction` | `std::function<void()> action` | Callback invoked on click. |
| `UIHover` | (empty tag) | Added at runtime when the mouse hovers over a UI element. |
| `GUIButtonTag` | (empty tag) | Marks an entity as a GUI sprite button. |
| `GUISprite` | `sf::Sprite sprite` | Texture-based GUI element. |
| `GUIRedX` | `sf::Sprite sprite` | Red X overlay used for mute toggle indicators. |
| `UIToggleCond` | `std::function<bool()> shouldShowOverlay` | Condition for showing the red X overlay. |
> **Alternative approach — per-state tag components** (used in [Breakdown](https://github.com/nantr0nic/breakdown)):
> Instead of a single `UITagID` with an enum, you can define separate tag components per UI state:
> ```cpp
> struct MenuUITag {};
> struct SettingsUITag {};
> struct TransUITag {};
> struct PauseUITag {};
> ```
> These are defined in [Components.hpp](https://github.com/nantr0nic/breakdown/blob/main/breakdown/include/ECS/Components.hpp).
> This simplifies cleanup — you destroy all entities with that exact tag component in one view call:
> ```cpp
> registry.view<MenuUITag>();      // Get all menu entities
> registry.destroy(view.begin(), view.end());  // Destroy them in one call
> ```
> See [State.cpp](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/State.cpp) for the cleanup in action.
> No need to loop and check an enum value. Both approaches are valid — use whichever feels cleaner for your project.

### Adding a New Component

Define your component as a plain struct or tag:

```cpp
// Tag component — marks an entity as an enemy
struct EnemyTag {};

// Data component — holds health data
struct Health
{
    int current{ 100 };
    int max{ 100 };
};

// Data component with constructor
struct Bullet
{
    Bullet(sf::Vector2f direction, float speed)
        : direction(direction), speed(speed) {}

    sf::Vector2f direction;
    float speed;
    float lifetime{ 3.0f }; // seconds before auto-removal
};
```

Put new components in [`Components.hpp`](../include/ECS/Components.hpp) in the relevant section.

---

## EntityFactory — Creating Prefabs

[`EntityFactory`](../include/ECS/EntityFactory.hpp) is a namespace of factory functions that assemble entities by creating an `entt::entity` and attaching the right components. This is where you define **prefabs** — reusable entity templates.

### Existing Prefabs

#### `createPlayer()`

Assembles a player entity with: `PlayerTag`, `MovementSpeed`, `Velocity`, `Facing`, `SpriteComponent`, `BaseScale`, `ConfineToWindow`, `AnimatorComponent`, and default animation definitions (`"idle"` and `"walk"`).

```cpp
auto player = EntityFactory::createPlayer(m_AppContext, { 400.0f, 300.0f });
```

#### `createButton()`

Creates a clickable UI button with `UITagID`, `UIShape`, `UIText`, `UIBounds`, and `UIAction`.

```cpp
EntityFactory::createButton(m_AppContext, *font, "Play", center,
    [this]() { /* callback */ });
```

### Creating Your Own Prefab

Here's a complete example of adding an enemy prefab:

**1.** Add any new components to [`Components.hpp`](../include/ECS/Components.hpp):

```cpp
struct EnemyTag {};
struct Health { int current{ 100 }; };
struct Damage { int value{ 10 }; };
```

**2.** Add the factory function to [`EntityFactory.hpp`](../include/ECS/EntityFactory.hpp) and implement it in [`EntityFactory.cpp`](../src/ECS/EntityFactory.cpp):

```cpp
// In EntityFactory.hpp
entt::entity createEnemy(AppContext& context, sf::Vector2f position, int health);

// In EntityFactory.cpp
entt::entity EntityFactory::createEnemy(AppContext& context, sf::Vector2f position, int health)
{
    auto& registry = *context.m_Registry;
    auto* texture = context.m_ResourceManager->getResource<sf::Texture>(Assets::Textures::Enemy);

    if (!texture)
    {
        logger::Error("Couldn't create Enemy because missing texture.");
        return entt::null;
    }

    auto entity = registry.create();

    registry.emplace<EnemyTag>(entity);
    registry.emplace<Health>(entity, health);
    registry.emplace<Velocity>(entity);
    registry.emplace<MovementSpeed>(entity, 120.0f);

    auto& spriteComp = registry.emplace<SpriteComponent>(entity, sf::Sprite(*texture));
    spriteComp.sprite.setPosition(position);
    utils::centerOrigin(spriteComp.sprite);
    spriteComp.sprite.setScale({ 2.0f, 2.0f });

    registry.emplace<Facing>(entity);
    registry.emplace<BaseScale>(entity, sf::Vector2f{ 2.0f, 2.0f });

    logger::Info("Enemy created.");
    return entity;
}
```

**3.** Use it in a state:

```cpp
// Inside PlayState
EntityFactory::createEnemy(m_AppContext, { 500.0f, 200.0f }, 50);
```

### Key Points About Factories

- Factories **assemble** entities — they don't contain game logic.
- They return the `entt::entity` handle so you can attach more components later.
- They check for missing resources (e.g., textures) and return `entt::null` on failure.
- They can read from config files for data-driven values (like `createPlayer()` does).

### Real-world Example: HUD Score Display

[Breakdown](https://github.com/nantr0nic/breakdown) creates a HUD score display as an ECS entity — it attaches `HUDTag`, `ScoreHUDTag`, `CurrentScore`, and `UIText` components ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/EntityFactory.cpp)):

```cpp
entt::entity EntityFactory::createScoreDisplay(AppContext &context,
    sf::Font& font, unsigned int size, const sf::Color& color, sf::Vector2f position)
{
    auto& registry = *context.m_Registry;
    auto scoreEntity = registry.create();

    registry.emplace<HUDTag>(scoreEntity);
    registry.emplace<ScoreHUDTag>(scoreEntity);
    registry.emplace<CurrentScore>(scoreEntity, 0);

    auto& scoreText = registry.emplace<UIText>(scoreEntity,
        sf::Text(font, "Score: 0", size));
    scoreText.text.setFillColor(color);
    utils::centerOrigin(scoreText.text);
    scoreText.text.setPosition(position);

    return scoreEntity;
}
```

When a brick is destroyed, the collision system updates the score text directly:
```cpp
auto scoreView = registry.view<HUDTag, ScoreHUDTag, CurrentScore, UIText>();
for (auto scoreEntity : scoreView)
{
    auto& scoreText = scoreView.get<UIText>(scoreEntity);
    auto& scoreCurrentValue = scoreView.get<CurrentScore>(scoreEntity);
    scoreCurrentValue.value += brickScoreValue.value;
    scoreText.text.setString(std::format("Score: {}", scoreCurrentValue.value));
}
```

This shows how UI elements can be fully data-driven through the ECS — the score is just another set of components, updated by a system ([collision system source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/Systems.cpp)).

---

## Systems — Operating on Entities

Systems are free functions in [`Systems.cpp`](../src/ECS/Systems.cpp) grouped into two namespaces.

### CoreSystems

Gameplay logic:

| System | Query (Components) | Behavior |
|--------|-------------------|-----------|
| `handlePlayerInput` | `PlayerTag + Velocity + MovementSpeed + AnimatorComponent + SpriteComponent + Facing` | Reads WASD keys, sets velocity, switches between idle/walk animations. |
| `movementSystem` | `SpriteComponent + Velocity` | Moves sprites by `velocity × deltaTime`. Optionally enforces `ConfineToWindow` bounds. |
| `facingSystem` | `SpriteComponent + Facing + BaseScale` | Flips the sprite horizontally based on facing direction. |
| `renderSystem` | `SpriteComponent` | Draws all sprites. With `showDebug`, draws bounding boxes. |
| `animationSystem` | `SpriteComponent + AnimatorComponent` | Advances sprite texture rect based on the current animation's frame timing. |

### UISystems

UI logic:

| System | Query (Components) | Behavior |
|--------|-------------------|-----------|
| `uiHoverSystem` | `UIBounds` | Adds/removes `UIHover` tag based on mouse position. |
| `uiRenderSystem` | `UIShape / UIText / GUISprite / GUIRedX` | Draws each UI element type with hover-state color changes. |
| `uiClickSystem` | `UIHover + UIAction` | Invokes the action callback on left-click. |
| `uiSettingsChecks` | `GUISprite + UIToggleCond` | Adds/removes `GUIRedX` overlay based on toggle conditions. |

### How Systems Use the Registry

Systems use `registry.view<ComponentA, ComponentB>()` to iterate over all entities that have **all** of the requested components. Example:

```cpp
auto view = registry.view<SpriteComponent, Velocity>();
for (auto entity : view)
{
    auto& sprite = view.get<SpriteComponent>(entity);
    auto& vel    = view.get<Velocity>(entity);
    sprite.sprite.move(vel.value * deltaTime.asSeconds());
}
```

This is the **data-oriented** approach — the system only touches entities that have exactly the data it needs.

### Real-world Example: Collision System

[Breakdown](https://github.com/nantr0nic/breakdown) has a more complex gameplay system — its `collisionSystem()` ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/Systems.cpp)) handles ball-vs-wall, ball-vs-paddle, ball-vs-brick, brick-vs-paddle, and brick-vs-window collisions all in one place:

```cpp
void CoreSystems::collisionSystem(AppContext& context, sf::Time deltaTime)
{
    auto& registry = context.m_Registry;
    auto& window = context.m_MainWindow;
    sf::Vector2f windowSize = { context.m_AppSettings.targetWidth,
                                context.m_AppSettings.targetHeight };

    // Cache paddle bounds (avoids repeated lookups)
    std::vector<sf::FloatRect> paddleBounds;
    auto paddleView = registry->view<Paddle, Velocity>();
    for (auto entity : paddleView)
    {
        auto& paddleComp = paddleView.get<Paddle>(entity);
        paddleBounds.push_back(paddleComp.shape.getGlobalBounds());
    }

    // Check all bricks against window and paddle
    auto brickView = registry->view<Brick>();
    for (auto entity : brickView)
    {
        // ... collision logic ...
        // Use registry->get<BrickHealth>(entity) for multi-hit bricks
        // Use registry->destroy(entity) on brick destruction
    }

    // Check ball against walls, paddle, and remaining bricks
    auto ballView = registry->view<Ball, Velocity>();
    for (auto entity : ballView)
    {
        // ... wall bouncing, paddle deflections, brick destruction ...
        // Update score HUD entity when a brick breaks
    }
}
```

Key patterns visible in this system:
- **Caching** — paddle bounds are collected once before the collision loop begins.
- **`registry->valid(entity)`** — checks if an entity still exists before operating on it.
- **Deferred state transitions** — `stateManager->replaceState(...)` signals game over.
- **Sound triggers** — `CoreSystems::playSound(context, Assets::SoundBuffers::BrickHit)`.

### Adding a System for Your Own Prefab

Continuing the enemy example, here's a simple AI system:

```cpp
// In Systems.hpp
namespace CoreSystems
{
    void enemyAISystem(entt::registry& registry, sf::Time deltaTime);
}

// In Systems.cpp
void CoreSystems::enemyAISystem(entt::registry& registry, sf::Time deltaTime)
{
    auto view = registry.view<EnemyTag, SpriteComponent, Velocity, MovementSpeed>();
    for (auto entity : view)
    {
        auto& velocity = view.get<Velocity>(entity);
        const auto& speed = view.get<MovementSpeed>(entity);

        // Simple AI: move left and right with a sine wave.
        // Use a per-entity component (not a static local) so the phase
        // resets when the entity is recreated on state change.
        auto& phase = registry.get_or_emplace<MovementPhase>(entity);
        phase.time += deltaTime.asSeconds();
        velocity.value.x = std::sin(phase.time) * speed.value;
    }
}
```

You'll also need the corresponding component (add to `Components.hpp`):

```cpp
struct MovementPhase { float time{ 0.0f }; };
```

Then call it from your state's `update()`:

```cpp
void PlayState::update(sf::Time deltaTime)
{
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::enemyAISystem(*m_AppContext.m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext.m_Registry, deltaTime, *m_AppContext.m_MainWindow);
    CoreSystems::animationSystem(*m_AppContext.m_Registry, deltaTime);
}
```

---

## The Separation in Practice

```
EntityFactory::createPlayer()
    Creates an entity with: PlayerTag, Velocity, MovementSpeed,
    SpriteComponent, Facing, BaseScale, ConfineToWindow, AnimatorComponent

Systems that operate on it:
    handlePlayerInput  → reads PlayerTag + writes Velocity + manages animation state
    movementSystem     → reads Velocity + writes SpriteComponent position
    facingSystem       → reads Facing + writes SpriteComponent scale
    animationSystem    → reads AnimatorComponent + writes SpriteComponent texture rect
    renderSystem       → reads SpriteComponent
```

Each system only asks for the components it needs. If the player entity is missing a component, the system simply skips it. This makes it easy to create new entity types by mixing and matching existing components.

---

## States and Systems

States are the bridge between the ECS and the game loop. Each state calls the appropriate systems in its `update()` and `render()`:

| State | Update calls | Render calls |
|-------|-------------|--------------|
| `MenuState` | `UISystems::uiHoverSystem` | `UISystems::uiRenderSystem` |
| `PlayState` | `CoreSystems::handlePlayerInput`, `facingSystem`, `animationSystem`, `movementSystem` | `CoreSystems::renderSystem` |
| `PauseState` | `UISystems::uiHoverSystem` | `UISystems::uiRenderSystem` |
| `SettingsMenuState` | `UISystems::uiHoverSystem`, `uiSettingsChecks` (also updates `m_MusicVolumeText` / `m_SfxVolumeText` strings inline) | `UISystems::uiRenderSystem` |

### Cleanup Pattern: Bulk Tag Destruction

Each state destroys the entities it created on destruction. The template's `PlayState` cleans up by `PlayerTag`. [Breakdown](https://github.com/nantr0nic/breakdown) takes this further with a **group-tag pattern** — it tags every game-renderable entity with `RenderableTag` and every HUD element with `HUDTag`, then cleans up in two bulk calls ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/State.cpp)):

```cpp
PlayState::~PlayState()
{
    auto& registry = *m_AppContext.m_Registry;

    // Destroy all game entities (paddle, ball, bricks, etc.)
    auto gameView = registry.view<RenderableTag>();
    registry.destroy(gameView.begin(), gameView.end());

    // Destroy all HUD entities (score display, etc.)
    auto hudView = registry.view<HUDTag>();
    registry.destroy(hudView.begin(), hudView.end());
}
```

Similarly, UI states use per-state tags:
```cpp
MenuState::~MenuState()
{
    auto view = registry.view<MenuUITag>();
    registry.destroy(view.begin(), view.end());
}
```

This avoids iterating with `.each()` and checking an enum value — the tag alone is enough.

---

## See Also

- [Game Loop](game-loop.md) — how systems are invoked each frame
- [Managers](managers.md) — how the registry is accessed through AppContext
- [Getting Started](getting-started.md) — adding your first entity and system
