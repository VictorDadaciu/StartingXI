cmake_minimum_required(VERSION 3.15..4.0)

project(MysteriousGame VERSION 1.0
                       LANGUAGES C CXX)

add_executable(${PROJECT_NAME} src/MysteriousGame.cpp)
target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
    glm::glm
    SDL3::SDL3
    SXIMath
    SXICore
    SXIPathfinding
    SXIRenderer)
target_include_directories(${PROJECT_NAME} PRIVATE
    Vulkan
    SDL3
    glm
    ../SXIMath/include
    ../SXICore/include
    ../SXIPathfinding/include
    ../SXIRenderer/include)