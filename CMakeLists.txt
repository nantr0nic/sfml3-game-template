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

# This runs every time the sfml3-game-template target is built.
# It will copy the resources to the build directory
add_custom_command(
    TARGET sfml3-game-template
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/sfml3-game-template/resources"
            "$<TARGET_FILE_DIR:sfml3-game-template>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/sfml3-game-template/config"
            "$<TARGET_FILE_DIR:sfml3-game-template>/config"
    COMMENT "Copying resources and config to build directory"
)

# -------------------- #
#  compile definitions #
# -------------------- #

target_compile_definitions(sfml3-game-template PRIVATE TOML_EXCEPTIONS=0)

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
    SFML::Graphics
    SFML::Window
    SFML::Audio
    EnTT::EnTT
    tomlplusplus::tomlplusplus
)

# ---------------------- #
#   Precompile Headers   #
# ---------------------- #
target_precompile_headers(sfml3-game-template PRIVATE 
    # STL
    <vector>
    <string>
    <string_view>
    <map>
    <memory>
    <optional>
    <format>
    <print>
    <functional>
    <source_location>

    # Third party libraries
    <SFML/Graphics.hpp>
    <SFML/Window.hpp>
    <SFML/Audio.hpp>
    <entt/entt.hpp>
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