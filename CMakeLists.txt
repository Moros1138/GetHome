cmake_minimum_required(VERSION 3.15)
project(GetHome)
 
# Add them to the include paths
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/olcPixelGameEngine3"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/olcPixelGameEngine3/extensions/miniaudio"
)

# Wrangle Source files
file(
    GLOB SOURCES
    src/*.c
    src/*.cpp
    src/*.h
    src/*.hpp
)

# Include helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Helpers.cmake)

# platform detection and preparation
pge3_prepare_platforms()

# add_executable with all of the pge3 batteries includes
pge3_add_program(${CMAKE_PROJECT_NAME} ${SOURCES})

# copy/manage assets directory
pge3_handle_assets(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/assets)
