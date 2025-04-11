#pragma once
//class Mesh {
//private:
//	VertexArray *m_Vertices;
//	IndexBuffer *m_Index;
//
//public:
//	Mesh(VertexArray& va, IndexBuffer& ib);
//	~Mesh();
//	//Mesh(const Mesh&) = delete;
//	unsigned int GetVertexCount() const { return m_Index->GetCount(); }
//	void Bind() const;
//	void UnBind() const;
//private:
//};

struct Mesh
{
	unsigned int vertexID;
	unsigned int indexID;
	unsigned int count;
	Mesh(unsigned int vertexID = 0, unsigned int indexID = 0, unsigned int count = 0) 
		: vertexID(vertexID), indexID(indexID), count(count) {}
};