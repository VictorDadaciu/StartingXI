cmake_minimum_required(VERSION 3.15..4.0)

project(SXIMath VERSION 1.0
                DESCRIPTION "Math utilities for Starting XI. Uses glm."
                LANGUAGES C CXX)

find_package(glm REQUIRED)
add_library(${PROJECT_NAME} STATIC
            src/Line.cpp
            src/AABB.cpp
            include/${PROJECT_NAME}/Line.h
            include/${PROJECT_NAME}/AABB.h
            include/${PROJECT_NAME}/Vec.h
            include/${PROJECT_NAME}/Mat.h)

target_link_libraries(${PROJECT_NAME} glm::glm)
target_include_directories(${PROJECT_NAME} PRIVATE include/${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC glm)