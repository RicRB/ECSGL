#pragma once
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "program.h"
#include "memory/memory_pool.hpp"
using GLProgramID = unsigned int;
using GLenum = unsigned int;
struct ShaderUniformMetaData
{
	char* data = nullptr;
	GLenum* uniformTypes = nullptr;
	uint16_t* uniformOffsets = 0;
	uint16_t size = 0;
	uint16_t uniformCount = 0;
};
struct ShaderData
{
	ShaderData();
	std::shared_ptr<Program> program;
	// sources
	std::string vertexSource;
	std::string fragmentSource;
	// metadata
	size_t sourceHash;
	bool operator==(const ShaderData& other) const {
		return sourceHash == other.sourceHash;
	}
	bool operator!=(const ShaderData& other) const {
		return sourceHash != other.sourceHash;
	}
};
using ShaderDataHandle = MemoryHandle<ShaderData>;
struct Shader
{
	ShaderDataHandle handle;
	Shader(const ShaderDataHandle h = ShaderDataHandle()) : handle(h) {};
};

namespace shader
{
	int GetShaderUniformLocation(const unsigned int shaderID, const std::string& name);
	void SetUniform1b(const unsigned int shaderID, const std::string& name, bool value);
	void SetUniform1i(const unsigned int shaderID, const std::string& name, int value);
	void SetUniform1f(const unsigned int shaderID, const std::string& name, float value);
	void SetUniform3f(const unsigned int shaderID, const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const unsigned int shaderID, const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform4f(const unsigned int shaderID, const std::string& name, glm::vec4 vec);
	void SetUniformMat3f(const unsigned int shaderID, const std::string& name, glm::mat3& matrix);
	void SetUniformMat4f(const unsigned int shaderID, const std::string& name, const glm::mat4& matrix);

}
