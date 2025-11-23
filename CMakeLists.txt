# Trying out a single top-level CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project("SFML-CMake-Prac" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# For IDEs and language servers like clangd to understand the project structure
# (e.g., include paths), we need to generate a compile_commands.json file.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --------------------------------------------------------------------
# STAGE 1: Fetch and configure SFML using FetchContent
# --------------------------------------------------------------------
include(FetchContent)

# Declare the SFML dependency
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

# This command downloads, configures, and adds the SFML targets to our build.
FetchContent_MakeAvailable(sfml_dependency)

# --------------------------------------------------------------------
# STAGE 2: Fetch and configure EnTT, toml11, and magic_enum
# --------------------------------------------------------------------

# Declare the EnTT dependency (header-only)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.15.0
)

# Declare the toml11 dependency (header-only)
FetchContent_Declare(
    toml11
    GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
    GIT_TAG        v4.4.0
    SYSTEM        # Setting as SYSTEM to suppress deprecated literal operator warning
)

# Declare the magic_enum dependency (header-only)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        v0.9.7
)

FetchContent_MakeAvailable(entt toml11 magic_enum) 

# --------------------------------------------------------------------
# STAGE 3: Configure your application
# --------------------------------------------------------------------

# Define a variable for our project's source code subdirectory.
# This makes it easier to reference files within it.
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/SFML-CMake-Prac")

add_executable(SFML-CMake-Prac
    "${PROJECT_SOURCE_DIR}/src/Main.cpp"
    "${PROJECT_SOURCE_DIR}/src/Application.cpp"
    "${PROJECT_SOURCE_DIR}/src/WindowManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/StateManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/State.cpp"
    "${PROJECT_SOURCE_DIR}/src/GlobalEventManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/ECS/EntityFactory.cpp"
    "${PROJECT_SOURCE_DIR}/src/ECS/Systems.cpp"
    "${PROJECT_SOURCE_DIR}/src/RandomMachine.cpp"
    "${PROJECT_SOURCE_DIR}/src/ConfigManager.cpp"
    "${PROJECT_SOURCE_DIR}/src/Utils.cpp"

)

# Copy resources during configuration time.
# Note: Changes to resources will require re-running CMake configuration.
#file(COPY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "${CMAKE_BINARY_DIR}/resources")
#file(COPY "${PROJECT_SOURCE_DIR}/config/" DESTINATION "${CMAKE_BINARY_DIR}/config")

### This is an alternative way to include the resources at build time,
### it does NOT require re-running CMake when resources change.
### However, it may slow down build times slightly.
# This runs every time the SFML-CMake-Prac target is built.
add_custom_command(
    TARGET SFML-CMake-Prac
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/resources"
            "$<TARGET_FILE_DIR:SFML-CMake-Prac>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/config"
            "$<TARGET_FILE_DIR:SFML-CMake-Prac>/config"
    COMMENT "Copying resources and config to build directory"
)

# The ${entt_SOURCE_DIR} variable is automatically set by FetchContent.
target_include_directories(SFML-CMake-Prac PRIVATE
    "${entt_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/include"
)

# Link to all the required libraries that FetchContent made available
target_link_libraries(SFML-CMake-Prac PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::Audio
    EnTT::EnTT
    toml11::toml11
    magic_enum::magic_enum
)

### MSVC-specific settings ###
if(MSVC)
    target_compile_options(SFML-CMake-Prac PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(SFML-CMake-Prac PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET SFML-CMake-Prac PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()