#include "resource_manager.h"
#include <sstream>
#include "stb_image/stb_image.h"
#include "components/lights.h"
#include "renderer/vertex_buffer_layout.h"
#include "core/engine.h"

void ResourceManager::CreateResourcePools()
{
	m_shaders = std::make_unique<MemoryPool<ShaderData, MAX_RESOURCE_POOL_SIZE>>();
	m_computeShaders = std::make_unique<MemoryPool<ComputeShaderData, MAX_RESOURCE_POOL_SIZE>>();
	m_materials = std::make_unique<MemoryPool<MaterialData, MAX_RESOURCE_POOL_SIZE>>();
	m_meshes = std::make_unique<MemoryPool<MeshData, MAX_RESOURCE_POOL_SIZE>>();
	m_textures = std::make_unique<MemoryPool<TextureData, MAX_RESOURCE_POOL_SIZE>>();
}

Shader ResourceManager::LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, ShaderCreateInfo& info)
{
    MemoryHandle shaderHandle = m_shaders->allocate();
	ShaderData& shader = m_shaders->at(shaderHandle);
	int vertexHash, fragmentHash;
    ParseShader(shader.vertexSource, vertexShaderPath);
	ParseShader(shader.fragmentSource, fragmentShaderPath);

	// combine hashes
	std::hash<std::string> hasher;
	shader.sourceHash = hasher(shader.vertexSource + shader.fragmentSource);
    shader.alwaysRender = info.alwaysRender;

    graphics::GetGraphicsAPI()->compileShader(
		std::to_string(static_cast<uint32_t>(shaderHandle)).c_str(),
		shader.vertexSource,
		shader.fragmentSource,
		&shader
	);

	return Shader(shaderHandle);
}

ComputeShader ResourceManager::LoadComputeShader(const std::string& computeShaderPath)
{
    MemoryHandle computeShaderHandle = m_computeShaders->allocate();
    ComputeShaderData& computeShader = m_computeShaders->at(computeShaderHandle);
    std::string line;
    std::stringstream ss;
    // Compute
    std::ifstream computeStream(computeShaderPath);
    while (std::getline(computeStream, line))
    {
        ss << line << '\n';
    }
    computeStream.close();
    computeShader.source = ss.str();
    std::hash<std::string> hasher;
    computeShader.sourceHash = hasher(computeShader.source);

    graphics::GetGraphicsAPI()->compileComputeShader(
        "main",
        &computeShader
	);
	return ComputeShader(computeShaderHandle);
}

Material ResourceManager::LoadMaterial(const Shader shader)
{
    MaterialDataHandle materialHandle = m_materials->allocate();
    MaterialData& material = (*m_materials)[materialHandle];
	material.shaderHandle = shader.handle;
    material.ubo.ambientColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    material.ubo.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.ubo.specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    graphics::GetGraphicsAPI()->loadMaterial(
        std::to_string((uint32_t)materialHandle).c_str(),
        m_shaders->at(shader.handle),
        &material
    );

    return Material(materialHandle);
}

Texture ResourceManager::LoadTexture(const std::string& path, TextureUsage usage)
{
    {
        auto textureHandleResult = m_textureHandles.find(path);
        if (textureHandleResult != m_textureHandles.end())
        {
            return Texture(textureHandleResult->second);
        }
    }

    auto textureHandle = m_textures->allocate();
	m_textureHandles.emplace(path, textureHandle);
    auto& textureData = m_textures->at(textureHandle);
    stbi_set_flip_vertically_on_load(1); // play with it if upside down
    textureData.pixels = stbi_load(path.c_str(), &textureData.width, &textureData.height, &textureData.bpp, 4);
    textureData.usage = usage;
    textureData.depth = 1;
    if (textureData.pixels)
        graphics::GetGraphicsAPI()->loadTexture(&textureData);

	if (textureData.pixels)
		stbi_image_free(textureData.pixels);
    return Texture(textureHandle);
}

Texture ResourceManager::LoadEmptyTexture(TextureCreateInfo& createInfo)
{
    auto textureHandle = m_textures->allocate();
    auto& textureData = m_textures->at(textureHandle);

    int depth = createInfo.depth > 0 ? createInfo.depth : 1; // in case im dumb and forget lol

    textureData.width = createInfo.width;
    textureData.height = createInfo.height;
    textureData.depth = depth;
    textureData.bpp = 4;
    int size = createInfo.width * createInfo.height * depth * 4;
    textureData.pixels = new unsigned char[size];

    memset(textureData.pixels, 0, size);
    graphics::GetGraphicsAPI()->loadTexture(&textureData);
    delete[] textureData.pixels;
	return Texture(textureHandle);
}

Mesh ResourceManager::LoadOBJ(const std::string& filepath)
{
    std::fstream fstream(filepath);
    Mesh meshComponent = ProcessMesh(fstream);
    return meshComponent;
}
Scene ResourceManager::ParseScene(const std::string& shaderPath)
{
    return Scene();
}

void ResourceManager::unloadAll()
{
	for (auto& shader : *m_shaders)
	{
		graphics::GetGraphicsAPI()->unloadShader(&shader);
	}
	for (auto& material : *m_materials)
	{
        graphics::GetGraphicsAPI()->unloadMaterial(&material);
	}
	for (auto& mesh : *m_meshes)
	{
        graphics::GetGraphicsAPI()->unloadMesh(&mesh);
	}
	for (auto& texture : *m_textures)
	{
        graphics::GetGraphicsAPI()->unloadTexture(&texture);
	}

    m_shaders->clear();
	m_materials->clear();
	m_meshes->clear();
	m_textures->clear();
}

