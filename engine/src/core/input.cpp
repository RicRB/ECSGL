#include "input.h"
#include <array>
#include "GLFW/glfw3.h"
std::weak_ptr<Window> Input::window;

void Input::Init(const std::shared_ptr<Window>& window)
{
	Input::window = window;
	Input::window.lock();
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
