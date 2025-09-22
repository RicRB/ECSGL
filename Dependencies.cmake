include(FetchContent)
# ------------------ GLFW ------------------
find_package(glfw3 3.4 QUIET)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
            glfw3
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    )
    FetchContent_GetProperties(glfw3)
    if (NOT glfw3_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(glfw3)
        add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
    endif()
endif()
# ------------------ GLEW ------------------
find_package(glew CONFIG QUIET)
if (NOT glew_FOUND)
    FetchContent_Declare(
      glew
      GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
      GIT_TAG        origin/master
    )
    FetchContent_GetProperties(glew)
    if(NOT glew_POPULATED)
      FetchContent_Populate(glew)

      add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()
# ------------------ Vulkan ------------------
find_package(Vulkan COMPONENTS shaderc_combined)