#pragma once
#include "renderer/image.h"
#include "GL/glew.h"

struct GlImage : public Image {
	GLuint textureID;
};