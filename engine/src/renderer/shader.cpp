#include "shader.h"
#include "core/engine.h"
#include "platform/opengl/gl_def.h"
#include "platform/opengl/gl_program.h"
#include "platform/vulkan/vk_program.h"

ShaderData::ShaderData()
{
	program = Program::Create();
}
/*
void shader::SetUniform1b(const unsigned int shaderID, const std::string& name, bool value)
{
	GLCall(glUniform1i(shader::GetShaderUniformLocation(shaderID, name), value));
}
void shader::SetUniform1i(const unsigned int shaderID, const std::string& name, int value)
{
	GLCall(glUniform1i(shader::GetShaderUniformLocation(shaderID, name), value));
}
void shader::SetUniform1f(const unsigned int shaderID, const std::string& name, float value)
{
	GLCall(glUniform1f(shader::GetShaderUniformLocation(shaderID, name), value));
}
void shader::SetUniform3f(const unsigned int shaderID, const std::string& name, float v0, float v1, float v2)
{
	GLCall(glUniform3f(shader::GetShaderUniformLocation(shaderID, name), v0, v1, v2));
}
void shader::SetUniform4f(const unsigned int shaderID, const std::string& name, float v0, float v1, float v2, float v3)
{
	GLCall(glUniform4f(shader::GetShaderUniformLocation(shaderID, name), v0, v1, v2, v3));
}
void shader::SetUniform4f(const unsigned int shaderID, const std::string& name, glm::vec4 vec)
{
	GLCall(glUniform4f(shader::GetShaderUniformLocation(shaderID, name), vec.x, vec.y, vec.z, vec.a));
}
void shader::SetUniformMat3f(const unsigned int shaderID, const std::string& name, glm::mat3& matrix)
{
	GLCall(glUniformMatrix3fv(shader::GetShaderUniformLocation(shaderID, name), 1, GL_FALSE, &matrix[0][0]));
}
void shader::SetUniformMat4f(const unsigned int shaderID, const std::string& name, const glm::mat4& matrix)
{
	GLCall(glUniformMatrix4fv(shader::GetShaderUniformLocation(shaderID, name), 1, GL_FALSE, &matrix[0][0]));
}
*/
