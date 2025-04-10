# CMakeList.txt : CMake project for ECSGL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Recurse into source files
file(GLOB_RECURSE ECSGL_SOURCES
    src/*.cpp
    vendor/*.cpp
)

# Remove ImGui's main.cpp (we use our own main in Application.cpp)
list(REMOVE_ITEM ECSGL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/main.cpp")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ECSGL")

# Add source to this project's executable.
add_executable (ECSGL ${ECSGL_SOURCES})

# Enable vcpkg manifest mode if using vcpkg
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ECSGL PROPERTY CXX_STANDARD 20)
endif()


# Include directories
target_include_directories(ECSGL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
)
# Link libraries from vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

target_link_libraries(ECSGL PRIVATE glfw GLEW::GLEW assimp::assimp)

# Copy the 'res' directory to the build directory
add_custom_command(TARGET ECSGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:ECSGL>/res
)

