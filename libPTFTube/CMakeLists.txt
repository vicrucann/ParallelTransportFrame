cmake_minimum_required(VERSION 2.8.11)

if(POLICY CMP0020)
    CMAKE_POLICY(SET CMP0020 NEW)
endif()
if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

include_directories(${CMAKE_SOURCE_DIR})

set (libPTF_SRCS
    PTFTube.h
    PTFTube.cpp
    )

add_library(libPTFTube
    STATIC ${libPTF_SRCS}
    )

target_link_libraries( libPTFTube
${OPENSCENEGRAPH_LIBRARIES}
)