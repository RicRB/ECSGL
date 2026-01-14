#pragma once
#include <iostream>
#include <vector>
enum class ShaderType {
	Vertex,
	Fragment,
	Compute
};

class ShaderCompiler {
public:
	static std::vector<uint32_t> compileShaderGLSLToSPIRV(const std::string& source, const ShaderType shaderType, bool optimize);
};