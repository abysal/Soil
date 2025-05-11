target_sources(${PROJECT_NAME} PRIVATE main.cpp main_page.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/frontend/include)

# Link frontend of nui inside of emscripten
target_link_libraries(${PROJECT_NAME} PRIVATE nui-frontend)

nui_prepare_emscripten_target(
    TARGET
        ${PROJECT_NAME}
    PARCEL_ARGS
        # --no-optimize
    # Provide this argument if you dont want the main application bundled.
    # This can be useful if it gets large and you want to show a load screen.
    # LEAN_INDEX_HTML
    NPM_INSTALL_ARGS
        # --verbose
    PREJS
        ${PREJS_FILE}
    EMSCRIPTEN_LINK_OPTIONS
        -sEXPORTED_FUNCTIONS=_frontendMain
        -O2
        -sALLOW_MEMORY_GROWTH=1
        # -g
        # or other options, see emscripten docs.
    EMSCRIPTEN_COMPILE_OPTIONS
        -O3
        # -g
)