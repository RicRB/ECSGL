#include "opengl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "vertex_buffer_layout.h"
void OpenGl::initialize(GLFWwindow* window)
{
	this->window = window;
    glewInit();
    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR!" << std::endl;
    }
    glEnable(GL_BLEND);
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}
void OpenGl::initializeImGui()
{
    ImGui::CreateContext();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();


    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    isImGuiInitialized = true;
}

void OpenGl::drawFrame(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const GlobalUBO* globalUniformObject, const int entityCount, const std::vector<Transform*> transforms, const std::vector<MeshData*> meshes, const std::vector<MaterialData*> materials, const std::vector<ShaderData>* shaders)
{
	for (size_t i = 0; i < entityCount; i++)
	{
		const Transform* transform = transforms[i];
		const MeshData* mesh = meshes[i];
		const MaterialData* material = materials[i];
		const ShaderData* shader = &(*shaders)[material->shaderID];
        int textureIndex = 0;
		// Bind vertex array
		GLCall(glBindVertexArray(mesh->vertexID));
		// Bind index buffer
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexID));

        // bind Shader
		GLCall(glUseProgram(shader->programID));

        // bind material
		if (material->diffuse.type == VariableType::TEXTURE)
		{
			shader::SetUniform1b(shader->programID, "u_Material.diffuseIsMap", true);
            GLCall(glActiveTexture(GL_TEXTURE0 + textureIndex));
            GLCall(glBindTexture(GL_TEXTURE_2D, material->diffuse.texture.rendererID));
            textureIndex++;
		}
		else
		{
			shader::SetUniform1b(shader->programID, "u_Material.diffuseIsMap", false);
			shader::SetUniform3f(shader->programID, "u_Material.diffuse", material->diffuse.vec3.x, material->diffuse.vec3.y, material->diffuse.vec3.z);
		}
        //Specular Uniform
    	if (material->specular.type == VariableType::TEXTURE)
    	{
    		shader::SetUniform1b(shader->programID, "u_Material.specularIsMap", true);
    		GLCall(glActiveTexture(GL_TEXTURE0 + textureIndex));
    		GLCall(glBindTexture(GL_TEXTURE_2D, material->specular.texture.rendererID));
    		textureIndex++;
    	}
    	else
    	{
    		shader::SetUniform1b(shader->programID, "u_Material.specularIsMap", false);
    		shader::SetUniform3f(shader->programID, "u_Material.specular", material->specular.vec3.x, material->specular.vec3.y, material->specular.vec3.z);
    	}

        GLCall(glDrawElements(GL_TRIANGLES, mesh->vertexCount, GL_UNSIGNED_INT, nullptr));
	}
}

void OpenGl::beginDrawImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OpenGl::endDrawImGui()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::Render();
}

void OpenGl::cleanup()
{
    if (isImGuiInitialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }
}

void OpenGl::compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData)
{
    unsigned int vertexID = glCreateShader(GL_VERTEX_SHADER);
    {
        // vertex
        const char* vertSrc = vertexSource.c_str();
        glShaderSource(vertexID, 1, &vertSrc, nullptr);
        glCompileShader(vertexID);

        int result;
        glGetShaderiv(vertexID, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(vertexID, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)_malloca(length * sizeof(char));
            glGetShaderInfoLog(vertexID, length, &length, message);
            std::cout << "Failed to compile vertex shader!" << std::endl;
            std::cout << message << std::endl;
            glDeleteShader(vertexID);
            return;
        }
    }
    unsigned int fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
    {
        // fragment

        const char* fragSrc = fragmentSource.c_str();
        glShaderSource(fragmentID, 1, &fragSrc, nullptr);
        glCompileShader(fragmentID);

        int result;
        glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(fragmentID, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)_malloca(length * sizeof(char));
            glGetShaderInfoLog(fragmentID, length, &length, message);
            std::cout << "Failed to compile fragment shader!" << std::endl;
            std::cout << message << std::endl;
            glDeleteShader(fragmentID);
            return;
        }
    }
	unsigned int programID = glCreateProgram();
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);
    glValidateProgram(programID);
	// Clean up shaders
	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);
	shaderData->programID = programID;
	shaderData->VertexSource = vertexSource;
	shaderData->FragmentSource = fragmentSource;
}

void OpenGl::loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData)
{
}

void OpenGl::loadMesh(MeshData* mesh)
{
    // generate vertex Buffer
    unsigned int vertexBuffer;
    GLCall(glGenBuffers(1, &vertexBuffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, mesh->vertexCount * sizeof(Vertex), mesh->vertices.data(), GL_STATIC_DRAW));
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
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW));
    mesh->vertexID = vertexBuffer;
	mesh->indexID = indexBuffer;
}

void OpenGl::unloadShader(ShaderData* shaderData)
{
}

void OpenGl::unloadMaterial(MaterialData* materialData)
{
}

void OpenGl::unloadMesh(MeshData* mesh)
{
}
