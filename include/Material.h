#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H
#include "glm/glm.hpp"
#include "Shader.h"
#include "Texture.h"
enum class VariableType
{
	TEXTURE,
	VEC3
};
// FIXME: find a better thing to do here, waste of memory
struct MaterialVariable
{
	VariableType type;
	union
	{
		Texture texture;
		glm::vec3 vec3;
	};
	MaterialVariable(Texture texture)
		:type(VariableType::TEXTURE), texture(texture)
	{}
	MaterialVariable(const glm::vec3& vec3)
		:type(VariableType::VEC3), vec3(vec3)
	{}
};
//class MaterialOld
//{
//public:
//	MaterialVariable m_Diffuse, m_Specular;
//	float m_Shininess;
//	Shader *m_Shader;
//	MaterialOld(Shader& shader, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess);
//	MaterialOld(Shader& shader, Texture *diffuse, const glm::vec3& specular, const float shininess);
//	MaterialOld(Shader& shader, const glm::vec3& diffuse, Texture *specular, const float shininess);
//	MaterialOld(Shader& shader, Texture *diffuse, Texture *specular, const float shininess);
//
//	~MaterialOld();
//	void Bind(int& textureIndex);
//	Shader& GetShader();
//};

struct Material {
	unsigned int shaderID;
	MaterialVariable diffuse, specular;
	float shininess;
	// constructors
	// TODO: make default shader
	Material()
		: shaderID(0), diffuse(glm::vec3(1.0f)), specular(glm::vec3(1.0f)), shininess(32.0f) {}
	Material(unsigned int shaderID)
		: shaderID(shaderID), diffuse(glm::vec3(1.0f)), specular(glm::vec3(1.0f)), shininess(32.0f) {}
	Material(unsigned int shaderID, const MaterialVariable& diffuse, const MaterialVariable& specular, const float shininess)
		:shaderID(shaderID), diffuse(diffuse), specular(specular), shininess(shininess) {}
	Material(unsigned int shader, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess)
		:shaderID(shader), diffuse(diffuse), specular(specular), shininess(shininess) {}
	Material(unsigned int shader, Texture diffuse, const glm::vec3& specular, const float shininess)
		:shaderID(shader), diffuse(diffuse), specular(specular), shininess(shininess) {}
	Material(unsigned int shader, const glm::vec3& diffuse, Texture specular, const float shininess)
		:shaderID(shader), diffuse(diffuse), specular(specular), shininess(shininess) {}
	Material(unsigned int shader, Texture diffuse, Texture specular, const float shininess)
		:shaderID(shader), diffuse(diffuse), specular(specular), shininess(shininess) {}
};
#endif