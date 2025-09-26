#include "mesh.h"

MeshData::MeshData()
{
	vertexBuffer = VertexBuffer::Create();
	indexBuffer = IndexBuffer::Create();
}
