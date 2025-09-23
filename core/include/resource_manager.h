#pragma once
#include "material.h"
#include "scene.h"
#include "mesh.h"
#include "ecs/ecs.hpp"
#include <unordered_map>
#include <fstream>
#include <string>

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager() = default;
	std::vector<ShaderData> mShaders;
	std::vector<MaterialData> mMaterials;
	std::vector<MeshData> mMeshes;
private:
	bool initialized = false;

	// TODO: add a map string -> ShaderID to avoid copies
	std::unordered_map<ShaderID, std::unordered_map<std::string, int>> mUniformLocationCache;
	std::unordered_map<ShaderID, std::unordered_map<std::string, GLenum>> mUniforms;
	std::unordered_map<std::string, TextureData> mTextures;

public: 
	Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Material LoadMaterial(const ShaderID shaderID);
	Texture LoadTexture(const std::string& path);
	Mesh LoadOBJ(const std::string& filepath);
	Scene ParseScene(const std::string& shaderPath);
	std::string& SaveScene();
	int GetUniformLocation(const unsigned int rendererID, const std::string& name);
	void unloadAll();
private:

	// SHADER STUFFS
	void ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment);
	void CompileUniforms(const unsigned int rendererID);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	// ENTITY LOADING
	Mesh ProcessMesh(std::fstream& fstream);

};