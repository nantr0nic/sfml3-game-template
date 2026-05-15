# SFML3 Game Template — Documentation

This folder contains the reference documentation for the SFML3 Game Template. It covers the application architecture, the game loop, the manager classes, the ECS system, the provided utilities, and recommended practices for extending the template.

---

## Document Index

| Document | Description |
|----------|-------------|
| [Getting Started](getting-started.md) | Project structure, adding states/entities/assets, animation system guide, config file reference. Start here. |
| [Game Loop](game-loop.md) | How the application boots, the per-frame loop (process → update → render), event merging, and delta time flow. |
| [Managers](managers.md) | AppContext service locator, StateManager, WindowManager, ConfigManager, ResourceManager, and GlobalEventManager. |
| [ECS](ecs.md) | EnTT integration, component reference, EntityFactory prefabs, systems (Core + UI), common EnTT call patterns. |
| [Utilities](utilities.md) | Logger (async, colored, file output), RandomMachine (dice, floats, error handling), Utils (boxView, centerOrigin, sprite padding, color loading). |
| [CMake Build System](cmake.md) | Build requirements, Linux/Windows build steps, CMake presets, SFML flags (Network module), logging to file, platform settings, packaging. |
| [Recommended Practices](recommended-practices.md) | Performance tips, AppContext signature conventions, codebase idioms (deferred states, cleanup patterns, animation guards, ECS HUD, sound playback, per-entity configs). |

---

## Quick Reference — Where to Find Code

| What you need | Look in |
|---------------|---------|
| Entry point | [`src/Main.cpp`](../src/Main.cpp) |
| Game loop | [`src/Application.cpp`](../src/Application.cpp) |
| States (Menu, Play, Pause, etc.) | [`src/State.cpp`](../src/State.cpp) + [`include/State.hpp`](../include/State.hpp) |
| ECS components | [`include/ECS/Components.hpp`](../include/ECS/Components.hpp) |
| ECS systems | [`src/ECS/Systems.cpp`](../src/ECS/Systems.cpp) + [`include/ECS/Systems.hpp`](../include/ECS/Systems.hpp) |
| Entity factories (prefabs) | [`src/ECS/EntityFactory.cpp`](../src/ECS/EntityFactory.cpp) + [`include/ECS/EntityFactory.hpp`](../include/ECS/EntityFactory.hpp) |
| Managers implementations | [`src/Managers/`](../src/Managers/) |
| Managers declarations | [`include/Managers/`](../include/Managers/) |
| Utilities | [`src/Utilities/`](../src/Utilities/) + [`include/Utilities/`](../include/Utilities/) |
| Config files | [`config/`](../config/) |
| Resource keys | [`include/AssetKeys.hpp`](../include/AssetKeys.hpp) |
| AppContext (service locator) | [`include/AppContext.hpp`](../include/AppContext.hpp) |

---

## Example Game

For a complete working example built with this template, see **[Breakdown](https://github.com/nantr0nic/breakdown)** — a breakout game where bricks descend. The documentation references Breakdown's source code throughout to provide examples.
