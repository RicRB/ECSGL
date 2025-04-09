#pragma once

#include "VertexBuffer.h"

class VertexBufferLayout;
class VertexArray
{
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();
	VertexArray(const VertexArray&) = delete;
	void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	void AddBuffer(const VertexBuffer* vb, const VertexBufferLayout* layout);
	void AddBuffer(const VertexBuffer& vb);
	void Bind() const;
	void UnBind() const;

};
class VertexArrayComponent
{
	unsigned int rendererID;
};