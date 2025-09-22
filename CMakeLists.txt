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

project ("ECSGL")
set(CMAKE_CXX_STANDARD 20)
# Add source to this project's executable.
add_executable (ECSGL ${ECSGL_SOURCES})



# Include directories
target_include_directories(ECSGL PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ECS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${Vulkan_INCLUDE_DIRS}
)
# Link libraries from vcpkg
# Link shaderc lib
link_directories("$ENV{VULKAN_SDK}/Lib")


find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW CONFIG REQUIRED)
target_link_libraries(ECSGL PRIVATE glfw GLEW::GLEW Vulkan::Vulkan "$ENV{VULKAN_SDK}/Lib/shaderc_shared.lib")
message(STATUS "$ENV{VULKAN_SDK}/Lib")
message(STATUS "Vulkan include: ${Vulkan_INCLUDE_DIRS}")
# Copy the 'res' directory to the build directory
add_custom_command(TARGET ECSGL POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
   ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:ECSGL>/res
)

