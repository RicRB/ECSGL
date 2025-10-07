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
	static const uint32_t MAX_RESOURCE_POOL_SIZE = 32;
	std::unique_ptr<MemoryPool<ShaderData, 32>> m_shaders = nullptr;
	std::unique_ptr<MemoryPool<MaterialData, 32>> m_materials = nullptr;
	std::unique_ptr<MemoryPool<MeshData, 32>> m_meshes = nullptr;
	std::unique_ptr<MemoryPool<TextureData, 32>> m_textures = nullptr;
private:
	// TODO: add a map string -> ShaderID to avoid copies
	std::unordered_map<std::string, MemoryHandle<TextureData>> m_textureHandles;

public: 
	void CreateResourcePools();
	Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	Material LoadMaterial(const Shader shader);
	Texture LoadTexture(const std::string& path);
	Mesh LoadOBJ(const std::string& filepath);
	Scene ParseScene(const std::string& shaderPath);
	std::string& SaveScene();
	void unloadAll();

	void bindTextureToMaterial(const Material& material, const Texture& texture, uint32_t binding);
private:

	// SHADER STUFFS
	void ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	// ENTITY LOADING
	Mesh ProcessMesh(std::fstream& fstream);
};