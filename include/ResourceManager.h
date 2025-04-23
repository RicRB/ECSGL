#pragma once
#include "Shader.h"
#include "Texture.h"
#include "Scene.h"
#include "Mesh.h"
#include "ECS/ECS.hpp"
#include <unordered_map>
#include <fstream>
#include <string>
class ResourceManager
{
public:
	ResourceManager() = default;
	~ResourceManager() = default;
private:
	std::unordered_map<std::string, ShaderData> mShaders;
	std::unordered_map<unsigned int, std::unordered_map<std::string, int>> mUniformLocationCache;
	std::unordered_map<unsigned int, std::unordered_map<std::string, GLenum>> mUniforms;
	std::unordered_map<std::string, TextureData> mTextures;

public: 
	Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Texture LoadTexture(const std::string& path);
	Mesh LoadOBJ(const std::string& filepath);
	Scene ParseScene(const std::string& shaderPath);
	std::string& SaveScene();
	void InitShaders(Registry& registry);
	int GetUniformLocation(const unsigned int rendererID, const std::string& name);
private:

	// SHADER STUFFS
	void ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment);
	void CompileUniforms(const unsigned int rendererID);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	// ENTITY LOADING
	Mesh ProcessMesh(std::fstream& fstream);

};