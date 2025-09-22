
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
)

add_library(Core STATIC)

target_sources(Core PRIVATE ${SOURCES})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

target_link_libraries(Core PRIVATE 
glfw 
GLEW::GLEW 
Vulkan::Vulkan 
Vulkan::shaderc_combined
)

target_include_directories(Core PUBLIC
    "src"
    "include"
    "vendor/stb_image"
)