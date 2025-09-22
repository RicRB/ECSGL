#include "renderer/opengl.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "vertex_buffer_layout.h"
#include <sstream>
#include <unordered_set>
void OpenGl::initialize(GLFWwindow* window)
{
	this->window = window;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glFrontFace(GL_CCW); // Counter-clockwise winding order
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB); // Enable sRGB framebuffer for color correction
    glDepthFunc(GL_LESS);

	// create global uniform buffer
	globalUniformBuffer = createUniformBuffer(sizeof(GlobalUBO));
	pushConstantBuffer = createUniformBuffer(sizeof(PushConstants));
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projectView = projectionMatrix * viewMatrix;
    glm::vec3 cameraWorldPos = glm::inverse(viewMatrix)[3];
    const glm::vec4 cameraPosition = glm::vec4(cameraWorldPos, 1.0f);
	updateUniformBuffer(globalUniformBuffer, globalUniformObject, sizeof(GlobalUBO));

    // get new uniforms
    std::unordered_set<MaterialID> updatedMaterials;
    for (auto material : materials) {
        if (updatedMaterials.contains(material->id)) continue;
        updatedMaterials.insert(material->id);
        updateUniformBuffer(material->gl_uniformBuffer, &material->ubo, sizeof(MaterialUBO));
    }

	for (size_t i = 0; i < entityCount; i++)
	{
		Transform* transform = transforms[i];
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
        PushConstants pushConstants;
        pushConstants.mvp = projectView * transforms[i]->getMatrix();

        glm::mat4 model = transforms[i]->getMatrix();
        pushConstants.model = glm::mat3x4(
            glm::vec4(model[0].x, model[0].y, model[0].z, model[3].x),
            glm::vec4(model[1].x, model[1].y, model[1].z, model[3].y),
            glm::vec4(model[2].x, model[2].y, model[2].z, model[3].z));
        pushConstants.cameraPosition = cameraPosition;
		updateUniformBuffer(pushConstantBuffer, &pushConstants, sizeof(PushConstants));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUniformBuffer));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 1, material->gl_uniformBuffer));
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2, pushConstantBuffer));
        GLCall(glDrawElements(GL_TRIANGLES, mesh->vertexCount, GL_UNSIGNED_INT, nullptr));
	}
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGl::beginDrawImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OpenGl::endDrawImGui()
{
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
		std::istringstream vertexStream(vertexSource);
        std::string line;
        std::string version;
        std::string body;
        bool firstLine = true;
		while (std::getline(vertexStream, line))
		{
			if (firstLine)
			{
				version = line + "\n";
                firstLine = false;
			}
			else
			{
				body += line + "\n";
			}
		}

		std::string finalSource = version + "#define OPENGL\n" + body;
        const char* vertSrc = finalSource.c_str();
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
        std::istringstream fragStream(fragmentSource);
        std::string line;
        std::string version;
        std::string body;
        bool firstLine = true;
        while (std::getline(fragStream, line))
        {
            if (firstLine)
            {
                version = line + "\n";
                firstLine = false;
            }
            else
            {
                body += line + "\n";
            }
        }

        std::string finalSource = version + "#define OPENGL\n" + body;
        const char* fragSrc = finalSource.c_str();
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
    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length);
        glGetProgramInfoLog(programID, length, &length, message);
        std::cout << "Failed to link shader program!" << std::endl;
        std::cout << message << std::endl;
        glDeleteProgram(programID);
        return;
    }
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
	materialData->shaderID = shaderData.id;
	materialData->gl_uniformBuffer = createUniformBuffer(sizeof(MaterialUBO));
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
    mesh->vertexID = vertexArray;
	mesh->indexID = indexBuffer;
}

void OpenGl::loadTexture(TextureData* textureData)
{

}

void OpenGl::unloadShader(ShaderData* shaderData)
{
	GLCall(glDeleteProgram(shaderData->programID));
	shaderData->programID = 0;
}

void OpenGl::unloadMaterial(MaterialData* materialData)
{
	GLCall(glDeleteBuffers(1, &materialData->gl_uniformBuffer));
	materialData->gl_uniformBuffer = 0;
}

void OpenGl::unloadMesh(MeshData* mesh)
{
	GLCall(glDeleteBuffers(1, &mesh->vertexID));
	GLCall(glDeleteBuffers(1, &mesh->indexID));
	mesh->vertexID = 0;
	mesh->indexID = 0;
}

void OpenGl::unloadTexture(TextureData* textureData)
{
}

GLuint OpenGl::createUniformBuffer(const size_t size)
{
	GLuint bufferID;
    GLCall(glGenBuffers(1, &bufferID));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, bufferID));
    GLCall(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    return bufferID;
}

void OpenGl::updateUniformBuffer(GLuint uniformBuffer, const void* data, const size_t size)
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer));
    GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}
