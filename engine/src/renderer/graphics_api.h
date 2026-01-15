#pragma once
#include "components/material.h"
#include "components/mesh.h"
#include "components/transform.h"
#include "compute_shader.h"
#include <string>
#include <vector>
#include "stb_image/stb_image.h"
#include "core/window.h"
#include "memory/memory_pool.hpp"
using GraphicsAPIType = uint32_t;
constexpr GraphicsAPIType GRAPHICS_API_OPENGL = 0;
constexpr GraphicsAPIType GRAPHICS_API_VULKAN = 1;

struct FrameDrawData {
	const glm::mat4 viewMatrix;
	const glm::mat4 projectionMatrix;
	const GlobalUBO* globalUniformObject;
	const int entityCount;
	const std::vector<glm::mat4x4>& modelMatrices;
	const std::vector<MeshData*>& meshes;
	const std::vector<MaterialData*>& materials;
	const MemoryPool<ShaderData, 32>& shaders;
};
struct DrawObject {
	size_t index;
	ShaderData& shader;
	MaterialData& material;
	MeshData& mesh;

	DrawObject(size_t i, ShaderData& s, MaterialData& m, MeshData& md)
		: index(i), shader(s), material(m), mesh(md) {
	}
};

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
	virtual void drawFrame(FrameDrawData& frameDrawData) = 0;
	virtual void beginDrawImGui() = 0;
	virtual void endDrawImGui() = 0;
	// Cleanup resources
	virtual void cleanup() = 0;
	// create Shader
	virtual void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) = 0;
	virtual void compileComputeShader(const char* name, ComputeShaderData* computeShaderData) = 0;
	virtual void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) = 0;
	virtual void loadMesh(MeshData* mesh) = 0;
	virtual void loadTexture(TextureData* textureData) = 0;
	virtual void unloadShader(ShaderData* shaderData) = 0;
	virtual void unloadComputeShader(ComputeShaderData* computeShaderData) = 0;
	virtual void unloadMaterial(MaterialData* materialData) = 0;
	virtual void unloadMesh(MeshData* mesh) = 0;
	virtual void unloadTexture(TextureData* textureData) = 0;
	virtual void bindTextureToMaterial(MaterialData* materialData, TextureData* textureData, int binding) = 0;
	// Compute stuff
	GraphicsAPIType getType() {
		return graphicsAPIType;
	}
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
};