cmake_minimum_required(VERSION 3.31)
project(Soil)

set(CMAKE_CXX_STANDARD 26)

add_library(Soil
        external/tfd/tinyfiledialogs.c
        src/clay/clay_binding.cpp
        src/clay/render.cpp
        src/clay/clay_binding.hpp
        src/clay/templates.hpp
        src/clay/components/header.hpp
        src/types.hpp
        src/Application/application.cpp
        src/Application/components/folder_tree.cpp
        src/Application/project/filesystem_tree.cpp
        src/Application/application_sidebar.cpp
        src/Application/application.hpp
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
        GIT_TAG main # Use the appropriate branch, tag, or commit
)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/0bdccf4aa2f5c67af967193caf31d42d5c49bde2.zip
)

# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)



FetchContent_MakeAvailable(clay_headers)
FetchContent_MakeAvailable(raylib)
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
target_include_directories(Soil PUBLIC ${clay_headers_SOURCE_DIR})
target_include_directories(Soil PUBLIC src)
target_include_directories(Soil PUBLIC external)

add_library(clay INTERFACE)
target_link_libraries(Soil PUBLIC raylib)

add_executable(SoilUi main.cpp)
target_include_directories(SoilUi PUBLIC ${clay_headers_SOURCE_DIR})
target_include_directories(SoilUi PUBLIC external)
target_include_directories(SoilUi PUBLIC src)


target_link_libraries(SoilUi PUBLIC raylib)
target_link_libraries(SoilUi PUBLIC Soil)
target_compile_options(Soil PRIVATE -Wno-c99-designator)
target_compile_options(SoilUi PRIVATE -Wno-c99-designator)

enable_testing()

include(GoogleTest)

add_executable(
        fallback_allocator
        tests/fallback_allocator.cpp
)

target_link_libraries(fallback_allocator PUBLIC raylib)
target_link_libraries(fallback_allocator PUBLIC Soil)
target_link_libraries(
        fallback_allocator
        PUBLIC
        GTest::gtest_main
)

gtest_discover_tests(fallback_allocator)
