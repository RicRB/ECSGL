#pragma once
#include <GL/glew.h>
#include "gl_def.h"
#include <vector>
struct VertexBufferElement
{
	unsigned int type;
	unsigned int count;
	unsigned int normalized;
	unsigned int offset;
	/*
	to do:
	1. instead of adding vertex, add glm::vec3() as push method
	2. find useable type for glVertexAttribPointer!!!!
	*/
	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
			case GL_FLOAT:			return 4;
			case GL_UNSIGNED_INT:	return 4;
			case GL_UNSIGNED_BYTE:	return 1;
		}
		ASSERT(false);
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferElement> m_Elements;
	unsigned int m_Stride;
public:
	VertexBufferLayout()
		:m_Stride(0) {}

	template<typename T>
	void Push(unsigned int count)
	{
	
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_Elements.push_back({GL_FLOAT, count, GL_FALSE} );
		m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
	}
	template<>
	void Push<Vertex>(unsigned int count)
	{
		m_Elements.push_back({ GL_FLOAT, 3 * count, GL_FALSE, offsetof(Vertex, position.x)});
		m_Elements.push_back({ GL_FLOAT, 3 * count, GL_FALSE, offsetof(Vertex, normal.x)});
		m_Elements.push_back({ GL_FLOAT, 2 * count, GL_FALSE, offsetof(Vertex, textCoords.x)});
		m_Stride += sizeof(Vertex) * count;
	}
	inline const std::vector<VertexBufferElement>& GetElements() const { return m_Elements; }
	inline unsigned int GetStride() const { return m_Stride;  }
};