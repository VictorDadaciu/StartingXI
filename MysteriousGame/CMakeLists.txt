cmake_minimum_required(VERSION 3.15..4.0)

project(MysteriousGame VERSION 1.0
                       LANGUAGES C CXX)

add_executable(${PROJECT_NAME} 
    src/MysteriousGame.cpp
    include/ECSSettings.h)
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
    include
    ../SXIMath/include
    ../SXICore/include
    ../SXIPathfinding/include
    ../SXIRenderer/include)

file(GLOB SHADER_MODULES 
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag)

add_custom_target(compile_shaders ALL
    python3 ${CMAKE_SOURCE_DIR}/scripts/compile_shaders.py ${CMAKE_CURRENT_SOURCE_DIR}/shaders -o ${CMAKE_CURRENT_SOURCE_DIR}/shaders/generated
    DEPENDS ${SHADER_MODULES}
    COMMENT "Compiling shaders..."
    VERBATIM)