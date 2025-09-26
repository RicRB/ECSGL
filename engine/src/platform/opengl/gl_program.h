#pragma once
#include "renderer/program.h"
#include <GL/glew.h>

struct GlProgram : public Program {
	GLuint programID = 0;
};