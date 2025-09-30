#include "graphics_api.h"
#include "GLFW/glfw3.h"
void GraphicsAPI::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<GraphicsAPI*>(glfwGetWindowUserPointer(window));
	app->frameBufferResized = true;
}
