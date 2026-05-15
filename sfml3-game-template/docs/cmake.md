# CMake Build System

This document explains how the project's CMake build is structured, how to build on Linux and Windows, and how to configure build options.

---

## Table of Contents

1. [Build Structure](#build-structure)
2. [Requirements](#requirements)
3. [Building on Linux](#building-on-linux)
4. [Building on Windows](#building-on-windows)
5. [CMake Presets](#cmake-presets)
   1. [Linux Presets](#linux-presets)
   2. [Windows Presets](#windows-presets)
6. [Dependencies (FetchContent)](#dependencies-fetchcontent)
   1. [SFML](#sfml)
   2. [EnTT](#entt)
   3. [toml++](#toml)
7. [Project Configuration](#project-configuration)
   1. [Resource Copying (CopyAssets)](#resource-copying-copyassets)
   2. [Precompile Headers](#precompile-headers)
   3. [Compile Definitions](#compile-definitions)
   4. [Logging to File](#logging-to-file)
8. [Platform-Specific Settings](#platform-specific-settings)
   1. [MSVC (Windows)](#msvc-windows)
   2. [Console vs GUI Mode (Windows)](#console-vs-gui-mode-windows)
   3. [macOS Notes](#macos-notes)
9. [Packaging (Breakdown Example)](#packaging-breakdown-example)
10. [See Also](#see-also)

---

## Build Structure

The project uses a **single `CMakeLists.txt`** at the repository root. There is no nested `CMakeLists.txt` inside the source directory. All source files are referenced relative to the root:

```
project-root/
├── CMakeLists.txt                  # Single build file
├── CMakePresets.json               # Build presets (Linux / Windows)
└── sfml3-game-template/
    ├── src/                        # Source files
    ├── include/                    # Headers
    ├── config/                     # TOML configuration files
    └── resources/                  # Assets (fonts, textures, sounds, music)
```

The executable target is named `sfml3-game-template` (the template itself) or your project name (when using the template-cleanup GitHub Action).

---

## Requirements

| Requirement | Version / Detail |
|-------------|------------------|
| **CMake** | ≥ 3.28 |
| **C++ Standard** | C++23 (required, not optional) |
| **Compiler** | GCC 13+, Clang 16+, MSVC 19.35+ |
| **Generator** | Ninja (recommended), or Visual Studio / Makefiles |
| **Dependencies** | Fetched automatically via `FetchContent` |

No pre-installed dependencies are required — CMake downloads SFML 3.1.0, EnTT 3.16.0, and toml++ 3.4.0 at configure time.

> **Note:** macOS is **not** reliably supported by either CMake or SFML for this template. A `macos-debug` preset exists in `CMakePresets.json` but is untested. Linux and Windows are the supported platforms.

---

## Building on Linux

### With Ninja (recommended)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

### Without Ninja

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The first build will take a while because CMake downloads and compiles SFML, EnTT, and toml++ from source. Subsequent builds will be incremental.

To enable file logging:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DLOG_TO_FILE=ON
cmake --build build
```

### Using CMake Presets

```bash
cmake --preset linux-release
cmake --build out/build/linux-release
```

---

## Building on Windows

### With Ninja and CMake Presets (recommended)

```bash
cmake --preset x64-release
cmake --build out/build/x64-release
```

### With Ninja manually

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build
```

### With Visual Studio

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Console vs GUI Mode

By default, Windows builds produce a **console application** (you see a terminal window alongside the game window). For a release build you may want to hide the console:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSHOW_CONSOLE=OFF
```

In GUI mode, the binary links `SFML::Main` to provide the `WinMain` entry point. The game continues to log to file if `LOG_TO_FILE=ON` is set.

---

## CMake Presets

[`CMakePresets.json`](../CMakePresets.json) defines preconfigured build profiles. Using `cmake --preset <name>` sets the generator, compiler, build type, and output directory all at once.

### Linux Presets

| Preset | Build Type | Compiler | Output Directory |
|--------|-----------|----------|------------------|
| `linux-debug` | Debug | Clang | `out/build/linux-debug` |
| `linux-release` | Release | Clang | `out/build/linux-release` |

The Linux presets use the **Ninja** generator and set `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` to Clang.

### Windows Presets

| Preset | Build Type | Arch | Output Directory |
|--------|-----------|------|------------------|
| `x64-debug` | Debug | x64 | `out/build/x64-debug` |
| `x64-release` | Release | x64 | `out/build/x64-release` |
| `x86-debug` | Debug | x86 | `out/build/x86-debug` |
| `x86-release` | Release | x86 | `out/build/x86-release` |

All Windows presets inherit from a hidden `windows-base` preset that sets the Ninja generator and MSVC compilers (`cl.exe`). The architecture is set via CMake's `-A` flag (external strategy). Only install the **x64** preset on 64-bit systems.

> **Note:** The `macos-debug` preset exists for reference but is **not tested or supported**.

---

## Dependencies (FetchContent)

All three dependencies are fetched via `FetchContent` at configure time. This means the first CMake configure downloads them from GitHub and caches them in the build directory.

### SFML

- **Repository:** [SFML/SFML](https://github.com/SFML/SFML)
- **Version:** 3.1.0
- **Tag:** `3.1.0`

SFML is configured with several CMake cache variables:

| Variable | Value | Purpose |
|----------|-------|---------|
| `SFML_STATIC_LIBRARIES` | `ON` | Build SFML as static libraries (no DLLs to ship). |
| `SFML_USE_SYSTEM_DEPS` | `OFF` | Use SFML's bundled dependency sources. |
| `SFML_BUILD_GRAPHICS` | `ON` | Include the Graphics module (sprites, text, shapes). |
| `SFML_BUILD_WINDOW` | `ON` | Include the Window module (windowing, events). |
| `SFML_BUILD_AUDIO` | `ON` | Include the Audio module (sound, music). |
| `SFML_BUILD_NETWORK` | **`OFF`** | **Network module is disabled by default.** |

**To enable the Network module**, set it to `ON` in `CMakeLists.txt`:

```cmake
set(SFML_BUILD_NETWORK ON CACHE BOOL "Build the Network module")
```

Then link `SFML::Network` in `target_link_libraries`.

### EnTT

- **Repository:** [skypjack/entt](https://github.com/skypjack/entt)
- **Version:** 3.16.0
- **Tag:** `v3.16.0`

EnTT is a header-only library — no compilation needed. The include directory is set manually in `target_include_directories` to work around clangd issues.

### toml++

- **Repository:** [marzer/tomlplusplus](https://github.com/marzer/tomlplusplus)
- **Version:** 3.4.0
- **Tag:** `v3.4.0`

Parses TOML configuration files at runtime. The definition `TOML_EXCEPTIONS=0` is set project-wide so that toml++ returns error objects instead of throwing exceptions.

---

## Project Configuration

### Resource Copying (CopyAssets)

A custom CMake target called `CopyAssets` copies the `resources/` and `config/` directories into the build output directory every time the project is built:

```cmake
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/sfml3-game-template/resources"
            "$<TARGET_FILE_DIR:sfml3-game-template>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/sfml3-game-template/config"
            "$<TARGET_FILE_DIR:sfml3-game-template>/config"
)
```

> **Performance note:** This copies all resources on **every build**, not just when files change. For small projects this is fine, but if your resource directory grows large, consider replacing this with a more efficient sync mechanism.

### Precompile Headers

`target_precompile_headers` precompiles commonly used headers (STL, SFML, EnTT, toml++) to speed up compilation. If you add new widely-used headers, add them to this list.

### Compile Definitions

| Definition | Purpose |
|------------|---------|
| `TOML_EXCEPTIONS=0` | Disables exceptions in toml++; errors are returned as `toml::parse_result` instead. |

### Logging to File

The `LOG_TO_FILE` option is defined as a CMake option and is `OFF` by default:

```cmake
option(LOG_TO_FILE "Enable logging to file" OFF)
```

Enable it at configure time:

```bash
cmake -S . -B build -DLOG_TO_FILE=ON
```

When `ON`, the preprocessor define `LOG_TO_FILE` is passed to the compiler, which activates the file-logging code path in [`Logger.hpp`](../include/Utilities/Logger.hpp).

---

## Platform-Specific Settings

### MSVC (Windows)

When building with MSVC, the following compiler and linker options are applied:

```cmake
if(MSVC)
    target_compile_options(sfml3-game-template PRIVATE
        "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(sfml3-game-template PRIVATE
        /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET sfml3-game-template PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
```

These enable:
- **`/std:c++latest`** — Latest C++ standard support.
- **`/Zi`** / **`/GL`** — Program database debug info and whole-program optimisation.
- **`/guard:cf`** / **`/GUARD:CF`** — Control Flow Guard (security mitigation against ROP exploits).
- **`/LTCG`** — Link-Time Code Generation.
- **`MultiThreadedDLL`** — Dynamic MSVC runtime (debug suffix for Debug builds).

> These settings also help mitigate false-positive "trojan" detections by Windows Defender on release builds.

### Console vs GUI Mode (Windows)

The `SHOW_CONSOLE` option controls whether the application runs as a console or GUI executable on Windows:

- **`SHOW_CONSOLE=ON`** (default) — Console window appears alongside the game window. Useful for development to see log output.
- **`SHOW_CONSOLE=OFF`** — No console window. The game runs as a pure Windows GUI application. `SFML::Main` is linked to provide the `WinMain` entry point.

Set it at configure time:

```bash
cmake -S . -B build -DSHOW_CONSOLE=OFF
```

### macOS Notes

A `macos-debug` preset exists in `CMakePresets.json` for reference but is **untested**. SFML and this template target Linux and Windows as primary platforms.

---

## Packaging (Breakdown Example)

[Breakdown](https://github.com/nantr0nic/breakdown) (the example game built with this template) adds packaging via CPack in its `CMakeLists.txt`:

```cmake
set(CPACK_PACKAGE_NAME "breakdown")
set(CPACK_PACKAGE_VERSION "1.0.1")
set(CPACK_GENERATOR "TGZ")         # Linux
# set(CPACK_GENERATOR "ZIP")       # Windows
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

include(CPack)
```

Installation rules copy the executable, resources, and config into the package:

```cmake
install(TARGETS breakdown DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/resources" DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/breakdown/config" DESTINATION .)
```

To create a distributable archive:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
cpack
```

This produces something like `breakdown-1.0.1-Linux.tar.gz`.

---

## See Also

- [Getting Started](getting-started.md) — project structure and adding content
- [Game Loop](game-loop.md) — how the built application runs
- [Managers](managers.md) — how managers are initialized
