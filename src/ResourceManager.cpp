#include "ResourceManager.h"
#include <sstream>
#include "stb_image/stb_image.h"
#include "Lights.h"
#include "GLDef.h"
#include "VertexBufferLayout.h"

Shader ResourceManager::LoadShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    ResourceManager::GetInstance().mShaders[vertexShaderPath + fragmentShaderPath] = ShaderData{};
    ShaderData& shader = ResourceManager::GetInstance().mShaders[vertexShaderPath + fragmentShaderPath];
    Shader returnShader;
    returnShader.rendererID = &shader.rendererID;
	ParseShader(shader ,vertexShaderPath, fragmentShaderPath);
	return returnShader;
}

Texture ResourceManager::LoadTexture(const std::string& path)
{
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

    GetInstance().mTextures[path] = texInfo;
    return Texture(texInfo.mRendererID);
}

Entity ResourceManager::LoadEntityFromOBJ(const std::string& filepath)
{
    Registry& reg = Registry::GetInstance();
    Entity mainEntity = reg.CreateEntity();

    Assimp::Importer import;
    const aiScene * scene = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return Entity();
    }
    //GetInstance().mModels[filepath.substr(0, filepath.find_last_of('/'))].path = filepath;
    
    ProcessNode(scene->mRootNode, scene,mainEntity);

    return mainEntity;
}
Scene ResourceManager::ParseScene(const std::string& shaderPath)
{
    return Scene();
}
std::string& ResourceManager::SaveScene()
{
    std::string hey = std::string("Hey");
    // TODO: insert return statement here
    return hey;
}
// TODO : CREATE SHADER WITH THE LIGHT TYPES AND AMOUNTS !!!!!!!!!!!!!
void ResourceManager::InitShaders()
{
    
    for (auto& shader : GetInstance().mShaders)
	{
        // TODO: GET LIGHTS 
        // Get Lights from current renderer in game
        Registry& reg = Registry::GetInstance();
        auto& res = GetInstance();
        auto viewDir = reg.GetView<DirectionalLight>();
        auto viewPoint = reg.GetView<PointLight>();
        int dirLightCount = viewDir.size();
        int pointLightCount = viewPoint.size();
        // make sure the counters cant be below 1
        if (dirLightCount < 1)
            dirLightCount = 1;
        if (pointLightCount < 1)
            pointLightCount = 1;
        std::string shaderVersion = "#version 330 core\n";
        std::string fragmentArraySizes = "const int MAX_POINT_LIGHTS = " + std::to_string(dirLightCount) + ";\nconst int MAX_DIR_LIGHTS = " + std::to_string(pointLightCount) + ";\n";
        shader.second.source.FragmentSource = shaderVersion + fragmentArraySizes + shader.second.source.FragmentSource;
		shader.second.rendererID = CreateShader(shader.second.source.VertexSource, shader.second.source.FragmentSource);
	}
}



int ResourceManager::GetUniformLocation(const unsigned int rendererID, const std::string& name)
{
    auto& res = GetInstance();
    if (res.mUniformLocationCache[rendererID].find(name) != res.mUniformLocationCache[rendererID].end())
        return res.mUniformLocationCache[rendererID][name];
    else
        printf("Warning: uniform '%s' doesn't exist!\n", name.c_str());

    return -1;
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
    shader.source.VertexSource = ss[0].str();
    shader.source.FragmentSource = ss[1].str();
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
        GetInstance().mUniformLocationCache[programID].insert(std::pair<std::string, int>(name, location));
        GetInstance().mUniforms[programID].insert(std::pair<std::string, GLenum>(name, type));
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
        char* message = (char*)alloca(length * sizeof(char));
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
// TODO:
// 1. make it so if it has children it creates more meshes per entity
void ResourceManager::ProcessNode(aiNode* node, const aiScene* scene, Entity parentEntity)
{
    Registry& reg = Registry::GetInstance();
    // process all the node's meshes (if any)
    if(node->mNumMeshes > 1)
		std::cout << "ERROR::ASSIMP::NODE HAS MORE THAN 1 MESH" << std::endl;
    //Entity entity;
    if (node->mNumMeshes != 0)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
        //entity = reg.CreateEntity(parentEntity);
        reg.AssignComponent<Mesh>(parentEntity, ProcessMesh(mesh, scene));
    }
    else
    {
        std::cout << "ERROR::ASSIMP::NODE HAS NO MESH" << std::endl;
        ProcessNode(node->mChildren[0], scene, parentEntity);
        //entity = parentEntity;
    }

    // then do the same for each of its children
    /*for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, entity);
    }*/
}

Mesh ResourceManager::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    //std::vector<Material> material;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.position = position;
        glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        vertex.normal = normal;
        if (mesh->mTextureCoords[0])
        {
            auto length = mesh->mTextureCoords[1];
            glm::vec2 textCoords(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            vertex.textCoords = textCoords;
        }
        else
            vertex.textCoords = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // TODO: ADD TEXTURE LOADIGN AND MATERIAL LOADING
    /*if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = LoadMaterialTextures(material,
            aiTextureType_DIFFUSE);

        std::vector<Texture> specularMaps = LoadMaterialTextures(material,
            aiTextureType_SPECULAR);
    }*/

    // creating the mesh

    // generate vertex Buffer
    unsigned int vertexBuffer;
    GLCall(glGenBuffers(1, &vertexBuffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));
    // generate vertex array
    unsigned int vertexArray;
    GLCall(glGenVertexArrays(1, &vertexArray));
    
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
    return meshComponent;

}

std::vector<TextureData> ResourceManager::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    return std::vector<TextureData>();
}


/////////////////////////////////////////
// TEXTURE STUFF ////////////////////////
/////////////////////////////////////////
