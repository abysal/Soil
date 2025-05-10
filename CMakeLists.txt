cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
project(Soil)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(GRAPHICS GRAPHICS_API_OPENGL_43)

set(SOURCE_FILES external/tfd/tinyfiledialogs.c
        soil_application/soil.cpp)

file(GLOB_RECURSE SOURCE_FILES_REST CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/soil_application/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/soil_application/*.cpp)

list(APPEND SOURCE_FILES ${SOURCE_FILES_REST})


include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)



add_library(Soil
        ${SOURCE_FILES})

target_include_directories(Soil PUBLIC external)
target_include_directories(Soil PUBLIC resources)
target_include_directories(Soil PUBLIC soil_application)
target_compile_definitions(Soil PUBLIC NOMINMAX)


if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(Soil PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wsign-conversion
                -Wundef
                -Wcast-align
                -Wnon-virtual-dtor
                -Wold-style-cast
                -Woverloaded-virtual
                -Wdouble-promotion
                -Wformat=2
                -Wnull-dereference
                -Wno-unused-parameter
                -Wno-nullability-extension
                -Wno-nullability-completeness
                -Wno-c23-extensions
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Werror
                -Wno-newline-eof
                -Wno-shadow
                -Wno-global-constructors
                -Wno-exit-time-destructors
                -Wno-missing-prototypes
                -Wno-unused-macros
        )


        message(STATUS "Using Clang, throwing in all warnings")

        set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/external/tfd/tinyfiledialogs.c
                PROPERTIES
                COMPILE_FLAGS "-w -Wno-error"
        )
        set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/soil_application/rml_ui_backend/RmlUi_Renderer_GL3.cpp
                PROPERTIES
                COMPILE_FLAGS "-w -Wno-error"
        )
        set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/soil_application/rml_ui_backend/RmlUi_Platform_GLFW.cpp
                PROPERTIES
                COMPILE_FLAGS "-w -Wno-error"
        )
        set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/soil_application/rml_ui_backend/RmlUi_Backend_GLFW_GL3.cpp
                PROPERTIES
                COMPILE_FLAGS "-w -Wno-error"
        )
        set_source_files_properties(
                ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/glad.h
                PROPERTIES
                COMPILE_FLAGS "-w -Wno-error"
        )
endif()


find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Soil PUBLIC glfw)

find_package(Freetype REQUIRED)
target_link_libraries(Soil PUBLIC Freetype::Freetype)

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(Soil PUBLIC Microsoft.GSL::GSL)

add_executable(SoilUi main.cpp)
target_include_directories(SoilUi PUBLIC external)

find_package(RmlUi CONFIG REQUIRED)
target_link_libraries(SoilUi PUBLIC RmlUi::RmlUi)

find_package(reflectcpp CONFIG REQUIRED)
target_link_libraries(SoilUi PRIVATE reflectcpp::reflectcpp)

file(COPY ${CMAKE_SOURCE_DIR}/resources
        DESTINATION ${CMAKE_BINARY_DIR}/)


target_link_libraries(SoilUi PUBLIC Soil)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(Soil PRIVATE -Wno-c99-designator)
        target_compile_options(SoilUi PRIVATE -Wno-c99-designator)
endif()

