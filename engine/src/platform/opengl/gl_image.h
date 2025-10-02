#pragma once
#include "renderer/image.h"
#include "GL/glew.h"

struct GlTextureImage : public TextureImage {
	GLuint textureID;
};