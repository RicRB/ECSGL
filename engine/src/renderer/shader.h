#pragma once
#include <string>
#include "glm/glm.hpp"
#include "program.h"
#include "memory/memory_pool.hpp"
using GLProgramID = unsigned int;
using GLenum = unsigned int;

struct ShaderCreateInfo
{
	bool alwaysRender = false;
};

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
	int sourceHash;
	bool alwaysRender = false;
	bool operator==(const ShaderData& other) const {
		return sourceHash == other.sourceHash;
	}
	bool operator!=(const ShaderData& other) const {
		return sourceHash != other.sourceHash;
	}
};
using ShaderDataHandle = MemoryHandle;
struct Shader
{
	ShaderDataHandle handle;
	Shader(const ShaderDataHandle h = ShaderDataHandle()) : handle(h) {};
};
