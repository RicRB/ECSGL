#pragma once
#include <string>
#include "memory/memory_pool.hpp"

struct ComputeShaderData
{
	ComputeShaderData();
	// source
	std::string source;
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
