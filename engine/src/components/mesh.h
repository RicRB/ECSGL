#pragma once
#include <array>
#include <memory>
#include <vector>
#include "renderer/buffer.h"
#include <glm/glm.hpp>
#include "memory/memory_pool.hpp"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textCoords;
};

struct MeshData
{
	MeshData();
	MemoryHandle handle;
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int vertexCount;
};

using MeshDataHandle = MemoryHandle;

struct Mesh
{
	MemoryHandle handle;
	Mesh(const MemoryHandle h = MemoryHandle()) : handle(h) {};
};
