# Trying out a single top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.28)
project("sfml3-game-template" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------ #
# Fetch dependencies #
# ------------------ #
include(FetchContent)

# ----- SFML ----- #
FetchContent_Declare(
    sfml_dependency
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.0.2
)

# Configure SFML options
set(SFML_STATIC_LIBRARIES ON CACHE BOOL "Build SFML as static libraries")
set(SFML_USE_SYSTEM_DEPS OFF CACHE BOOL "Use SFML's internal dependencies")
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "Build the Graphics module")
set(SFML_BUILD_WINDOW ON CACHE BOOL "Build the Window module")
set(SFML_BUILD_AUDIO ON CACHE BOOL "Build the Audio module")
set(SFML_BUILD_NETWORK OFF CACHE BOOL "Do not build the Network module")

# ----- EnTT ----- #
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.15.0
)

# ----- toml++ ----- #
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(sfml_dependency entt tomlplusplus)

# ------------------ #
#  Find Packages     #
# ------------------ #
find_package(Threads REQUIRED)

# ------------------ #
# Configure project  #
# ------------------ #

add_executable(sfml3-game-template
    "sfml3-game-template/src/Main.cpp"
    "sfml3-game-template/src/Application.cpp"
    "sfml3-game-template/src/State.cpp"
    "sfml3-game-template/src/Managers/WindowManager.cpp"
    "sfml3-game-template/src/Managers/StateManager.cpp"
    "sfml3-game-template/src/Managers/GlobalEventManager.cpp"
    "sfml3-game-template/src/Managers/ConfigManager.cpp"
    "sfml3-game-template/src/Managers/ResourceManager.cpp"
    "sfml3-game-template/src/ECS/EntityFactory.cpp"
    "sfml3-game-template/src/ECS/Systems.cpp"
    "sfml3-game-template/src/Utilities/RandomMachine.cpp"
    "sfml3-game-template/src/Utilities/Utils.cpp"
)

# This will copy the resources to the build directory
#!!! This will copy the resources every time the project is built!
#!!! For now with the limited resources being used, this is OK but at a certain
#!!! point it will be necessary to optimize this process. I looked into it
#!!! and it looks like more than I want to deal with right now. So be aware!
#!!! If your project is large and you have a lot of resources, consider using a
#!!! more efficient method for copying resources.
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/sfml3-game-template/resources"
            "$<TARGET_FILE_DIR:sfml3-game-template>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_LIST_DIR}/sfml3-game-template/config"
            "$<TARGET_FILE_DIR:sfml3-game-template>/config"
    COMMENT "Syncing resources and config files..."
)

add_dependencies(sfml3-game-template CopyAssets)

# This is to copy compile_commands.json to out directory for clangd
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(
        copy-compile-commands ALL
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${CMAKE_BINARY_DIR}/compile_commands.json"
                "${CMAKE_SOURCE_DIR}/out/compile_commands.json"
        DEPENDS "${CMAKE_BINARY_DIR}/compile_commands.json"
        COMMENT "Copying compile_commands.json to out directory"
    )
endif()

# -------------------- #
#       options        #
# -------------------- #
option(LOG_TO_FILE "Enable logging to file" OFF)

# -------------------- #
#  compile definitions #
# -------------------- #
target_compile_definitions(sfml3-game-template PRIVATE TOML_EXCEPTIONS=0)

if(LOG_TO_FILE)
    target_compile_definitions(sfml3-game-template PRIVATE LOG_TO_FILE)
endif()

# ------------------ #
# Configure include  #
# ------------------ #

# manually setting entt because clangd was being weird >:/
target_include_directories(sfml3-game-template PRIVATE
    "${entt_SOURCE_DIR}/include"
    "sfml3-game-template/include"
)

# ------------------ #
#   Link libraries   #
# ------------------ #

target_link_libraries(sfml3-game-template PRIVATE
    SFML::Audio
    SFML::Graphics
    SFML::Window
    EnTT::EnTT
    tomlplusplus::tomlplusplus
    Threads::Threads
)

# ---------------------- #
#   Precompile Headers   #
# ---------------------- #
target_precompile_headers(sfml3-game-template PRIVATE
    # STL
    <map>
    <vector>
    <list>
    <string>
    <string_view>
    <format>
    <print>
    <source_location>
    <memory>
    <optional>
    <functional>
    <random>
    <utility>
    <limits>
    <algorithm>
    <atomic>
    <queue>
    <mutex>
    <thread>
    <condition_variable>
    <chrono>
    <type_traits>

    # C STD
    <cstdint>
    <cstdio>
    <cstdlib>
    <cmath>

    # Third party libraries
    <SFML/Graphics.hpp>
    <SFML/Window.hpp>
    <SFML/Audio.hpp>
    <SFML/System.hpp>
    <entt/entt.hpp>
    <toml++/toml.hpp>
)

# ------------------ #
#   MSVC  Settings   #
# ------------------ #

# This should hopefully mitigate any false 'trojan' flags by Windows Defender
if(MSVC)
    target_compile_options(sfml3-game-template PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(sfml3-game-template PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET sfml3-game-template PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()

# ----- Windows Settings ----- #
# Default to ON (Show console)
# Turn this OFF to disable console window on Windows (preferred for Release builds)
option(SHOW_CONSOLE "Show console window on Windows builds" ON)

if(WIN32)
    if(SHOW_CONSOLE)
        # 1. CONSOLE MODE
        message(STATUS "Windows Build: Console Window ENABLED")
    else()
        # 2. GUI MODE
        set_target_properties(sfml3-game-template PROPERTIES WIN32_EXECUTABLE ON)

        # Link SFML::Main to provide the 'WinMain' entry point required by GUI apps
        target_link_libraries(sfml3-game-template PRIVATE SFML::Main)

        message(STATUS "Windows Build: Console Window DISABLED (GUI Mode)")
    endif()
endif()
