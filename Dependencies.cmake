# ------------------ GLFW ------------------
find_package(glfw3 3.4 QUIET)
# ------------------ GLEW ------------------
find_package(GLEW CONFIG QUIET)

# ------------------ Vulkan ------------------
find_package(Vulkan COMPONENTS shaderc_combined)