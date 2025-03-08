cmake_minimum_required(VERSION 3.31)
project(Soil)

set(CMAKE_CXX_STANDARD 26)

add_executable(Soil
        src/main.cpp
        src/clay/clay_binding.cpp
        src/clay/clay_binding.hpp
        src/clay/templates.hpp
        src/clay/components/header.hpp
        src/types.hpp
        src/Application/Application.cpp
        src/Application/Application.hpp
        src/Application/header_bar.hpp
        src/Application/editor_config.hpp
        src/Application/jet_brains_mono.hpp)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
)

# Declare the external repository
FetchContent_Declare(
        clay_headers
        GIT_REPOSITORY https://github.com/abysal/clay_headers.git
        GIT_TAG main  # Use the appropriate branch, tag, or commit
)


FetchContent_MakeAvailable(clay_headers)
FetchContent_MakeAvailable(raylib)
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
target_include_directories(Soil PRIVATE ${clay_headers_SOURCE_DIR})

add_library(clay INTERFACE)
target_link_libraries(Soil PRIVATE raylib)
