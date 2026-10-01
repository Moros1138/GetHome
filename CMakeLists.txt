cmake_minimum_required(VERSION 3.10)
project(GetHome)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(GetHome)
target_sources(GetHome
    PRIVATE
    src/main.cpp
    src/olcPGEX_AnimatedSprite.h
    src/olcPixelGameEngine.h
    src/tileson.hpp
)

if(UNIX AND NOT APPLE)
    target_link_options(GetHome PRIVATE
        -lX11
        -lGL
        -lpng
        -lstdc++
        -lpthread
        -lm
    )
endif()

if(EMSCRIPTEN)
    target_link_options(GetHome PRIVATE --preload-file ${ASSETS_DIR}@assets)
else()
    # Copy assets when not Web platform
    add_custom_command(
        TARGET GetHome
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        ARGS -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/assets
    )        
endif()