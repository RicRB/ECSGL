
# engine

set(VENDOR_SOURCES
vendor/stb_image/stb_image.cpp
vendor/imgui/imgui.cpp
vendor/imgui/backends/imgui_impl_glfw.cpp
vendor/imgui/backends/imgui_impl_opengl3.cpp
vendor/imgui/backends/imgui_impl_vulkan.cpp
vendor/imgui/imgui_draw.cpp
vendor/imgui/imgui_tables.cpp
vendor/imgui/imgui_widgets.cpp
)
file(GLOB_RECURSE ENGINE_SOURCES
    src/*.cpp
)

file(GLOB_RECURSE HEADERS
    src/*.h
    src/*.hpp
)
add_library(Engine STATIC)

target_sources(Engine PRIVATE ${VENDOR_SOURCES} ${ENGINE_SOURCES} ${HEADERS})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${VENDOR_SOURCES} ${ENGINE_SOURCES} ${HEADERS})
add_compile_definitions(
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
)

target_link_libraries(Engine PRIVATE 
glfw 
libglew_static 
Vulkan::Vulkan 
Vulkan::shaderc_combined
)

target_include_directories(Engine PUBLIC
    "src"
    "vendor"
    "vendor/stb_image"
    "vendor/glm"
    "vendor/imgui"
)