cmake_minimum_required(VERSION 3.15..4.0)

project(SXIRenderer VERSION 1.0
                    DESCRIPTION "Rendering library. Uses Vulkan."
                    LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
            src/Renderer.cpp
            src/detail/Model.cpp
            src/detail/Texture.cpp
            src/detail/Scene.cpp
            src/detail/Context.cpp
            src/detail/Window.cpp
            src/detail/RenderPass.cpp
            src/detail/GraphicsPipeline.cpp
            src/detail/Buffer.cpp
            src/detail/Utils.cpp
            include/${PROJECT_NAME}/Renderer.h
            include/${PROJECT_NAME}/ECSConfig.h
            include/${PROJECT_NAME}/detail/Model.h
            include/${PROJECT_NAME}/detail/Texture.h
            include/${PROJECT_NAME}/detail/Scene.h
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

if (DEFINED SXI_SHADERS_DIR)
    message("SXI_SHADERS_DIR defined as ${SXI_SHADERS_DIR}")
    file(GLOB SHADER_MODULES 
        ${SXI_SHADERS_DIR}/*.vert
        ${SXI_SHADERS_DIR}/*.frag)

    add_custom_target(compile_shaders ALL
        python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile_shaders.py ${SXI_SHADERS_DIR}/ -o ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/generated
        DEPENDS ${SHADER_MODULES}
        COMMENT "Compiling shaders..."
        VERBATIM)
else()
    message("WARNING: Variable SXI_SHADERS_DIR not provided, cannot compile/generate shader code!")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
endif()