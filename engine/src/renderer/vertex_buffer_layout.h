#pragma once
#include "platform/opengl/gl_def.h"
#include "components/mesh.h"
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
	std::vector<VertexBufferElement> m_elements;
	unsigned int m_stride;
public:
	VertexBufferLayout()
		:m_stride(0) {}

	template<typename T>
	void Push(unsigned int count)
	{
	
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_elements.push_back({GL_FLOAT, count, GL_FALSE} );
		m_stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
	}
	template<>
	void Push<Vertex>(unsigned int count)
	{
		m_elements.push_back({ GL_FLOAT, 3 * count, GL_FALSE, offsetof(Vertex, position.x)});
		m_elements.push_back({ GL_FLOAT, 3 * count, GL_FALSE, offsetof(Vertex, normal.x)});
		m_elements.push_back({ GL_FLOAT, 2 * count, GL_FALSE, offsetof(Vertex, textCoords.x)});
		m_stride += sizeof(Vertex) * count;
	}
	inline const std::vector<VertexBufferElement>& GetElements() const { return m_elements; }
	inline unsigned int GetStride() const { return m_stride;  }
};