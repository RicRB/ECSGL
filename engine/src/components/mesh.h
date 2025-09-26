#pragma once
#include <array>
#include <memory>
#include <vector>
#include "renderer/buffer.h"
#include <glm/glm.hpp>
using MeshID = unsigned int;

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textCoords;
};
struct Mesh
{
	MeshID id;
};
struct MeshData
{
	MeshData();
	MeshID id;
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int vertexCount;
};