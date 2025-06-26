#pragma once
#include "material.h"
#include <string>
class GraphicsAPI {
public:
	virtual ~GraphicsAPI() = default;
	// Initialize the graphics API
	virtual void initialize() = 0;
	// Render a frame
	virtual void renderFrame() = 0;
	// Cleanup resources
	virtual void cleanup() = 0;
	// Get the name of the graphics API
	virtual const char* getName() const = 0;
	// create Shader
	virtual void loadShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) = 0;
	virtual void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) = 0;
};