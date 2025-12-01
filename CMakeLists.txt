# Trying out a single top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.28)
project("SFML-CMake-Prac" LANGUAGES CXX)

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

add_executable(SFML-CMake-Prac
    "SFML-CMake-Prac/src/Main.cpp"
    "SFML-CMake-Prac/src/Application.cpp"
    "SFML-CMake-Prac/src/State.cpp"
    "SFML-CMake-Prac/src/Managers/WindowManager.cpp"
    "SFML-CMake-Prac/src/Managers/StateManager.cpp"
    "SFML-CMake-Prac/src/Managers/GlobalEventManager.cpp"
    "SFML-CMake-Prac/src/Managers/ConfigManager.cpp"
    "SFML-CMake-Prac/src/Managers/ResourceManager.cpp"
    "SFML-CMake-Prac/src/ECS/EntityFactory.cpp"
    "SFML-CMake-Prac/src/ECS/Systems.cpp"
    "SFML-CMake-Prac/src/Utilities/RandomMachine.cpp"
    "SFML-CMake-Prac/src/Utilities/Utils.cpp"
)

# This runs every time the SFML-CMake-Prac target is built.
# It will copy the resources to the build directory
add_custom_command(
    TARGET SFML-CMake-Prac
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/SFML-CMake-Prac/resources"
            "$<TARGET_FILE_DIR:SFML-CMake-Prac>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/SFML-CMake-Prac/config"
            "$<TARGET_FILE_DIR:SFML-CMake-Prac>/config"
    COMMENT "Copying resources and config to build directory"
)

# -------------------- #
#  compile definitions #
# -------------------- #

target_compile_definitions(SFML-CMake-Prac PRIVATE TOML_HEADER_ONLY=0 TOML_EXCEPTIONS=0)

# ------------------ #
# Configure include  #
# ------------------ #

# manually setting entt because clangd was being weird >:/
target_include_directories(SFML-CMake-Prac PRIVATE
    "${entt_SOURCE_DIR}/include"
    "SFML-CMake-Prac/include"
)

# ------------------ #
#   Link libraries   #
# ------------------ #

target_link_libraries(SFML-CMake-Prac PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::Audio
    EnTT::EnTT
    tomlplusplus::tomlplusplus
)

# ---------------------- #
#   Precompile Headers   #
# ---------------------- #
target_precompile_headers(SFML-CMake-Prac PRIVATE 
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
    target_compile_options(SFML-CMake-Prac PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(SFML-CMake-Prac PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET SFML-CMake-Prac PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()