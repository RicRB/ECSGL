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
private:
	std::unordered_map<std::string, ShaderData> mShaders;
	std::unordered_map<unsigned int, std::unordered_map<std::string, int>> mUniformLocationCache;
	std::unordered_map<unsigned int, std::unordered_map<std::string, GLenum>> mUniforms;
	std::unordered_map<std::string, TextureData> mTextures;
	ResourceManager() = default;
	~ResourceManager() = default;
public: 
	static Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	static Texture LoadTexture(const std::string& path);
	static Entity LoadEntityFromOBJ(const std::string& filepath);
	static Scene ParseScene(const std::string& shaderPath);
	static std::string& SaveScene();
	static void InitShaders();
	static int GetUniformLocation(const unsigned int rendererID, const std::string& name);
private:
	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	// static instance
	static ResourceManager& GetInstance()
	{
		static ResourceManager instance;
		return instance;
	}

	// SHADER STUFFS
	static void ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment);
	static void CompileUniforms(const unsigned int rendererID);
	static unsigned int CompileShader(unsigned int type, const std::string& source);
	static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	// ENTITY LOADING
	static Mesh ProcessMesh(std::fstream& fstream);

};