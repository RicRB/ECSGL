#include "Material.h"
/*
Material::~Material()
{
}

Material::Material(Shader& shader, const glm::vec3& diffuse = glm::vec3(1), const glm::vec3& specular = glm::vec3(1), const float shininess = 0.5f)
	: m_Shader(&shader), 
	m_Shininess(shininess)
{
	// create Material variable for diffuse
	m_Diffuse.mType = VariableType::VEC3;
	m_Diffuse.mVec3 = diffuse;

	// create Material variable for specular
	m_Specular.mType = VariableType::VEC3;
	m_Specular.mVec3 = specular;
}
// Add missing constructors
Material::Material(Shader& shader, Texture* diffuse, const glm::vec3& specular, const float shininess)
	: m_Shader(&shader),
	m_Shininess(shininess)
{
	// create Material variable for diffuse
	m_Diffuse.mType = VariableType::TEXTURE;
	m_Diffuse.mTexture = diffuse;

	// create Material variable for specular
	m_Specular.mType = VariableType::VEC3;
	m_Specular.mVec3 = specular;
}
Material::Material(Shader& shader, const glm::vec3& diffuse, Texture* specular, const float shininess)
	: m_Shader(&shader),
	m_Shininess(shininess)
{
		// create Material variable for diffuse
	m_Diffuse.mType = VariableType::VEC3;
	m_Diffuse.mVec3 = diffuse;

	// create Material variable for specular
	m_Specular.mType = VariableType::TEXTURE;
	m_Specular.mTexture = specular;
}

Material::Material(Shader& shader, Texture* diffuse, Texture* specular, const float shininess)
	: m_Shader(&shader),
	m_Shininess(shininess)
{
	// create Material variable for diffuse
	m_Diffuse.mType = VariableType::TEXTURE;
	m_Diffuse.mTexture = diffuse;

	// create Material variable for specular
	m_Specular.mType = VariableType::TEXTURE;
	m_Specular.mTexture = specular;
}
void Material::Bind(int& textureIndex)
{
	m_Shader->Bind();
	// Diffuse Uniform
	if (m_Diffuse.mType == VariableType::TEXTURE)
	{
		m_Shader->SetUniform1b("u_Material.diffuseIsMap", true);
		m_Diffuse.mTexture->Bind(textureIndex);
		textureIndex++;
	}
	else
	{
		m_Shader->SetUniform1b("u_Material.diffuseIsMap", false);
		m_Shader->SetUniform3f("u_Material.diffuse", m_Diffuse.mVec3.x, m_Diffuse.mVec3.y, m_Diffuse.mVec3.z);
	}
	// Specular Uniform
	if (m_Specular.mType == VariableType::TEXTURE)
	{
		m_Shader->SetUniform1b("u_Material.specularIsMap", true);
		m_Specular.mTexture->Bind(textureIndex);
		textureIndex++;
	}
	else
	{
		m_Shader->SetUniform1b("u_Material.specularIsMap", false);
		m_Shader->SetUniform3f("u_Material.specular", m_Specular.mVec3.x, m_Specular.mVec3.y, m_Specular.mVec3.z);
	}
	// texture
}

Shader& Material::GetShader()
{
	return *m_Shader;
}
*/
