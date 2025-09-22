# CMakeList.txt : CMake project for ECSGL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Recurse into source files
file(GLOB_RECURSE ECSGL_SOURCES
    src/*.cpp
    vendor/*.cpp
)

# Remove ImGui's main.cpp (we use our own main in Application.cpp)
list(REMOVE_ITEM ECSGL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/main.cpp")

project (ECSGL)


set(CMAKE_CXX_STANDARD 23)

include(Dependencies.cmake)

add_subdirectory(Core)
add_subdirectory(App)

set(RES_DIR "${CMAKE_SOURCE_DIR}/res")
file(TO_CMAKE_PATH "${RES_DIR}" RES_DIR_PATH)

target_compile_definitions(Core PUBLIC RES_DIR="${RES_DIR_PATH}")
target_compile_definitions(App PUBLIC RES_DIR="${RES_DIR_PATH}")



