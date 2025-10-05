cmake_minimum_required(VERSION 3.15..4.0)

project(SXIPathfinding VERSION 1.0
                       DESCRIPTION "Pathfinding and map construction library."
                       LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
            src/CDT.cpp
            src/Map.cpp
            src/MapShape.cpp
            src/PolyAnya.cpp
            src/RStarTree.cpp
            include/${PROJECT_NAME}/CDT.h
            include/${PROJECT_NAME}/Map.h
            include/${PROJECT_NAME}/MapShape.h
            include/${PROJECT_NAME}/PolyAnya.h
            include/${PROJECT_NAME}/RStarTree.h)

target_link_libraries(${PROJECT_NAME} SXIMath)
target_include_directories(${PROJECT_NAME} PRIVATE include/${PROJECT_NAME} ../SXIMath/include)