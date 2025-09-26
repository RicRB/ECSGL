#include "window.h"
#include "engine.h"
#include "platform/opengl/gl_def.h"
Window::Window(const WindowSpecification& specification) : specification(specification)
{
}

Window::~Window()
{
}

void Window::create() {
    if (*engine::currentRendererType == GRAPHICS_API_OPENGL)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS compatibility
    }
    else if (*engine::currentRendererType == GRAPHICS_API_VULKAN)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Prevents OpenGL context creation
    }
    /* Create a windowed mode window and its OpenGL context */
    handle = glfwCreateWindow(specification.width, specification.height, specification.title.c_str(), NULL, NULL);
}

void Window::destroy() {
	if (handle) {
		glfwDestroyWindow(handle);
		handle = nullptr;
	}
}

void Window::update()
{
	glfwSwapBuffers(handle);
}
