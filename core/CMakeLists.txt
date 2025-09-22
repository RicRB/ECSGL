
# core

set(SOURCES
src/renderer/opengl.cpp
src/renderer/vulkan.cpp
src/application.cpp
src/engine.cpp
src/gl_def.cpp
src/input.cpp
src/renderer.cpp
src/resource_manager.cpp
src/scenegraph.cpp
src/shader.cpp
src/time.cpp
vendor/stb_image/stb_image.cpp
vendor/imgui/imgui.cpp
vendor/imgui/backends/imgui_impl_glfw.cpp
vendor/imgui/backends/imgui_impl_opengl3.cpp
vendor/imgui/backends/imgui_impl_vulkan.cpp
vendor/imgui/imgui_draw.cpp
vendor/imgui/imgui_tables.cpp
vendor/imgui/imgui_widgets.cpp
)
file(GLOB_RECURSE INCLUDE
    include/*.h
    include/*.hpp
)
add_library(Core STATIC)

target_sources(Core PRIVATE ${SOURCES} ${INCLUDE})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES} ${INCLUDE})
add_compile_definitions(
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
)

target_link_libraries(Core PRIVATE 
glfw 
libglew_static 
Vulkan::Vulkan 
Vulkan::shaderc_combined
)

target_include_directories(Core PUBLIC
    "src"
    "include"
    "vendor"
    "vendor/stb_image"
    "vendor/glm"
    "vendor/imgui"
)