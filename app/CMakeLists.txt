# App

file(GLOB_RECURSE APP_SOURCES
    src/*.cpp
)
file(GLOB_RECURSE APP_HEADERS
    src/*.h
    src/*.hpp
)
add_executable(App)

target_sources(App PRIVATE ${APP_SOURCES} ${APP_HEADERS})

# Match VS filters to directory structure on disk
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_SOURCES} ${APP_HEADERS})

add_compile_definitions(
    GLFW_INCLUDE_NONE
    GLM_ENABLE_EXPERIMENTAL
)

target_link_libraries(App Engine)

target_include_directories(App PRIVATE src)

# Set working directory for VS debugger
set_target_properties(App PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/app"
)