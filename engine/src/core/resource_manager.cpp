#include "resource_manager.h"
#include <sstream>
#include "stb_image/stb_image.h"
#include "components/lights.h"
#include "renderer/vertex_buffer_layout.h"
#include "core/engine.h"
ResourceManager::ResourceManager()
{
	if (!initialized)
	{
		initialized = true;
		// Initialize shaders
		m_shaders.reserve(256); // reserve space for 256 shaders to avoid reallocations
		m_textures.reserve(256); // reserve space for 256 textures to avoid reallocations
		m_materials.reserve(256); // reserve space for 256 materials to avoid reallocations
		m_meshes.reserve(256); // reserve space for 256 meshes to avoid reallocations
	}
}

Shader ResourceManager::LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    Shader returnShader;
	std::cout << m_shaders.size() << " shaders loaded\n";
    ShaderData& shader = m_shaders.emplace_back();
	shader.id = static_cast<ShaderID>(m_shaders.size() - 1); // Assign a unique ID based on the index
    returnShader.id = shader.id;
	returnShader.data = &shader;
    ParseShader(shader, vertexShaderPath, fragmentShaderPath);

	engine::currentEngine->renderer.graphicsAPI->compileShader(
		std::to_string(shader.id).c_str(),
		shader.VertexSource,
		shader.FragmentSource,
		&shader
	);

	return returnShader;
}

Material ResourceManager::LoadMaterial(const ShaderID shaderID)
{
	Material returnMaterial;
	MaterialData& material = m_materials.emplace_back();
	material.shaderID = shaderID;
	material.id = static_cast<MaterialID>(m_materials.size() - 1); // Assign a unique ID based on the index
	material.ubo.ambientColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	material.ubo.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	material.ubo.specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	returnMaterial.id = material.id;

	engine::currentEngine->renderer.graphicsAPI->loadMaterial(
		std::to_string(material.id).c_str(),
		m_shaders[shaderID],
		&material
	);

    return returnMaterial;
}

Texture ResourceManager::LoadTexture(const std::string& path)
{
    auto textureInfo = m_textures.find(path);
    if (textureInfo != m_textures.end())
    {
        Texture tex;
		tex.id = textureInfo->second.id;
        return tex;
    }
    auto it = m_textures.emplace(path, TextureData()).first;
    auto& texInfo = it->second;
	texInfo.id = it->first;
    stbi_set_flip_vertically_on_load(1); // play with it if upside down
    texInfo.pixels = stbi_load(path.c_str(), &texInfo.width, &texInfo.height, &texInfo.bpp, 4);

    if (texInfo.pixels)
		engine::currentEngine->renderer.graphicsAPI->loadTexture(&texInfo);

	if (texInfo.pixels)
		stbi_image_free(texInfo.pixels);
    Texture tex(it->first);
    return tex;
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

int ResourceManager::GetUniformLocation(const unsigned int rendererID, const std::string& name)
{
    if (m_uniformLocationCache[rendererID].find(name) != m_uniformLocationCache[rendererID].end())
        return m_uniformLocationCache[rendererID][name];
    else
        printf("Warning: uniform '%s' doesn't exist!\n", name.c_str());

    return -1;
}

void ResourceManager::unloadAll()
{
	for (auto& shader : m_shaders)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadShader(&shader);
	}
	for (auto& material : m_materials)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadMaterial(&material);
	}
	for (auto& mesh : m_meshes)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadMesh(&mesh);
	}
	for (auto& texture : m_textures)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadTexture(&texture.second);
	}
    // TODO: TEXTURES

    m_shaders.clear();
	m_materials.clear();
	m_meshes.clear();
	m_textures.clear();
}

void ResourceManager::bindTextureToMaterial(const Material& material, const Texture& texture, uint32_t binding)
{
	auto& texIt = m_textures[texture.id.data()];
	engine::currentEngine->renderer.graphicsAPI->bindTextureToMaterial(&m_materials[material.id], &m_textures[texture.id.data()], binding);
	MaterialData& matData = m_materials[material.id];
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

/////////////////////// PRIVATE ///////////////////////


/////////////////////////////////////////
// SHADER STUFF /////////////////////////
/////////////////////////////////////////

void ResourceManager::ParseShader(ShaderData& shader, const std::string& vertex, const std::string& fragment)
{
    std::string line;
    std::stringstream ss[2];
    // Vertex
    std::ifstream vertexStream(vertex);
    while (std::getline(vertexStream, line))
    {
        ss[0] << line << '\n';
    }
    vertexStream.close();
    // Fragment
    std::ifstream fragmentStream(fragment);
    while (getline(fragmentStream, line))
    {
        ss[1] << line << '\n';
    }
    fragmentStream.close();
    shader.VertexSource = ss[0].str();
    shader.FragmentSource = ss[1].str();
}

void ResourceManager::CompileUniforms(const unsigned int programID)
{
    GLint i;
    GLint count; // amount of uniforms

    GLint size; // sizeS of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 32; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &count);
    //printf("Active Uniforms: %d\n", count);

    for (i = 0; i < count; i++)
    {
        glGetActiveUniform(programID, (GLuint)i, bufSize, &length, &size, &type, name);
        int location = glGetUniformLocation(programID, name);
        m_uniformLocationCache[programID].insert(std::pair<std::string, int>(name, location));
        m_uniforms[programID].insert(std::pair<std::string, GLenum>(name, type));
        //printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }
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



unsigned int ResourceManager::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);
    CompileUniforms(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
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
    MeshData& meshData = m_meshes.emplace_back();
	meshData.vertices = vertices;
	meshData.indices = indices;
    meshData.vertexCount = static_cast<unsigned int>(vertices.size());
	meshData.id = static_cast<MeshID>(m_meshes.size() - 1); // TODO : UUID????
	engine::currentEngine->renderer.graphicsAPI->loadMesh(&meshData);
	Mesh meshComponent;
	meshComponent.id = meshData.id;
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
