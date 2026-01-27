#include "input.h"
#include <array>
#include "GLFW/glfw3.h"
std::weak_ptr<Window> Input::window;

double Input::scrollX = 0.0;  // Add this
double Input::scrollY = 0.0;  // Add this

void Input::Init(const std::shared_ptr<Window>& window)
{
	Input::window = window;
	Input::window.lock();
	Input::scrollX = 0.0;
	Input::scrollY = 0.0;
	glfwSetScrollCallback(window.get()->getHandle(),
		[](GLFWwindow* window, double xoffset, double yoffset)
		{
			Input::scrollX = xoffset;
			Input::scrollY = yoffset;
		}
	);
}
bool Input::GetKeyPressed(KeyCode key)
{
	return glfwGetKey(window.lock()->getHandle(), static_cast<int>(key)) == GLFW_PRESS;
}
bool Input::GetKeyReleased(KeyCode key)
{
	return glfwGetKey(window.lock()->getHandle(), static_cast<int>(key)) == GLFW_RELEASE;
}
bool Input::GetMouseButtonPressed(KeyCode button)
{
	return glfwGetMouseButton(window.lock()->getHandle(), static_cast<int>(button)) == GLFW_PRESS;
}
bool Input::GetMouseButtonReleased(KeyCode button)
{
	return glfwGetMouseButton(window.lock()->getHandle(), static_cast<int>(button)) == GLFW_RELEASE;
}

double Input::GetVerticalMouseScroll()
{
	double input = Input::scrollY;
	Input::scrollY = 0;
	return input;
}

double Input::GetHorizontalMouseScroll()
{
	return Input::scrollX;
}


