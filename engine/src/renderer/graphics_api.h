#pragma once
#include "components/material.h"
#include "components/mesh.h"
#include "components/transform.h"
#include <string>
#include <vector>
#include "stb_image/stb_image.h"
#include "GLFW/glfw3.h"
using GraphicsAPIType = uint32_t;
constexpr GraphicsAPIType GRAPHICS_API_OPENGL = 0;
constexpr GraphicsAPIType GRAPHICS_API_VULKAN = 1;
class GraphicsAPI {
protected:
	GraphicsAPIType graphicsAPIType;
private:

public:
	GLFWwindow* window = nullptr;
	bool frameBufferResized = false;
	bool isImGuiInitialized = false;
	virtual ~GraphicsAPI() = default;
	// Initialize the graphics API
	virtual void initialize(GLFWwindow* window) = 0;
	virtual void initializeImGui() = 0;
	// Render a frame
	virtual void drawFrame(
		const glm::mat4 viewMatrix,
		const glm::mat4 projectionMatrix,
		const GlobalUBO* globalUniformObject,
		const int entityCount, 
		const std::vector<Transform*> transforms, 
		const std::vector<MeshData*> meshes, 
		const std::vector<MaterialData*> materials, 
		const std::vector<ShaderData>* shaders
	) = 0;
	virtual void beginDrawImGui() = 0;
	virtual void endDrawImGui() = 0;
	// Cleanup resources
	virtual void cleanup() = 0;
	// create Shader
	virtual void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) = 0;
	virtual void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) = 0;
	virtual void loadMesh(MeshData* mesh) = 0;
	virtual void loadTexture(TextureData* textureData) = 0;
	virtual void unloadShader(ShaderData* shaderData) = 0;
	virtual void unloadMaterial(MaterialData* materialData) = 0;
	virtual void unloadMesh(MeshData* mesh) = 0;
	virtual void unloadTexture(TextureData* textureData) = 0;
	GraphicsAPIType getType() {
		return graphicsAPIType;
	}
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<GraphicsAPI*>(glfwGetWindowUserPointer(window));
		app->frameBufferResized = true;	
	}
};