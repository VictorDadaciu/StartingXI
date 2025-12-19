cmake_minimum_required(VERSION 3.15..4.0)

project(SXICore VERSION 1.0
                DESCRIPTION "Core functionality for Starting XI"
                LANGUAGES C CXX)

add_library(${PROJECT_NAME} STATIC
    src/logger/Logger.cpp
    src/JobScheduler.cpp
    src/File.cpp
    src/Timing.cpp
    include/${PROJECT_NAME}/MPL/Contains.h
    include/${PROJECT_NAME}/MPL/Count.h
    include/${PROJECT_NAME}/MPL/Filter.h
    include/${PROJECT_NAME}/MPL/IndexOf.h
    include/${PROJECT_NAME}/MPL/IsSame.h
    include/${PROJECT_NAME}/MPL/Macros.h
    include/${PROJECT_NAME}/MPL/Map.h
    include/${PROJECT_NAME}/MPL/PushFront.h
    include/${PROJECT_NAME}/MPL/Repeat.h
    include/${PROJECT_NAME}/MPL/Supersets.h
    include/${PROJECT_NAME}/MPL/Tuple.h
    include/${PROJECT_NAME}/MPL/Type.h
    include/${PROJECT_NAME}/MPL/TypeList.h
    include/${PROJECT_NAME}/MPL/TypeListOperations.h
    include/${PROJECT_NAME}/ECS/Settings.h
    include/${PROJECT_NAME}/ECS/Entity.h
    include/${PROJECT_NAME}/ECS/Manager.h
    include/${PROJECT_NAME}/ECS/detail/ArchetypeStorage.h
    include/${PROJECT_NAME}/logger/Logger.h
    include/${PROJECT_NAME}/ECSConfig.h
    include/${PROJECT_NAME}/JobScheduler.h
    include/${PROJECT_NAME}/Exception.h
    include/${PROJECT_NAME}/File.h
    include/${PROJECT_NAME}/Timing.h
    include/${PROJECT_NAME}/Types.h)

    target_link_libraries(${PROJECT_NAME} dp::thread-pool)
target_include_directories(${PROJECT_NAME} PRIVATE 
    thread-pool
    include/${PROJECT_NAME})