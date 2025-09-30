#pragma once
#include <string>
struct GLFWwindow;

struct WindowSpecification
{
	std::string title = "Engine";
	uint32_t width = 1280;
	uint32_t height = 720;
	bool isResizeable = true;
	bool vSync = true;
};

class Window {
private:
	GLFWwindow* handle = nullptr;
	WindowSpecification specification;
public:
	Window(const WindowSpecification& specification = WindowSpecification());
	~Window();
	void create();
	void destroy();
	void update();
	GLFWwindow* getHandle() const { return handle; }
};