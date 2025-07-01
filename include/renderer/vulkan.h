#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <shaderc/shaderc.hpp>
#include <vector>
#include <optional>
#include <string>
#include <array>
#include "graphics_api.h"
#ifdef max
#undef max
#endif
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Vulkan : public GraphicsAPI {
public:
	VkDevice device;
private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorPool descriptorPool;
	bool enableValidationLayers = true;

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	const int MAX_OBJECTS = 1000;
	uint32_t currentFrame = 0;
	bool framebufferResized = false;

public:
	void initialize(GLFWwindow* window) override;
	void drawFrame(const glm::mat4 viewProjection,
		const int entityCount, 
		const std::vector<Transform*> transforms, 
		const std::vector<MeshData*> meshes, 
		const std::vector<MaterialData*> materials, 
		const std::vector<ShaderData>* shaders,
		const std::vector<DirectionLightUniform>* directionalLights,
		const std::vector<PointLightUniform>* pointLights
	) override;
	std::vector<const char*> getRequiredExtensions();
	void cleanup();
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
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
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	void createSurface();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createGraphicsPipeline(const char* name, const std::string& vertexSource, const std::string& fragmentSource, VkPipeline* pipeline, VkPipelineLayout* pipelineLayout);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
	void createRenderPass();
	std::vector<uint32_t> compileShaderGLSLToSPIRV(const std::string& sourcePath, shaderc_shader_kind kind, bool optimize);
	std::vector<uint32_t> compileShaderGLSLToSPIRV(const char* name, const std::string& source, shaderc_shader_kind kind, bool optimize);
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void createSyncObjects();
	void cleanupSwapChain();
	void recreateSwapChain();
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkVertexBuffer* vertexBuffer);
	void createIndexBuffer(const std::vector<unsigned int>& indices, VkIndexBuffer* indexBuffer);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);
	void createDescriptorPool();
	void createDescriptorSets();

	void createUniformBuffer(VkUniformBuffer* buffer, const VkDeviceSize bufferSize);
	void createDescriptorSet(VkUniformBuffer* buffer, VkDescriptorSet* descriptorSet);
	void loadShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) override;
	void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) override;
	inline void updateUniformBufferData(VkUniformBuffer* buffer, const void* data);

	void loadMesh(MeshData* mesh) override;
};