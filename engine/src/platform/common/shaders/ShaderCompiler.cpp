#include "ShaderCompiler.h"
#include <shaderc/shaderc.hpp>
#include <stdexcept>
#include <iostream>

std::vector<uint32_t> ShaderCompiler::compileShaderGLSLToSPIRV(const std::string_view source, const ShaderType shaderType, bool optimize)
{
	shaderc_shader_kind kind;
	switch(shaderType) {
		case ShaderType::Vertex:
			kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
			break;
		case ShaderType::Fragment:
			kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
			break;
		case ShaderType::Compute:
			kind = shaderc_shader_kind::shaderc_glsl_compute_shader;
			break;
		default:
			throw std::runtime_error("Unknown shader type");
	}

	shaderc::Compiler compiler;
	shaderc::CompileOptions options;
	if (optimize) {
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
	}
	options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
	options.SetTargetSpirv(shaderc_spirv_version_1_0); // Or 1_5 for newer Vulkan
	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.data(), kind, "Shader", options);
	if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
		throw std::runtime_error(result.GetErrorMessage());
	}
	if (result.GetNumWarnings() > 0) {
		std::cerr << result.GetErrorMessage();
	}
	return { result.cbegin(), result.cend() };
}
