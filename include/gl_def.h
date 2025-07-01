#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))


#define GLC_VERTEX 2023
// Clears errors
void GLClearError();
// Returns whether it succeeded or nah, while loop shouldnt rly exist but im not gon bother
// use this when calling openGL functions to see if you're fucking stupid or not
bool GLLogCall(const char* function, const char* file, int line);
