cmake_minimum_required(VERSION 3.15..4.0)

project(SXICore VERSION 1.0
                DESCRIPTION "Core functionality for Starting XI"
                LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
            src/File.cpp
            src/Timing.cpp
            include/${PROJECT_NAME}/MPL.h
            include/${PROJECT_NAME}/ECS.h
            include/${PROJECT_NAME}/Exception.h
            include/${PROJECT_NAME}/File.h
            include/${PROJECT_NAME}/Timing.h
            include/${PROJECT_NAME}/Types.h)

target_include_directories(${PROJECT_NAME} PRIVATE include/${PROJECT_NAME})