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
# STAGE 2: Fetch and configure EnTT and TGUI
# --------------------------------------------------------------------

# Declare the EnTT dependency (header-only)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.15.0
)

### Declare the TGUI dependency
#FetchContent_Declare(
#    tgui
#    GIT_REPOSITORY https://github.com/texus/TGUI.git
#    GIT_TAG        v1.11.0
#)

### Configure TGUI: Must be done BEFORE MakeAvailable
### Set the backend to SFML so TGUI knows which renderer to build for.
#set(TGUI_BACKEND "SFML_GRAPHICS" CACHE STRING "TGUI backend")
#set(TGUI_BUILD_STATIC_LIBS ON CACHE BOOL "Build TGUI as static libraries")
#set(TGUI_CXX_STANDARD 20)

# Make both libraries available to the project
FetchContent_MakeAvailable(entt) #tgui)

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
    #TGUI::TGUI
)

### MSVC-specific settings ###
if(MSVC)
    target_compile_options(SFML-CMake-Prac PRIVATE "/std:c++latest" /Zi /GL /guard:cf /Gy)
    target_link_options(SFML-CMake-Prac PRIVATE /LTCG /GUARD:CF /OPT:REF /OPT:ICF)
    set_property(TARGET SFML-CMake-Prac PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()