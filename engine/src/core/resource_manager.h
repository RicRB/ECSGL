#pragma once
#include "components/material.h"
#include "scene.h"
#include "components/mesh.h"
#include "ecs/ecs.hpp"
#include <unordered_map>
#include <fstream>
#include <string>
#include "memory/memory_pool.hpp"
class ResourceManager
{
public:
	ResourceManager() = default;
	~ResourceManager() = default;
	std::unique_ptr<MemoryPool<ShaderData, 32>> m_shaders = nullptr;
	std::unique_ptr<MemoryPool<MaterialData, 32>> m_materials = nullptr;
	std::unique_ptr<MemoryPool<MeshData, 32>> m_meshes = nullptr;
private:
	// TODO: add a map string -> ShaderID to avoid copies
	std::unordered_map<ShaderID, std::unordered_map<std::string, int>> m_uniformLocationCache;
	std::unordered_map<ShaderID, std::unordered_map<std::string, GLenum>> m_uniforms;
	std::unordered_map<std::string, TextureData> m_textures;

public: 
	void CreateResourcePools();
	Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Material LoadMaterial(const ShaderID shaderID);
	Texture LoadTexture(const std::string& path);
	Mesh LoadOBJ(const std::string& filepath);
	Scene ParseScene(const std::string& shaderPath);
	std::string& SaveScene();
	int GetUniformLocation(const unsigned int rendererID, const std::string& name);
	void unloadAll();

	void bindTextureToMaterial(const Material& material, const Texture& texture, uint32_t binding);
private:

	// SHADER STUFFS
	void ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment);
	void CompileUniforms(const unsigned int rendererID);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	// ENTITY LOADING
	Mesh ProcessMesh(std::fstream& fstream);

};