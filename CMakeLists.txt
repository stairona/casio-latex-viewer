cmake_minimum_required(VERSION 3.15)
project(LaTeX_Viewer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use fxsdk's FindGint and GenerateG3A modules
# CMAKE_MODULE_PATH is set by fxsdk build-cg via -DCMAKE_MODULE_PATH
find_package(Gint 2.11 REQUIRED)

set(SOURCES
    src/main.c
    src/latex.c
    src/render.c
    src/layout.c
    src/filebrowser.c
    src/symbols.c
)

add_executable(LaTeX_Viewer ${SOURCES})

target_include_directories(LaTeX_Viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(LaTeX_Viewer Gint::Gint)

include(GenerateG3A)
include(Fxconv)

generate_g3a(
    TARGET LaTeX_Viewer
    OUTPUT "LaTeX_Viewer.g3a"
    NAME "LaTeX View"
)
