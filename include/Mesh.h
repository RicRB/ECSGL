#pragma once

struct Mesh
{
	unsigned int vertexID;
	unsigned int indexID;
	unsigned int count;
	Mesh(unsigned int vertexID = 0, unsigned int indexID = 0, unsigned int count = 0) 
		: vertexID(vertexID), indexID(indexID), count(count) {}
};