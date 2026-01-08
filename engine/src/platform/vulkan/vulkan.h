#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_raii.hpp>
#include <shaderc/shaderc.hpp>
#include <vector>
#include <optional>
#include <string>
#include <array>
#include "renderer/graphics_api.h"
#include "renderer/buffer.h"
#include "vk_buffer.h"
#include "vk_image.h"
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
private:
	VkInstance instance;
	VkDevice device;
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
	VkDescriptorSetLayout materialDescriptorSetLayout;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkDescriptorPool descriptorPool;
	VkDescriptorPool imGuiDescriptorPool;
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
	const int MAX_OBJECTS = 1024;
	const unsigned int MAX_MATERIALS = 1024;
	uint32_t currentFrame = 0;

	GlobalUBO globalUBO;
	VkUniformBuffer globalUniformBuffer;
	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSetLayout globalDescriptorSetLayout;
	QueueFamilyIndices indices;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

public:
	Vulkan() {
		graphicsAPIType = GRAPHICS_API_VULKAN;
	}
	void initialize(GLFWwindow* window) override;
	void initializeImGui() override;
	void drawFrame(FrameDrawData& frameDrawData) override;
	void beginDrawImGui() override;
	void endDrawImGui() override;
	std::vector<const char*> getRequiredExtensions();
	void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) override;
	void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) override;
	void loadMesh(MeshData* mesh) override;
	void loadTexture(TextureData* textureData) override;

	void cleanup();
	void unloadShader(ShaderData* shaderData) override;
	void unloadMaterial(MaterialData* materialData) override;
	void unloadMesh(MeshData* mesh) override;
	void unloadTexture(TextureData* textureData) override;
	void bindTextureToMaterial(MaterialData* materialData, TextureData* textureData, int binding) override;
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
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
	void createGraphicsPipeline(const char* name, const std::string& vertexSource, const std::string& fragmentSource, VkPipeline* pipeline, VkPipelineLayout* pipelineLayout);
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
	void createRenderPass();
	std::vector<uint32_t> compileShaderGLSLToSPIRV(const std::string& source, shaderc_shader_kind kind, bool optimize);
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncObjects();
	void cleanupSwapChain();
	void recreateSwapChain();
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkVertexBuffer* vertexBuffer);
	void createIndexBuffer(const std::vector<unsigned int>& indices, VkIndexBuffer* indexBuffer);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createDescriptorSetLayouts();
	void createGlobalUniformBuffer();
	void createDescriptorPool();
	void createUniformBuffer(VkUniformBuffer* buffer, const VkDeviceSize bufferSize);
	void createDescriptorSet(VkUniformBuffer* buffer, const VkDescriptorSetLayout* descriptorSetLayout, const VkDescriptorType type, VkDescriptorSet* descriptorSet);
	inline void updateUniformBufferData(VkUniformBuffer* buffer, const void* data);
	void createDepthResources();
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createTextureImageView(VkTextureImage* textureImage);
	void createTextureSampler(VkTextureImage* textureImage);
	constexpr std::array<VkVertexInputAttributeDescription, 3> getVertexAttributeDescriptions() const;
	constexpr VkVertexInputBindingDescription getVertexBindingDescription() const;
};