#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <optional>
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

class VulkanRenderer {
private:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;
	bool enableValidationLayers = true;
public:
	void init();
	std::vector<const char*> getRequiredExtensions();
	void mainLoop();
	void cleanup();
private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void pickPhysicsalDevice();
	bool isPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice, int* score);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};