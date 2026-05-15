# Getting Started

This guide covers the project structure, how to add new content, and explains the template's animation system.

## Table of Contents

1. [Quick Start](#quick-start)
   1. [Using the GitHub Template](#using-the-github-template)
   2. [Manual Setup](#manual-setup)
2. [Project Structure](#project-structure)
3. [Adding a New State](#adding-a-new-state)
   1. [Declare the state class](#1-declare-the-state-class-in-statehpp)
   2. [Implement it](#2-implement-it-in-statecpp)
   3. [Transition to your new state](#3-transition-to-your-new-state)
4. [Adding a New Entity and Component](#adding-a-new-entity-and-component)
   1. [Define Components](#step-1-define-components)
   2. [Create a Prefab](#step-2-create-a-prefab-in-entityfactory)
   3. [Write a System](#step-3-write-a-system)
   4. [Call the System from a State](#step-4-call-the-system-from-a-state)
5. [Adding New Assets](#adding-new-assets)
6. [How Animation Works](#how-animation-works)
   1. [Core Components](#core-components)
   2. [The Animation System](#the-animation-system)
   3. [How the Player Uses Animation](#how-the-player-uses-animation)
   4. [Animating Your Own Entities](#animating-your-own-entities)
   5. [Sprite Sheet Requirements](#sprite-sheet-requirements)
7. [Config Files](#config-files)
8. [See Also](#see-also)

---

## Quick Start

### Using the GitHub Template

1. Click **"Use this template"** on the [repository page](https://github.com/nantr0nic/sfml3-game-template).
2. Name your project (e.g., `my-new-game`).
3. Wait for the **Template Cleanup** GitHub Action to run — it renames folders, updates `CMakeLists.txt`, and sets the window title in `WindowConfig.toml`.
4. Clone your new repo and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
> Note: the template cleanup action won't remove this `/docs` directory -- this is so you can reference it locally while developing. I also suspect (if you wish) you can use it to prime AI agents for code generation.

### Manual Setup

If you clone without the template feature, you'll need to rename the `sfml3-game-template/` directory and update `CMakeLists.txt` targets yourself.

---

## Project Structure

```
project-root/                        # e.g., my-new-game/
├── CMakeLists.txt                   # Top-level CMake (or use the inner one directly)
└── sfml3-game-template/             # Renamed via template-cleanup to your project name
    ├── CMakeLists.txt               # Build configuration, fetches SFML/EnTT/toml++
    ├── config/
    │   ├── AssetsManifest.toml      # Asset manifest — lists all resource files
    │   ├── WindowConfig.toml        # Window title, dimensions
    │   └── Player.toml              # Player movement speed, scale
    ├── docs/                        # Documentation
    ├── include/
    │   ├── AppContext.hpp           # Central service locator
    │   ├── AppData.hpp              # Runtime game data + settings
    │   ├── Application.hpp          # Application class
    │   ├── AssetKeys.hpp            # Centralized resource string IDs
    │   ├── State.hpp                # Base state + all concrete states
    │   ├── ECS/
    │   │   ├── Components.hpp       # All ECS component structs
    │   │   ├── EntityFactory.hpp    # Prefab factory declarations
    │   │   └── Systems.hpp          # System function declarations
    │   ├── Managers/
    │   │   ├── ConfigManager.hpp
    │   │   ├── GlobalEventManager.hpp
    │   │   ├── ResourceManager.hpp
    │   │   ├── StateManager.hpp
    │   │   └── WindowManager.hpp
    │   └── Utilities/
    │       ├── Logger.hpp           # Async logger
    │       ├── RandomMachine.hpp    # Thread-safe RNG
    │       └── Utils.hpp            # boxView, centerOrigin, sprite padding, etc.
    ├── resources/
    │   ├── fonts/
    │   ├── GUI/
    │   ├── music/
    │   └── sprites/
    └── src/
        ├── Main.cpp                 # Entry point
        ├── Application.cpp          # Game loop
        ├── State.cpp                # All state implementations
        ├── ECS/
        │   ├── EntityFactory.cpp    # Prefab implementations
        │   └── Systems.cpp          # Core + UI system implementations
        ├── Managers/
        │   ├── ConfigManager.cpp
        │   ├── GlobalEventManager.cpp
        │   ├── ResourceManager.cpp
        │   ├── StateManager.cpp
        │   └── WindowManager.cpp
        └── Utilities/
            ├── RandomMachine.cpp
            └── Utils.cpp
```

---

## Adding a New State

States are the top-level screens of your game (menus, gameplay, pause screen, etc.).

### 1. Declare the state class in [`State.hpp`](../include/State.hpp)

```cpp
class GameOverState : public State
{
public:
    explicit GameOverState(AppContext& appContext);
    virtual ~GameOverState() override;

    virtual void update(sf::Time deltaTime) override;
    virtual void render() override;

private:
    void initUI();
    void assignStateEvents();

    std::optional<sf::Text> m_MessageText;
};
```

### 2. Implement it in [`State.cpp`](../src/State.cpp)

```cpp
GameOverState::GameOverState(AppContext& appContext)
    : State(appContext)
{
    initUI();
    assignStateEvents();
    logger::Info("GameOverState initialized.");
}

GameOverState::~GameOverState()
{
    auto& registry = *m_AppContext.m_Registry;
    // Collect entities first, then destroy them in a separate loop.
    // Destroying while iterating a view can invalidate its iterators.
    std::vector<entt::entity> entitiesToRemove;
    auto view = registry.view<UITagID>();
    for (auto [entity, tag] : view.each())
    {
        if (tag.id == UITags::Transition) // or a new UITags value
        {
            entitiesToRemove.push_back(entity);
        }
    }
    for (auto entity : entitiesToRemove)
    {
        registry.destroy(entity);
    }
}

void GameOverState::update(sf::Time deltaTime)
{
    // Call UI systems for hover effects
    UISystems::uiHoverSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
}

void GameOverState::render()
{
    UISystems::uiRenderSystem(*m_AppContext.m_Registry, *m_AppContext.m_MainWindow);
    if (m_MessageText)
        m_AppContext.m_MainWindow->draw(*m_MessageText);
}

void GameOverState::initUI()
{
    // Use EntityFactory::createButton() to create UI
    sf::Vector2f center = getWindowCenter();
    auto* font = m_AppContext.m_ResourceManager->getResource<sf::Font>(Assets::Fonts::MainFont);
    if (font)
    {
        EntityFactory::createButton(m_AppContext, *font, "Play Again", center,
            [this]() {
                auto playState = std::make_unique<PlayState>(m_AppContext);
                m_AppContext.m_StateManager->replaceState(std::move(playState));
            });
    }
}

void GameOverState::assignStateEvents()
{
    m_StateEvents.onMouseButtonPress = [this](const sf::Event::MouseButtonPressed& event) {
        UISystems::uiClickSystem(*m_AppContext.m_Registry, event);
    };
    m_StateEvents.onKeyPress = [this](const sf::Event::KeyPressed& event) {
        if (event.scancode == sf::Keyboard::Scancode::Escape)
            m_AppContext.m_MainWindow->close();
    };
}
```

### 3. Transition to your new state

```cpp
m_AppContext.m_StateManager->replaceState(std::make_unique<GameOverState>(m_AppContext));
```

---

## Adding a New Entity and Component

This follows the **data-oriented** separation described in [ECS](ecs.md#the-ecs-mindset):

- **Component** = data (pure struct)
- **EntityFactory** = assembly (creates entity + attaches components)
- **System** = logic (reads/writes component data)

### Step 1: Define Components

In [`Components.hpp`](../include/ECS/Components.hpp):

```cpp
// Tag — marks an entity as a collectible item
struct CollectibleTag {};

// Data — score value for the item
struct ScoreValue { int points{ 10 }; };

// Per-entity animation phase (accumulated time for bobbing)
struct BobPhase { float time{ 0.0f }; };
```

### Step 2: Create a Prefab in EntityFactory

In [`EntityFactory.hpp`](../include/ECS/EntityFactory.hpp):

```cpp
namespace EntityFactory
{
    entt::entity createCollectible(AppContext& context, sf::Vector2f position, int points);
}
```

In [`EntityFactory.cpp`](../src/ECS/EntityFactory.cpp):

```cpp
entt::entity EntityFactory::createCollectible(
    AppContext& context, sf::Vector2f position, int points)
{
    auto& registry = *context.m_Registry;
    auto* texture = context.m_ResourceManager->getResource<sf::Texture>(
        Assets::Textures::Collectible);  // Add this key to AssetKeys.hpp and the manifest

    if (!texture)
    {
        logger::Error("Couldn't create Collectible — missing texture.");
        return entt::null;
    }

    auto entity = registry.create();

    registry.emplace<CollectibleTag>(entity);
    registry.emplace<ScoreValue>(entity, points);
    registry.emplace<Velocity>(entity);
    registry.emplace<MovementSpeed>(entity, 50.0f);

    auto& spriteComp = registry.emplace<SpriteComponent>(entity, sf::Sprite(*texture));
    spriteComp.sprite.setPosition(position);
    utils::centerOrigin(spriteComp.sprite);

    logger::Info("Collectible created.");
    return entity;
}
```

### Step 3: Write a System

In [`Systems.hpp`](../include/ECS/Systems.hpp):

```cpp
namespace CoreSystems
{
    void collectibleBobSystem(entt::registry& registry, sf::Time deltaTime);
}
```

In [`Systems.cpp`](../src/ECS/Systems.cpp):

```cpp
void CoreSystems::collectibleBobSystem(entt::registry& registry, sf::Time deltaTime)
{
    // This system operates on entities with CollectibleTag + SpriteComponent + BobPhase
    auto view = registry.view<CollectibleTag, SpriteComponent, BobPhase>();
    for (auto entity : view)
    {
        auto& sprite = view.get<SpriteComponent>(entity);
        auto& phase = view.get<BobPhase>(entity);

        // Accumulate time so the sine wave produces smooth oscillation.
        // The move oscillates around the sprite's anchor because
        // sin returns values in [-1, 1], keeping the bob centered.
        phase.time += deltaTime.asSeconds();
        float offset = std::sin(phase.time * 3.0f) * 5.0f;
        sprite.sprite.move({ 0.0f, offset });
    }
}
```

### Step 4: Call the System from a State

```cpp
void PlayState::update(sf::Time deltaTime)
{
    CoreSystems::handlePlayerInput(m_AppContext);
    CoreSystems::collectibleBobSystem(*m_AppContext.m_Registry, deltaTime);
    CoreSystems::movementSystem(*m_AppContext.m_Registry, deltaTime, *m_AppContext.m_MainWindow);
    CoreSystems::animationSystem(*m_AppContext.m_Registry, deltaTime);
}
```

### Usage Example

Spawn a few collectibles with different positions and point values inside `PlayState`'s constructor:

```cpp
PlayState::PlayState(AppContext& context)
    : State(context)
{
    // Create the player as usual
    EntityFactory::createPlayer(context, { 400.0f, 300.0f });

    // Spawn two collectibles at different locations with different scores
    sf::Vector2f center = getWindowCenter();

    auto coin1 = EntityFactory::createCollectible(
        context, { center.x - 100.0f, center.y }, 50);
    auto coin2 = EntityFactory::createCollectible(
        context, { center.x + 100.0f, center.y }, 150);

    // The collectibleBobSystem will now animate both coins each frame
}
```

Clean them up by tag when leaving the state:

```cpp
PlayState::~PlayState()
{
    auto& registry = *m_AppContext.m_Registry;
    auto view = registry.view<CollectibleTag>();
    registry.destroy(view.begin(), view.end());

    auto playerView = registry.view<PlayerTag>();
    registry.destroy(playerView.begin(), playerView.end());
}
```

---

## Adding New Assets

### 1. Add the file to your project directory (e.g., `resources/sprites/`)

### 2. Register it in [`config/AssetsManifest.toml`](../config/AssetsManifest.toml)

```toml
[[textures]]
id = "Collectible"
path = "resources/sprites/coin.png"
```

### 3. Add a string constant in [`AssetKeys.hpp`](../include/AssetKeys.hpp)

```cpp
namespace Assets::Textures
{
    constexpr std::string_view Collectible = "Collectible";
}
```

Then access it via:

```cpp
auto* tex = m_AppContext.m_ResourceManager->getResource<sf::Texture>(Assets::Textures::Collectible);
```

> **Real-world example — Sound effects in [Breakdown](https://github.com/nantr0nic/breakdown):**
> The Breakdown game loads multiple sound buffers (paddle hits, wall hits, brick breaks) via the manifest and centralizes their IDs in [`AssetKeys.hpp`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/include/AssetKeys.hpp):
> ```cpp
> namespace Assets::SoundBuffers
> {
>     constexpr std::string_view BrickHit = "BrickHit";
>     constexpr std::string_view PaddleHit = "PaddleHit";
>     constexpr std::string_view WallHit = "WallHit";
> }
> ```
> A reusable [`playSound()`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/src/ECS/Systems.cpp) function handles loading, playing, and cleanup:
> ```cpp
> void CoreSystems::playSound(AppContext& context, std::string_view soundID)
> {
>     if (context.m_AppSettings.sfxMuted) return;
>
>     // Remove finished sounds
>     context.m_AppData.activeSounds.remove_if([](const sf::Sound& s) {
>         return s.getStatus() == sf::Sound::Status::Stopped;
>     });
>
>     auto* buffer = context.m_ResourceManager->getResource<sf::SoundBuffer>(soundID);
>     if (!buffer) { logger::Warn("Sound '{}' not found", soundID); return; }
>
>     context.m_AppData.activeSounds.emplace_back(*buffer);
>     context.m_AppData.activeSounds.back().setVolume(context.m_AppSettings.sfxVolume);
>     context.m_AppData.activeSounds.back().play();
> }
> ```
> Call it from any system: `CoreSystems::playSound(m_AppContext, Assets::SoundBuffers::PaddleHit);`

---

## How Animation Works

The template uses a sprite-sheet animation system defined in the ECS. This section explains how it works and how to animate your own entities.

### Core Components

**`Animation`** ([`Components.hpp`](../include/ECS/Components.hpp#L36-L56)) defines a single animation clip:

```cpp
struct Animation
{
    int row{ 0 };          // Row on the sprite sheet (0 = idle, 1 = walk, etc.)
    int frames{ 0 };       // Total number of frames in this animation
    sf::Time duration;     // Total duration the animation should take

    sf::Time getTimePerFrame() const
    {
        return (frames > 0) ? duration / static_cast<float>(frames) : sf::Time::Zero;
    }
};
```

**`AnimatorComponent`** ([`Components.hpp`](../include/ECS/Components.hpp#L58-L67)) holds runtime state:

```cpp
struct AnimatorComponent
{
    std::map<std::string, Animation> animations;  // e.g., "idle", "walk"
    std::string currentAnimationName{ "" };
    int currentFrame{ 0 };
    sf::Time elapsedTime{ sf::Time::Zero };
    sf::Vector2i frameSize{ 0, 0 };               // e.g., {32, 32}
};
```

### The Animation System

[`CoreSystems::animationSystem()`](../src/ECS/Systems.cpp#L175-L211) iterates over all entities with both `SpriteComponent` and `AnimatorComponent`:

1. **Looks up** the current animation by name in the animation map.
2. **Accumulates** `deltaTime` into `elapsedTime`.
3. **Advances frames** when `elapsedTime` exceeds `getTimePerFrame()`.
4. **Loops** back to frame 0 when reaching the last frame.
5. **Calculates the texture rect**:
   - `rect.position.x = currentFrame × frameSize.x`
   - `rect.position.y = currentAnim.row × frameSize.y`
6. **Sets** the sprite's texture rect to show the correct frame.

### How the Player Uses Animation

In [`EntityFactory::createPlayer()`](../src/ECS/EntityFactory.cpp#L24-L76), two animations are defined:

```cpp
animator.animations["idle"] = { 0, 4, sf::milliseconds(400) };
animator.animations["walk"] = { 3, 8, sf::milliseconds(800) };
```

Meaning:

| Animation | Sheet Row | Frames | Total Duration | Time Per Frame |
|-----------|-----------|--------|----------------|----------------|
| idle | 0 | 4 | 400 ms | 100 ms |
| walk | 3 | 8 | 800 ms | 100 ms |

In [`CoreSystems::handlePlayerInput()`](../src/ECS/Systems.cpp#L27-L75), the system detects movement and switches between animations:

```cpp
if (velocity.value.x != 0.0f || velocity.value.y != 0.0f)
{
    if (animator.currentAnimationName != "walk")
    {
        animator.currentAnimationName = "walk";
        animator.currentFrame = 0;       // Reset to first frame
        animator.elapsedTime = sf::Time::Zero;  // Reset timer
    }
}
```

This prevents resetting the animation every frame — the switch only happens once when the state changes.

### Animating Your Own Entities

**1.** Add an `AnimatorComponent` to your factory:

```cpp
// In your factory function
auto& animator = registry.emplace<AnimatorComponent>(entity);
animator.currentAnimationName = "idle";
animator.frameSize = { 32, 32 };

// Define animations
animator.animations["idle"] = { 0, 4, sf::milliseconds(600) };
animator.animations["walk"] = { 1, 6, sf::milliseconds(600) };
```

**2.** Ensure the entity also has a `SpriteComponent` (the animation system queries both).

**3.** Switch animations in your own system or input handler:

```cpp
if (shouldWalk)
{
    if (animator.currentAnimationName != "walk")
    {
        animator.currentAnimationName = "walk";
        animator.currentFrame = 0;
        animator.elapsedTime = sf::Time::Zero;
    }
}
```

**4.** Make sure the animation system runs each frame:

```cpp
CoreSystems::animationSystem(*m_AppContext.m_Registry, deltaTime);
```

### Sprite Sheet Requirements

Your sprite sheet should be laid out as a grid where:

- Each **row** is a different animation (row 0 = idle, row 1 = walk, etc.).
- Each **column** is a frame of that animation.
- All frames are the same size (`frameSize`).
- The template uses a spritesheet where each frame is **32×32 pixels** (Brackey's knight sprite), but you can use any size by setting `frameSize` accordingly.

---

## Config Files

### [`config/WindowConfig.toml`](../config/WindowConfig.toml)

```toml
[mainWindow]
Title = "SFML Game Template"
X = 1280
Y = 720
```

### [`config/Player.toml`](../config/Player.toml)

```toml
[player]
movementSpeed = 350.0
scaleFactor = 3.0
```

### Per-Entity Config Files

Beyond the manifest, you can have config files per entity type. [Breakdown](https://github.com/nantr0nic/breakdown) uses separate TOML files for each game object:

**`config/Ball.toml`** ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Ball.toml)) — controls ball radius, speed, color:
```toml
[ball]
ballRadius = 25.0
ballSpeed = 450.0
ballRGB = [255, 255, 255]
```

**`config/Bricks.toml`** ([source](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Bricks.toml)) — per-brick-type values (score, health, color):
```toml
[normal]
scoreValue = 5
healthMax = 1
normalRGB = [0, 125, 255]

[strong]
scoreValue = 10
healthMax = 2
strongRGB = [0, 200, 255]

[strongDamaged]
strongDamagedRGB = [200, 100, 100]
```

### Level Data via Config

Breakdown uses `ConfigManager::getStringArray()` to load level layouts from TOML as ASCII-art grids. Each character maps to a brick type:

[`config/Levels.toml`](https://github.com/nantr0nic/breakdown/blob/main/breakdown/config/Levels.toml):
```toml
totalLevels = 7

[level_1]
brickWidth = 120.0
brickHeight = 40.0
descentSpeed = 18.0
layout = [
    "..N....N..",
    "...N..N...",
    "....NN...."
]

[level_2]
brickWidth = 120.0
brickHeight = 40.0
descentSpeed = 15.0
layout = [
    "N...SS...N",
    ".G......G.",
    "N........N"
]
```

In code, this is loaded and parsed character-by-character to build bricks:
```cpp
std::vector<std::string> layout = context.m_ConfigManager->getStringArray(
    Assets::Configs::Levels, sectionName, "layout");

for (size_t row = 0; row < layout.size(); ++row)
    for (size_t col = 0; col < layout[row].size(); ++col)
        switch (layout[row][col])
        {
            case 'S': /* create strong brick */ break;
            case 'G': /* create gold brick */ break;
            case 'N': /* create normal brick */ break;
        }
```

### [`config/AssetsManifest.toml`](../config/AssetsManifest.toml)

Lists every resource the game uses. Each entry has an `id` (used in code) and a `path` (filesystem path).

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

---

## See Also

- [Game Loop](game-loop.md) — how everything connects at runtime
- [Managers](managers.md) — detailed API reference for all managers
- [ECS](ecs.md) — deep dive into components, factories, and systems
