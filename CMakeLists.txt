cmake_minimum_required(VERSION 3.15)
project(Platformer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/audio.cpp
    src/levels.cpp
    src/procgen.cpp
    src/render.cpp
    src/game.cpp
)

if(EMSCRIPTEN)
    set(RAYLIB_INCLUDE "" CACHE PATH "Path to raylib/include for web build")
    set(RAYLIB_LIB     "" CACHE FILEPATH "Path to libraylib.a for web build")

    add_executable(Platformer ${SOURCES})
    target_include_directories(Platformer PRIVATE "${RAYLIB_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(Platformer PRIVATE "${RAYLIB_LIB}")
    target_link_options(Platformer PRIVATE
        -sUSE_GLFW=3
        -sWASM=1
        -sALLOW_MEMORY_GROWTH=1
        -sGL_ENABLE_GET_PROC_ADDRESS=1
        -sFORCE_FILESYSTEM=1
        -sINITIAL_MEMORY=67108864
        --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/src/web_shell.html"
    )
    set_target_properties(Platformer PROPERTIES
        SUFFIX ".html"
        OUTPUT_NAME "index"
    )
else()
    set(RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/raylib")

    add_executable(Platformer ${SOURCES})
    target_include_directories(Platformer PRIVATE
        "${RAYLIB_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    target_link_libraries(Platformer PRIVATE
        "${RAYLIB_DIR}/lib/raylib.lib"
        winmm opengl32 gdi32 kernel32 user32 winspool shell32
        ole32 oleaut32 uuid comdlg32 advapi32
    )
endif()
