
project(bedrock_lsp LANGUAGES CXX)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        src/*.cpp
        src/*.hpp
)

add_executable(bedrock_lsp ${SOURCES})


target_include_directories(bedrock_lsp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
find_package(hjson CONFIG REQUIRED)
target_link_libraries(bedrock_lsp PRIVATE hjson)

