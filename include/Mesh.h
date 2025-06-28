#pragma once
#include "vk_def.h"
struct Mesh
{
	unsigned int vertexID;
	unsigned int indexID;
	uint32_t vertexCount;

	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	Mesh(unsigned int vertexID = 0, unsigned int indexID = 0, unsigned int count = 0) 
		: vertexID(vertexID), indexID(indexID), vertexCount(count) {}
};