#pragma once
#include "components/material.h"
#include "scene.h"
#include "components/mesh.h"
#include "renderer/compute_shader.h"
#include "renderer/texture.h"
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
	std::unique_ptr<MemoryPool<ComputeShaderData, 32>> m_computeShaders = nullptr;
	std::unique_ptr<MemoryPool<MaterialData, 32>> m_materials = nullptr;
	std::unique_ptr<MemoryPool<MeshData, 32>> m_meshes = nullptr;
	std::unique_ptr<MemoryPool<TextureData, 32>> m_textures = nullptr;
private:
	// TODO: add a map string -> ShaderID to avoid copies
	std::unordered_map<std::string, MemoryHandle> m_textureHandles;

public: 
	void CreateResourcePools();
	Shader LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, ShaderCreateInfo& info);
	ComputeShader LoadComputeShader(const std::string& computeShaderPath);
	Material LoadMaterial(const Shader shader);
	Texture LoadTexture(const std::string& path, TextureUsage usage);
	Texture LoadEmptyTexture(TextureCreateInfo& createInfo);
	Mesh LoadOBJ(const std::string& filepath);
	Scene ParseScene(const std::string& shaderPath);
	std::string& SaveScene();
	void unloadAll();

	void bindTextureToMaterial(const Material& material, const Texture& texture, uint32_t binding);
	void setTextureUsage(const Texture texture, TextureUsage newUsage);
private:

	// SHADER STUFFS
	void ParseShader(std::string& parseDestination, const std::string& shaderPath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	// ENTITY LOADING
	Mesh ProcessMesh(std::fstream& fstream);
};