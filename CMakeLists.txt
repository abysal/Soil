cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
project(Soil)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GRAPHICS GRAPHICS_API_OPENGL_43)

set(SOURCE_FILES external/tfd/tinyfiledialogs.c
        soil_application/clay/clay_binding.cpp
        soil_application/clay/render.cpp
        soil_application/clay/clay_binding.hpp
        soil_application/clay/simple_id_generator.hpp
        soil_application/clay/simple_id_generator.cpp
        soil_application/clay/templates.hpp
        soil_application/clay/components/header.hpp
        soil_application/types.hpp
        soil_application/Application/application.cpp
        soil_application/Application/scintilla/surface.cpp
        soil_application/Application/components/folder_tree.cpp
        soil_application/Application/project/filesystem_tree.cpp
        soil_application/Application/application_sidebar.cpp
        soil_application/Application/application.hpp
        soil_application/Application/header_bar.hpp
        soil_application/Application/editor_config.hpp
        soil_application/Application/jet_brains_mono.hpp
        soil_application/open_gl_magic/ogl.cpp)

set(SCINTILLA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scintilla)

file(GLOB_RECURSE SCINTILLA_SOURCES CONFIGURE_DEPENDS ${SCINTILLA_DIR}/include/*.h ${SCINTILLA_DIR}/src/*.cxx ${SCINTILLA_DIR}/src/*.h)

list(APPEND SOURCE_FILES ${SCINTILLA_SOURCES})

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # don't build the supplied example games
set(MSDFGEN_DISABLE_SVG ON CACHE BOOL "" FORCE)
set(MSDFGEN_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(MSDFGEN_USE_SKIA OFF CACHE BOOL "" FORCE)



if(WIN32)
        list(APPEND SOURCE_FILES "soil_application/DirectXMagic/dx_creation.cpp" "soil_application/Application/windows/injector.cpp" "soil_application/Application/windows/communication_manager.cpp" "soil_application/Application/windows/dx_talker.cpp")
        list(APPEND SOURCE_FILES soil_application/Application/windows/keyboard.cpp)
        FetchContent_Declare(dx12_headers
                GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
                GIT_TAG "main"
        )

        FetchContent_MakeAvailable(dx12_headers)
endif()



add_library(Soil
        ${SOURCE_FILES})

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
)

# Declare the external repository
FetchContent_Declare(
        clay_headers
        GIT_REPOSITORY https://github.com/abysal/clay_headers.git
        GIT_TAG main
)

# FetchContent_Declare(
#         googletest
#         URL https://github.com/google/googletest/archive/0bdccf4aa2f5c67af967193caf31d42d5c49bde2.zip
# )

# # set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)


# FetchContent_Declare(
#         msdfgen
#         GIT_REPOSITORY "https://github.com/Chlumsky/msdfgen.git"
#         GIT_TAG "master"
# )
# FetchContent_MakeAvailable(msdfgen)


FetchContent_MakeAvailable(clay_headers)
FetchContent_MakeAvailable(raylib)
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
target_include_directories(Soil PUBLIC ${clay_headers_SOURCE_DIR})
target_include_directories(Soil PUBLIC soil_application)
target_include_directories(Soil PUBLIC external)
target_include_directories(Soil PUBLIC ${SCINTILLA_DIR}/include)
target_include_directories(Soil PUBLIC ${SCINTILLA_DIR}/src)

add_library(clay INTERFACE)
target_link_libraries(Soil PUBLIC raylib)

add_executable(SoilUi main.cpp)
target_include_directories(SoilUi PUBLIC ${clay_headers_SOURCE_DIR})
target_include_directories(SoilUi PUBLIC external)
target_include_directories(SoilUi PUBLIC soil_application)

target_include_directories(SoilUi PUBLIC ${SCINTILLA_DIR}/include)
target_include_directories(SoilUi PUBLIC ${SCINTILLA_DIR}/src)

add_definitions(-DGRAPHICS_API_OPENGL_43)

if(WIN32)

        target_include_directories(Soil PUBLIC ${dx12_headers_SOURCE_DIR}/include)
        target_include_directories(SoilUi PUBLIC ${dx12_headers_SOURCE_DIR}/include)
        target_include_directories(Soil PUBLIC dx_12_frame_stealer/src)
        target_include_directories(SoilUi PUBLIC dx_12_frame_stealer/src)

endif()


target_link_libraries(SoilUi PUBLIC raylib)
target_link_libraries(SoilUi PUBLIC Soil)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(Soil PRIVATE -Wno-c99-designator)
        target_compile_options(SoilUi PRIVATE -Wno-c99-designator)
endif()

add_subdirectory(dx_12_frame_stealer)
