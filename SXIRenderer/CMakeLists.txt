cmake_minimum_required(VERSION 3.15..4.0)

project(SXIRenderer VERSION 1.0
                    DESCRIPTION "Rendering library. Uses Vulkan."
                    LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
            src/Model.cpp
            src/Renderer.cpp
            src/Texture.cpp
            src/Scene.cpp
            src/detail/Context.cpp
            src/detail/Window.cpp
            src/detail/RenderPass.cpp
            src/detail/GraphicsPipeline.cpp
            src/detail/Buffer.cpp
            src/detail/Utils.cpp
            include/${PROJECT_NAME}/Model.h
            include/${PROJECT_NAME}/Renderer.h
            include/${PROJECT_NAME}/Texture.h
            include/${PROJECT_NAME}/Scene.h
            include/${PROJECT_NAME}/detail/Context.h
            include/${PROJECT_NAME}/detail/Window.h
            include/${PROJECT_NAME}/detail/RenderPass.h
            include/${PROJECT_NAME}/detail/GraphicsPipeline.h
            include/${PROJECT_NAME}/detail/Buffer.h
            include/${PROJECT_NAME}/detail/Utils.h)

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