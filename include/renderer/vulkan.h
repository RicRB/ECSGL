#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

class VulkanRenderer {
private:
	VkInstance instance;
public:
	void init();
	void mainLoop();
	void cleanup();
};