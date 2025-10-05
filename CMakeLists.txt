cmake_minimum_required(VERSION 3.15..4.0)

set (CMAKE_CXX_STANDARD 20)

project(StartingXI VERSION 1.0
                   DESCRIPTION "C++ game engine running on Vulkan"
                   LANGUAGES C CXX)

include(FetchContent)

# Find dependencies or git clone them
# Vulkan
find_package(Vulkan REQUIRED)

# SDL3
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        a8589a84226a6202831a3d49ff4edda4acab9acd
)
FetchContent_MakeAvailable(SDL3)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        0af55ccecd98d4e5a8d1fad7de25ba429d60e863
)
FetchContent_MakeAvailable(glm)

# STB
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
)
FetchContent_MakeAvailable(stb)

# tinyobjloader
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v2.0.0rc13
)
FetchContent_MakeAvailable(
    SDL3
    glm
    stb
    tinyobjloader)

add_subdirectory(SXICore)
add_subdirectory(SXIMath)
add_subdirectory(SXIPathfinding)
add_subdirectory(SXIRenderer)
add_subdirectory(MysteriousGame)