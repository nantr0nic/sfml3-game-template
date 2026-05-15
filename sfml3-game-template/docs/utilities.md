# Utilities

This document covers the utility classes and functions provided by the template: the asynchronous logger, the random number generator, and the helper functions in `Utils.hpp`.

## Table of Contents

1. [Logger](#logger)
   1. [Log Levels](#log-levels)
   2. [Output Format](#output-format)
   3. [File Logging](#file-logging)
   4. [Log Level Control](#log-level-control)
   5. [Architecture](#architecture)
2. [RandomMachine](#randommachine)
   1. [Construction](#construction)
   2. [Integer Methods](#integer-methods)
   3. [Float Methods](#float-methods)
   4. [Error Handling](#error-handling)
   5. [Usage Example](#usage-example)
3. [Utils](#utils)
   1. [boxView()](#utils-boxview)
   2. [centerOrigin()](#utils-centerorigin)
   3. [getSpritePadding()](#utils-getspritepadding)
   4. [loadColorFromConfig()](#utils-loadcolorfromconfig)
4. [See Also](#see-also)

---

## Logger

**Header:** [`Logger.hpp`](../include/Utilities/Logger.hpp)

A header-only, asynchronous logger built on C++23's `std::print` and `std::source_location`. It logs to the console with ANSI color codes, and can optionally log to timestamped files.

### Log Levels

| Function | Color | Stream | Behavior |
|----------|--------|--------|-----------|
| `logger::Info(message)` | Green | stdout | General information. |
| `logger::Warn(message)` | Yellow | stdout | Something unusual but not fatal. |
| `logger::Error(message)` | Red | stderr | A failure that was handled gracefully. |
| `logger::Fatal(message)` | Red | stderr | **Terminates the program** via `std::abort()`. |

Every call automatically captures the caller's source file, line, and column via `std::source_location::current()`.

```cpp
logger::Info("Application initialized.");
logger::Warn(std::format("Key [{}] not found.", key));
logger::Error(std::format("Failed to load font: {}", filepath));
logger::Fatal("getMainWindow failed! m_MainWindow is null!");
```

### Output Format

```
[[INFO]] Application.cpp(13:5) --> Application initialized.
[[WARNING]] ConfigManager.hpp(42:8) --> Key [foobar] not found.
[[ERROR]] ResourceManager.cpp(31:12) --> Failed to load font: missing.ttf
[[FATAL]] WindowManager.cpp(45:5) --> getMainWindow failed! m_MainWindow is null!
```

### File Logging

File logging is off by default. Enable it in one of two ways:

**Via CMake** (recommended):
```bash
cmake -S . -B build -DLOG_TO_FILE=ON
```

**Via source define** (uncomment in `Logger.hpp`):
```cpp
#define LOG_TO_FILE 1
```

When enabled, log files are written to `logs/` with filenames like `2025-03-28_14-30-00.log`.

### Log Level Control

The current log level can be adjusted at runtime:

```cpp
logger::setLevel(LogLevel::Error);  // Only show errors
logger::forceVerbose();             // Show everything
```

In release builds (`NDEBUG`), the default level is `LogLevel::Info`. Comment/uncomment the relevant `#ifdef NDEBUG` block in `Logger.hpp` to restrict release builds to errors only.

### Architecture

The logger uses a background worker thread (`LogWorker`):

1. Calls to `logger::Info`/`Warn`/`Error` push a `LogEntry` onto a mutex-protected queue.
2. The worker thread pops entries and writes them to the console (and optionally a file).
3. On destruction, the worker flushes and joins.

This means logging calls are **non-blocking** in the hot path — the caller never waits for I/O.

> **Note:** `Fatal` does **not** use the queue. It flushes everything and calls `std::abort()` immediately.

---

## RandomMachine

**Header:** [`RandomMachine.hpp`](../include/Utilities/RandomMachine.hpp)  
**Source:** [`RandomMachine.cpp`](../src/Utilities/RandomMachine.cpp)

A self-contained, thread-safe random number generator. Each instance owns its own `std::random_device` and `std::mt19937` engine.

### Construction

```cpp
utils::RandomMachine rng;  // Seeded from true random device
```

### Integer Methods

| Method | Range | Example |
|--------|-------|---------|
| `getInt(min, max, fallback)` | `[min, max]` inclusive | `rng.getInt(1, 6)` → 1–6 |
| `d2()` | `[1, 2]` | Coin flip |
| `d4()` | `[1, 4]` | |
| `d6()` | `[1, 6]` | |
| `d8()` | `[1, 8]` | |
| `d10()` | `[1, 10]` | |
| `d12()` | `[1, 12]` | |
| `d20()` | `[1, 20]` | |
| `d100()` | `[1, 100]` | |

### Float Methods

| Method | Range | Example |
|--------|-------|---------|
| `getFloat(min, max, fallback)` | `[min, max]` inclusive | `rng.getFloat(0.0f, 1.0f)` |
| `zeroToOne()` | `[0.0f, 1.0f]` | |
| `negOneToOne()` | `[-1.0f, 1.0f]` | |

### Error Handling

If `min > max` is passed, the function logs an error (with source location) and returns the `fallback` value:

```cpp
int result = rng.getInt(10, 1, -1);  // logs error, returns -1
```

### Usage Example

```cpp
utils::RandomMachine rng;

// Random damage between 5 and 15
int damage = rng.getInt(5, 15);

// Random speed multiplier between 0.8 and 1.2
float multiplier = rng.getFloat(0.8f, 1.2f);

// Dice roll
if (rng.d20() >= 15) { /* success */ }
```

---

## Utils

**Header:** [`Utils.hpp`](../include/Utilities/Utils.hpp)  
**Source:** [`Utils.cpp`](../src/Utilities/Utils.cpp)

A collection of free functions and a template helper.

### `utils::boxView()`

Maintains the game's target aspect ratio inside the window by recalculating the SFML viewport. This produces letterboxing (black bars on top/bottom) or pillarboxing (bars on sides) as needed.

**Called automatically** in `Application::processEvents()` on window resize. You generally don't need to call it yourself, but it's available if you need to recalculate the view at other times.

```cpp
sf::View view(sf::FloatRect({0.0f, 0.0f}, {1280.0f, 720.0f}));
utils::boxView(view, actualWindowWidth, actualWindowHeight);
window.setView(view);
```

### `utils::centerOrigin()`

A template function that sets the origin of any SFML drawable to its center. Works with `sf::Sprite`, `sf::Text`, `sf::RectangleShape`, `sf::CircleShape`, etc.

```cpp
utils::centerOrigin(mySprite);
utils::centerOrigin(myText);
utils::centerOrigin(myShape);
```

Internally, it calls `item.setOrigin(item.getLocalBounds().getCenter())`.

### `utils::getSpritePadding()`

Analyses a sprite's texture to find the bounding box of non-transparent pixels. Returns a `SpritePadding` struct:

```cpp
struct SpritePadding
{
    float left, right, top, bottom;
};
```

This is used by `createPlayer()` to calculate `ConfineToWindow` padding — so the invisible transparent edges of a sprite don't trigger wall collision before the visible part reaches the edge.

```cpp
SpritePadding padding = utils::getSpritePadding(spriteComp.sprite);
registry.emplace<ConfineToWindow>(
    playerEntity,
    padding.left * scale, padding.right * scale,
    padding.top * scale, padding.bottom * scale
);
```

> **Performance:** `getSpritePadding()` calls `texture.copyToImage()`, which copies texture data from GPU to RAM. **Do not call this every frame** or inside hot loops. It is intended for one-time use during entity creation.

### `utils::loadColorFromConfig()`

Reads an RGB color array `[r, g, b]` from a TOML config file and returns an `sf::Color`. Values are clamped to 0–255. Returns `sf::Color::Magenta` as a visible error color if anything goes wrong.

```toml
; In some config file
[ui]
buttonColor = [50, 120, 200]
```

```cpp
sf::Color btnColor = utils::loadColorFromConfig(
    configManager, "UIConfig", "ui", "buttonColor");
```

---

## See Also

- [Game Loop](game-loop.md) — where Utils functions are used at runtime
- [ECS](ecs.md) — how `getSpritePadding` integrates with ConfineToWindow
- [Getting Started](getting-started.md) — using RandomMachine in gameplay code
