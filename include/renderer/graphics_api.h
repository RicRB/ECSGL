#pragma once
#include "material.h"
#include "mesh.h"
#include "transform.h"
#include <string>
#include <vector>
using GraphicsAPIType = uint16_t;
constexpr GraphicsAPIType GRAPHICS_API_OPENGL = 0;
constexpr GraphicsAPIType GRAPHICS_API_VULKAN = 1;
class GraphicsAPI {
private:
	
public:

	VkDevice device;
	bool frameBufferResized = false;
	virtual ~GraphicsAPI() = default;
	// Initialize the graphics API
	virtual void initialize(GLFWwindow* window) = 0;
	// Render a frame
	virtual void drawFrame(
		const glm::mat4 viewProjection,
		const GlobalUBO* globalUniformObject,
		const int entityCount, 
		const std::vector<Transform*> transforms, 
		const std::vector<MeshData*> meshes, 
		const std::vector<MaterialData*> materials, 
		const std::vector<ShaderData>* shaders
	) = 0;
	// Cleanup resources
	virtual void cleanup() = 0;
	// create Shader
	virtual void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) = 0;
	virtual void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) = 0;
	virtual void loadMesh(MeshData* mesh) = 0;
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<GraphicsAPI*>(glfwGetWindowUserPointer(window));
		app->frameBufferResized = true;
		std::cout << "Framebuffer resized: " << width << "x" << height << std::endl;
	}
};