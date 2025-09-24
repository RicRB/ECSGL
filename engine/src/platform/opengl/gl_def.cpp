#include "gl_def.h"
// Clears errors
void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}
// Returns whether it succeeded or nah, while loop shouldnt rly exist but im not gon bother
// use this when calling openGL functions to see if you're fucking stupid or not
bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}