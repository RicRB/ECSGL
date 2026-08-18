#pragma once
#include "renderer/image.h"
#include "GL/glew.h"

struct GlImage : public Image {
	GLuint textureID = 0;
	GLenum target = GL_TEXTURE_2D;        // GL_TEXTURE_2D / GL_TEXTURE_3D
	GLenum internalFormat = GL_RGBA8;     // sized internal format (for image binding / storage)
};