cmake_minimum_required(VERSION 3.30)
project(SoilDx12)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(KIERO_INCLUDE_D3D12 ON)
set(SAFETYHOOK_FETCH_ZYDIS ON)
set(ZYDIS_STATIC_BUILD ON)
include(FetchContent)

FetchContent_Declare(
    kiero
    GIT_REPOSITORY "https://github.com/BasedInc/kiero.git"
    GIT_TAG "master"
)
FetchContent_MakeAvailable(kiero)
include(FetchContent)

# Safetyhook
FetchContent_Declare(
    safetyhook
    GIT_REPOSITORY "https://github.com/cursey/safetyhook.git"
    GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(safetyhook)

add_library(SoilDx12 SHARED src/dllmain.cpp)
target_link_libraries(SoilDx12 PRIVATE kiero)
target_link_libraries(SoilDx12 PRIVATE safetyhook)

message("CMAKE_CURRENT_LIST_DIR is ${CMAKE_CURRENT_LIST_DIR}")
target_include_directories(SoilDx12 PRIVATE ${kiero_SOURCE_DIR}/include)
target_include_directories(SoilDx12 PRIVATE ${safetyhook_SOURCE_DIR}/include)
FetchContent_Declare(dx12_headers
    GIT_REPOSITORY https://github.com/microsoft/DirectX-Headers.git
    GIT_TAG "main"
)

FetchContent_MakeAvailable(dx12_headers)

target_include_directories(SoilDx12 PUBLIC ${dx12_headers_SOURCE_DIR}/include)
