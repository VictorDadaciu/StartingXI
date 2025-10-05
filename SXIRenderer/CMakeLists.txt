cmake_minimum_required(VERSION 3.15..4.0)

project(SXIRenderer VERSION 1.0
                    DESCRIPTION "Rendering library. Uses Vulkan."
                    LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
            src/Model.cpp
            src/Renderer.cpp
            src/Texture.cpp
            src/Window.cpp
            include/${PROJECT_NAME}/Model.h
            include/${PROJECT_NAME}/Renderer.h
            include/${PROJECT_NAME}/Texture.h
            include/${PROJECT_NAME}/Window.h)

target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
    glm::glm
    SDL3::SDL3
    SXIMath
    SXICore)
target_include_directories(${PROJECT_NAME} PRIVATE
    Vulkan
    SDL3
    glm
    ${FETCHCONTENT_BASE_DIR}/stb-src
    ${FETCHCONTENT_BASE_DIR}/tinyobjloader-src
    include/${PROJECT_NAME}
    ../SXIMath/include
    ../SXICore/include)