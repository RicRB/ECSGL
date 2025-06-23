#pragma once
#include "shader.h"
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
	virtual void createShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, const ShaderData* shaderData) = 0;
};