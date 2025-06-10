#include "renderer/vulkan.h"
#include <map>

void VulkanRenderer::init()
{

	//const uint32_t WIDTH = 800;
	//const uint32_t HEIGHT = 600;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	// create instance
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// create instance info 
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	// setup extensions
	auto extensions = getRequiredExtensions();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
	// validation layer and debug messenger for creation/destruction setup
#ifdef NDEBUG
	enableValidationLayers = false; // disable validation layers in release mode
	instanceCreateInfo.enabledLayerCount = 0; // no validation layers in release mode
	instanceCreateInfo.pNext = nullptr; // no debug messenger in release mode
#else
	enableValidationLayers = true; // enable validation layers in debug mode
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

	populateDebugMessengerCreateInfo(debugMessengerCreateInfo);
	instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo;
#endif

	// create instance fr
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

#ifdef NDEBUG
	
#else
	std::uint32_t layerCount; 
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerProperties : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerProperties.layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			throw std::runtime_error("validation layer not found!");
		}
	}
#endif

	setupDebugMessenger();
	pickPhysicsalDevice();

}

std::vector<const char*> VulkanRenderer::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
	return std::vector<const char*>();
}

void VulkanRenderer::cleanup()
{
	if (enableValidationLayers) {
		// destroy messenger function
		auto destroyDebugUtilsMessengerEXTFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (destroyDebugUtilsMessengerEXTFunction != nullptr) {
			destroyDebugUtilsMessengerEXTFunction(instance, debugMessenger, nullptr);
		}
	}

	vkDestroyInstance(instance, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRenderer::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	VkDebugUtilsMessageTypeFlagsEXT messageType, 
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
	void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	}
	return VK_FALSE;
}

void VulkanRenderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}

void VulkanRenderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	auto createDebugUtilsMessengerEXTFunction = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (createDebugUtilsMessengerEXTFunction != nullptr) {
		if (createDebugUtilsMessengerEXTFunction(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
	else {
		throw std::runtime_error("failed to find vkCreateDebugUtilsMessengerEXT function!");
	}
}

void VulkanRenderer::pickPhysicsalDevice()
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	// check if device is suitable
	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<int, VkPhysicalDevice> candidates;
	for (const auto& device : devices) {
		int score;
		if (isPhysicalDeviceSuitable(device, &score)) {
			candidates.insert(std::make_pair(score, device));
		}
	}
	
	if (candidates.rbegin()->first > 0) {
		// Get the device with the highest score
		physicalDevice = candidates.rbegin()->second;
	} else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool VulkanRenderer::isPhysicalDeviceSuitable(VkPhysicalDevice device, int* score)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);
	bool isSuitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader && indices.isComplete();
	if (isSuitable)
	{
		if (score) {
			*score += 1000; // arbitrary score for discrete GPU
			score += deviceProperties.limits.maxImageDimension2D; // add max image dimension to score
		}
	}
	return isSuitable; // check if the device supports geometry shaders
}

QueueFamilyIndices VulkanRenderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) break;

		i++;
	}

	return indices;
}
