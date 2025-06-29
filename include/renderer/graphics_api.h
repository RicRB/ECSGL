#pragma once
#include "material.h"
#include "mesh.h"
#include "transform.h"
#include <string>
#include <vector>
class GraphicsAPI {
public:
	virtual ~GraphicsAPI() = default;
	// Initialize the graphics API
	virtual void initialize() = 0;
	// Render a frame
	virtual void drawFrame(const glm::mat4 viewProjection, const int entityCount, const std::vector<Transform*> transforms, const std::vector<Mesh*> meshes, const std::vector<Material*> materials, const std::vector<ShaderData>* shaders) = 0;
	// Cleanup resources
	virtual void cleanup() = 0;
	// Get the name of the graphics API
	virtual const char* getName() const = 0;
	// create Shader
	virtual void loadShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) = 0;
	virtual void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) = 0;
	virtual void loadMesh(const std::string& name, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices, Mesh* mesh) = 0;
};