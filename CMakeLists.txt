cmake_minimum_required(VERSION 2.8.11)
project(PTF_test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

message("OpenSceneGraph is used. Searching for libraries...")
find_package(OpenSceneGraph REQUIRED COMPONENTS osgViewer osgDB osgGA)

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS} )

set(SOURCES
    main.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(libPTFTube)

target_link_libraries(${PROJECT_NAME}
    ${OPENSCENEGRAPH_LIBRARIES}
    libPTFTube
)
