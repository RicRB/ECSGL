#pragma once
#include "renderer/buffer.h"
#include <GL/glew.h>
using GLVertexID = GLuint;
using GLIndexID = GLuint;
using GLUniformID = GLuint;
using GLComputeID = GLuint;


struct GlBuffer : public BufferData {
	GLuint bufferID;
};

struct GlVertexBuffer : public VertexBuffer, public GlBuffer {
	GLVertexID vertexID;
};
struct GlIndexBuffer : public IndexBuffer, public GlBuffer {
	GLIndexID indexID;
};

struct GlUniformBuffer : public UniformBuffer, public GlBuffer {
	GLUniformID uniformID;
};

struct GlComputeBuffer : public ComputeBuffer, public GlBuffer {
};