#include "resource_manager.h"
#include <sstream>
#include "stb_image/stb_image.h"
#include "lights.h"
#include "gl_def.h"
#include "vertex_buffer_layout.h"
#include "engine.h"
ResourceManager::ResourceManager()
{
	if (!initialized)
	{
		initialized = true;
		// Initialize shaders
		mShaders.reserve(256); // reserve space for 256 shaders to avoid reallocations
		mTextures.reserve(256); // reserve space for 256 textures to avoid reallocations
		mMaterials.reserve(256); // reserve space for 256 materials to avoid reallocations
		mMeshes.reserve(256); // reserve space for 256 meshes to avoid reallocations
	}
}

Shader ResourceManager::LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    Shader returnShader;
	std::cout << mShaders.size() << " shaders loaded\n";
    ShaderData& shader = mShaders.emplace_back();
	shader.id = static_cast<ShaderID>(mShaders.size() - 1); // Assign a unique ID based on the index
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
	MaterialData& material = mMaterials.emplace_back();
	material.shaderID = shaderID;
	material.id = static_cast<MaterialID>(mMaterials.size() - 1); // Assign a unique ID based on the index
	material.ubo.ambientColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	material.ubo.diffuseColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	material.ubo.specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	returnMaterial.id = material.id;

	engine::currentEngine->renderer.graphicsAPI->loadMaterial(
		std::to_string(material.id).c_str(),
		mShaders[shaderID],
		&material
	);

    return returnMaterial;
}

Texture ResourceManager::LoadTexture(const std::string& path)
{
    auto textureInfo = mTextures.find(path);
    if (textureInfo != mTextures.end())
    {
        return Texture(textureInfo->second.mRendererID);
    }
    TextureData texInfo;
    stbi_set_flip_vertically_on_load(1); // play with it if upside down
    texInfo.mLocalBuffer = stbi_load(path.c_str(), &texInfo.mWidth, &texInfo.mHeight, &texInfo.mBPP, 4);
    GLCall(glGenTextures(1, &texInfo.mRendererID));
    GLCall(glBindTexture(GL_TEXTURE_2D, texInfo.mRendererID));

    // you need to specify these or else black texture
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // give data
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texInfo.mWidth, texInfo.mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texInfo.mLocalBuffer));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));

    if (texInfo.mLocalBuffer)
        stbi_image_free(texInfo.mLocalBuffer);

    mTextures[path] = texInfo;
    return Texture(texInfo.mRendererID);
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
    if (mUniformLocationCache[rendererID].find(name) != mUniformLocationCache[rendererID].end())
        return mUniformLocationCache[rendererID][name];
    else
        printf("Warning: uniform '%s' doesn't exist!\n", name.c_str());

    return -1;
}

void ResourceManager::unloadAll()
{
	for (auto& shader : mShaders)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadShader(&shader);
	}
	for (auto& material : mMaterials)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadMaterial(&material);
	}
	for (auto& mesh : mMeshes)
	{
		engine::currentEngine->renderer.graphicsAPI->unloadMesh(&mesh);
	}
    // TODO: TEXTURES

    mShaders.clear();
	mMaterials.clear();
	mMeshes.clear();
	mTextures.clear();
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
        mUniformLocationCache[programID].insert(std::pair<std::string, int>(name, location));
        mUniforms[programID].insert(std::pair<std::string, GLenum>(name, type));
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
				vertex.position = positions[vertexIndex - 1];
				vertex.normal = normals[normIndex - 1];
				vertex.textCoords = textCoords[texIndex - 1];
				vertices.push_back(vertex);
				indices.push_back(vertices.size() - 1);
            }
		}
    }
    MeshData& meshData = mMeshes.emplace_back();
	meshData.vertices = vertices;
	meshData.indices = indices;
    meshData.vertexCount = static_cast<unsigned int>(vertices.size());
	meshData.id = static_cast<MeshID>(mMeshes.size() - 1); // TODO : UUID????
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
