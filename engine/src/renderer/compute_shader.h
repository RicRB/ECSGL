#pragma once
#include <string>
#include "renderer/program.h"
#include "memory/memory_pool.hpp"
#include "renderer/descriptor_set.h"
struct ComputeShaderData
{
	ComputeShaderData();
	// source
	std::string source;
	std::shared_ptr<Program> program;
	DescriptorSetReflection descriptorReflection;
	// metadata
	int sourceHash;
	bool operator==(const ComputeShaderData& other) const {
		return sourceHash == other.sourceHash;
	}
	bool operator!=(const ComputeShaderData& other) const {
		return sourceHash != other.sourceHash;
	}
};
using ComputeShaderDataHandle = MemoryHandle<ComputeShaderData>;
struct ComputeShader
{
	ComputeShaderDataHandle handle;
	ComputeShader(const ComputeShaderDataHandle h = ComputeShaderDataHandle()) : handle(h) {};
};
