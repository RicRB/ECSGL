#pragma once
#include "renderer/buffer.h"
#include <GL/glew.h>
using GLVertexID = GLuint;
using GLIndexID = GLuint;
using GLUniformID = GLuint;
struct GlVertexBuffer : public VertexBuffer {
	GLVertexID vertexID;
};
struct GlIndexBuffer : public IndexBuffer {
	GLIndexID indexID;
};

struct GlUniformBuffer : public UniformBuffer {
	GLUniformID uniformID;
};