void ResourceManager::bindTextureToMaterial(const Material& material, const Texture& texture, uint32_t binding)
{
	auto& textureData = m_textures->at(texture.handle);
    graphics::GetGraphicsAPI()->bindTextureToMaterial(&m_materials->at(material.handle), &m_textures->at(texture.handle), binding);
	MaterialData& matData = m_materials->at(material.handle);
    switch (binding)
    {
	case(ALBEDO_MAP_BINDING):
		matData.albedoMap = texture;
		break;
	case(SPECULAR_MAP_BINDING):
		matData.specularMap = texture;
		break;
	case(NORMAL_MAP_BINDING):
		matData.normalMap = texture;
		break;
	default:
		std::cout << "Warning: trying to bind texture to invalid binding point " << binding << "!\n";
    }
}

void ResourceManager::setTextureUsage(const Texture texture, TextureUsage newUsage)
{
    auto& textureData = m_textures->at(texture.handle);
    graphics::GetGraphicsAPI()->setTextureUsage(&textureData, newUsage);
    
}

/////////////////////// PRIVATE ///////////////////////


/////////////////////////////////////////
// SHADER STUFF /////////////////////////
/////////////////////////////////////////
void ResourceManager::ParseShader(std::string& parseDestination, const std::string& shaderPath)
{
    std::string line;
    std::stringstream ss[2];
    // Vertex
    std::ifstream vertexStream(shaderPath);
    while (std::getline(vertexStream, line))
    {
        ss[0] << line << '\n';
    }
    vertexStream.close();
    
    parseDestination = ss[0].str();
}

unsigned int ResourceManager::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }


    return id;
}



Mesh ResourceManager::ProcessMesh(std::fstream& fstream)
{
    std::string currentLine;

	std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textCoords;
    while (std::getline(fstream, currentLine))
    {
		if (currentLine.empty() || currentLine[0] == '#') continue;

        // if vertex pos
		if (currentLine.compare(0, 2, "v ") == 0)
		{
			std::istringstream ss(currentLine.substr(2));
            glm::vec3 vertexPos;
			ss >> vertexPos.x >> vertexPos.y >> vertexPos.z;
			positions.push_back(vertexPos);
		}
        // if vertex normal
        else if (currentLine.compare(0, 3, "vn ") == 0)
        {
			std::istringstream ss(currentLine.substr(3));
			glm::vec3 vertexNormal;
			ss >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;
			normals.push_back(vertexNormal);
        }
		// if vertex texture coords
        else if (currentLine.compare(0, 3, "vt ") == 0)
        {
            std::istringstream ss(currentLine.substr(3));
			glm::vec2 vertexTextCoords;
			ss >> vertexTextCoords.x >> vertexTextCoords.y;
			textCoords.push_back(vertexTextCoords);
		}
    }
    // compile all arrays into vertices with indices
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    fstream.clear();
	fstream.seekg(0, std::ios::beg);
    while (std::getline(fstream, currentLine))
    {
        if (currentLine.compare(0, 2, "f ") == 0)
		{
			std::istringstream ss(currentLine.substr(2));
            std::string vertexData;
            //std::array<uint32_t, 3> triangle;
            for (int i = 0; i < 3; i++)
            {
                ss >> vertexData;
                std::istringstream vs(vertexData);
                std::string v, vt, vn;

                std::getline(vs, v, '/');
                std::getline(vs, vt, '/');
                std::getline(vs, vn, '/');

                unsigned int vertexIndex = std::stoi(v);
                unsigned int texIndex = std::stoi(vt);
                unsigned int normIndex = std::stoi(vn);

                Vertex vertex;
				vertex.position = positions[vertexIndex-1];
				vertex.normal = normals[normIndex-1];
				vertex.textCoords = textCoords[texIndex-1];
				vertices.push_back(vertex);
				indices.push_back(static_cast<unsigned int>(vertices.size() - 1)); // store index of the vertex
				//triangle[i] = static_cast<unsigned int>(vertices.size() - 1); // store index of the vertex
            }
            // Reverse winding: v0, v2, v1 instead of v0, v1, v2
            //indices.push_back(triangle[0]);
            //indices.push_back(triangle[2]);
            //indices.push_back(triangle[1]);
		}
    }
    MemoryHandle meshDataHandle = m_meshes->allocate();
	MeshData& meshData = (*m_meshes)[meshDataHandle];
	meshData.vertices = vertices;
	meshData.indices = indices;
    meshData.vertexCount = static_cast<unsigned int>(vertices.size());
	meshData.handle = meshDataHandle;
    graphics::GetGraphicsAPI()->loadMesh(&meshData);
	Mesh meshComponent;
	meshComponent.handle = meshDataHandle;
    return meshComponent;
}

/*  THIS IS HOW TO SAVE MESH ON OPENGL
    // generate vertex Buffer
    unsigned int vertexBuffer;
    GLCall(glGenBuffers(1, &vertexBuffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));
    // generate vertex array
    unsigned int vertexArray;
    GLCall(glGenVertexArrays(1, &vertexArray));

    // generate buffer layout
    VertexBufferLayout layout;
    layout.Push<Vertex>(1);
    // ADD BUFFER TO ARRAY
    GLCall(glBindVertexArray(vertexArray));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    const auto& elements = layout.GetElements();
    GLCall(glEnableVertexAttribArray(0));
    for (unsigned int i = 0; i < elements.size(); i++)
    {
        const auto& element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(), (const void*)element.offset));
    }
    GLCall(glBindVertexArray(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // CREATE INDEX BUFFER
    unsigned int indexBuffer;
    GLCall(glGenBuffers(1, &indexBuffer));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW));

    Mesh meshComponent(vertexArray, indexBuffer, indices.size());
*/

/////////////////////////////////////////
// TEXTURE STUFF ////////////////////////
/////////////////////////////////////////
