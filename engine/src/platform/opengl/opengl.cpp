#include "opengl.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "renderer/vertex_buffer_layout.h"
#include <sstream>
#include <unordered_set>
#include "gl_buffer.h"
#include "gl_image.h"
#include "gl_descriptor_set.h"
#include <memory>
#include <regex>
#include "renderer/uniform_objects.h"
#include "gl_program.h"
#include "GLFW/glfw3.h"
#include "gl_compute_manager.h"
#include "gl_buffer_manager.h"
#include "core/engine.h"

// Maps a TextureFormat to the GL sized internal format, base pixel format and
// pixel type used by glTexImage2D/3D and glBindImageTexture.
static void getGLFormats(TextureFormat format, GLenum& internalFormat, GLenum& pixelFormat, GLenum& type)
{
	switch (format)
	{
	case TextureFormat::R8G8B8A8_UNORM:
		internalFormat = GL_RGBA8; pixelFormat = GL_RGBA; type = GL_UNSIGNED_BYTE;
		break;
	case TextureFormat::R16G16_SFLOAT:
		internalFormat = GL_RG16F; pixelFormat = GL_RG; type = GL_HALF_FLOAT;
		break;
	case TextureFormat::R16G16_SNORM:
		internalFormat = GL_RG16_SNORM; pixelFormat = GL_RG; type = GL_SHORT;
		break;
	case TextureFormat::R16G16_UNORM:
		internalFormat = GL_RG16; pixelFormat = GL_RG; type = GL_UNSIGNED_SHORT;
		break;
	case TextureFormat::R16G16B16A16_UNORM:
		internalFormat = GL_RGBA16; pixelFormat = GL_RGBA; type = GL_UNSIGNED_SHORT;
		break;
	case TextureFormat::R32G32B32A32_SFLOAT:
		internalFormat = GL_RGBA32F; pixelFormat = GL_RGBA; type = GL_FLOAT;
		break;
	default:
		internalFormat = GL_RGBA8; pixelFormat = GL_RGBA; type = GL_UNSIGNED_BYTE;
		break;
	}
}

static std::string stripGLSLComments(const std::string& source)
{
	std::string result;
	result.reserve(source.size());
	size_t i = 0;
	while (i < source.size())
	{
		if (source[i] == '/' && i + 1 < source.size() && source[i + 1] == '/')
		{
			i += 2;
			while (i < source.size() && source[i] != '\n') ++i;
		}
		else if (source[i] == '/' && i + 1 < source.size() && source[i + 1] == '*')
		{
			i += 2;
			while (i + 1 < source.size() && !(source[i] == '*' && source[i + 1] == '/')) ++i;
			i += 2;
		}
		else
		{
			result += source[i];
			++i;
		}
	}
	return result;
}

// NOTE: OpenGL has no SPIR-V reflection. This parses the (preprocessed) compute
// source to produce the same binding/name/type table that ShaderReflection
// produces from SPIR-V on the Vulkan side, so setBuffer(name)/setTexture(name)
// resolve identically.
static DescriptorSetReflection reflectComputeGLSL(const std::string& source)
{
	DescriptorSetReflection reflection;
	const std::string clean = stripGLSLComments(source);

	auto addBinding = [&reflection](uint32_t binding, const std::string& name, DescriptorType type)
	{
		for (const auto& existing : reflection.bindings)
		{
			if (existing.binding == binding) return;
		}
		DescriptorBinding b;
		b.set = 0;
		b.binding = binding;
		b.name = name;
		b.type = type;
		reflection.bindings.push_back(b);
		reflection.nameToIndex[name] = reflection.bindings.size() - 1;
	};

	// layout(... binding = N ...) [qualifiers] buffer BlockName { ... } instanceName;
	{
		std::regex bufferRegex(R"(layout\s*\([^)]*?\bbinding\s*=\s*(\d+)[^)]*\)[^;{}]*?\bbuffer\s+\w+\s*\{[\s\S]*?\}\s*(\w+)\s*;)");
		auto it = clean.cbegin();
		std::smatch m;
		while (std::regex_search(it, clean.cend(), m, bufferRegex))
		{
			addBinding(static_cast<uint32_t>(std::stoul(m[1].str())), m[2].str(), DescriptorType::StorageBuffer);
			it = m.suffix().first;
		}
	}
	// layout(... binding = N ...) uniform sampler2D/3D name;
	{
		std::regex samplerRegex(R"(layout\s*\([^)]*?\bbinding\s*=\s*(\d+)[^)]*\)\s*uniform\s+sampler(?:2D|3D|Cube)\s+(\w+)\s*;)");
		auto it = clean.cbegin();
		std::smatch m;
		while (std::regex_search(it, clean.cend(), m, samplerRegex))
		{
			addBinding(static_cast<uint32_t>(std::stoul(m[1].str())), m[2].str(), DescriptorType::CombinedImageSampler);
			it = m.suffix().first;
		}
	}
	// layout(... binding = N ...) uniform [qualifiers] image2D/3D name;
	{
		std::regex imageRegex(R"(layout\s*\([^)]*?\bbinding\s*=\s*(\d+)[^)]*\)\s*uniform\s+[\w\s]*?\bimage(?:2D|3D)\s+(\w+)\s*;)");
		auto it = clean.cbegin();
		std::smatch m;
		while (std::regex_search(it, clean.cend(), m, imageRegex))
		{
			addBinding(static_cast<uint32_t>(std::stoul(m[1].str())), m[2].str(), DescriptorType::StorageImage);
			it = m.suffix().first;
		}
	}

	std::sort(reflection.bindings.begin(), reflection.bindings.end(),
		[](const DescriptorBinding& a, const DescriptorBinding& b) { return a.binding < b.binding; });

	reflection.nameToIndex.clear();
	for (size_t i = 0; i < reflection.bindings.size(); ++i)
	{
		reflection.nameToIndex[reflection.bindings[i].name] = i;
	}
	return reflection;
}

// Resolves a material texture handle to its GL image (nullptr if none bound).
static GlImage* getMaterialTextureImage(const Texture& texture)
{
	if (!texture.handle.isValid()) return nullptr;
	const auto& textureData = engine::GetEngine().resourceManager.m_textures->at(texture.handle);
	return static_cast<GlImage*>(textureData.image.get());
}

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
    computeManager = std::make_unique<GlComputeManager>();
	bufferManager = std::make_unique<GlBufferManager>();
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

void OpenGl::drawFrame(FrameDrawData& frameDrawData)
{
    auto& viewMatrix = frameDrawData.viewMatrix;
    auto& projectionMatrix = frameDrawData.projectionMatrix;
    auto& globalUniformObject = frameDrawData.globalUniformObject;
    auto& entityCount = frameDrawData.entityCount;
    auto& modelMatrices = frameDrawData.modelMatrices;
    auto& meshes = frameDrawData.meshes;
    auto& materials = frameDrawData.materials;
    auto& shaders = frameDrawData.shaders;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projectView = projectionMatrix * viewMatrix;
    glm::vec3 cameraWorldPos = glm::inverse(viewMatrix)[3];
    const glm::vec4 cameraPosition = glm::vec4(cameraWorldPos, 1.0f);
	updateUniformBuffer(globalUniformBuffer, globalUniformObject, sizeof(GlobalUBO));

    // get new uniforms
    std::unordered_set<MaterialData*> updatedMaterials;
    for (auto material : materials) {
        // TODO: COMPARE MATERIALS insTEAD OF CONTAINS...  also never insert them all the way
        if (updatedMaterials.contains(material)) continue;
        updatedMaterials.insert(material);
        updateUniformBuffer(std::static_pointer_cast<GlUniformBuffer>(material->uniformBuffer)->uniformID, &material->ubo, sizeof(MaterialUBO));
    }

	for (size_t i = 0; i < entityCount; i++)
	{
		const MeshData* mesh = meshes[i];
		const MaterialData* material = materials[i];
		const ShaderData* shader = &(shaders)[material->shaderHandle.index];
        int textureIndex = 0;
		// Bind vertex array
		GLCall(glBindVertexArray(std::static_pointer_cast<GlVertexBuffer>(mesh->vertexBuffer)->vertexID));
		// Bind index buffer
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, std::static_pointer_cast<GlIndexBuffer>(mesh->indexBuffer)->indexID));

        // bind Shader
		GLCall(glUseProgram(std::static_pointer_cast<GlProgram>(shader->program)->programID));

        // bind material
        PushConstants pushConstants;
        pushConstants.mvp = projectView * modelMatrices[i];

        glm::mat4 model = modelMatrices[i];
        pushConstants.model = glm::mat3x4(
            glm::vec4(model[0].x, model[0].y, model[0].z, model[3].x),
            glm::vec4(model[1].x, model[1].y, model[1].z, model[3].y),
            glm::vec4(model[2].x, model[2].y, model[2].z, model[3].z));
        pushConstants.cameraPosition = cameraPosition;
		updateUniformBuffer(pushConstantBuffer, &pushConstants, sizeof(PushConstants));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUniformBuffer));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 1, std::static_pointer_cast<GlUniformBuffer>(material->uniformBuffer)->uniformID));
        GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2, pushConstantBuffer));
        // Re-bind the material's albedo map to its texture unit. Compute
        // dispatches (e.g. the black hole passes) bind the same unit for their
        // own samplers and would otherwise leave the quad sampling stale data.
        if (GlImage* albedoImage = getMaterialTextureImage(material->albedoMap))
        {
            glActiveTexture(GL_TEXTURE0 + ALBEDO_MAP_BINDING + 1);
            GLCall(glBindTexture(albedoImage->target, albedoImage->textureID));
        }

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
	std::static_pointer_cast<GlProgram>(shaderData->program)->programID = programID;
	shaderData->vertexSource = vertexSource;
	shaderData->fragmentSource = fragmentSource;
}

void OpenGl::compileComputeShader(const char* name, ComputeShaderData* computeShaderData)
{
    // OpenGL GLSL does not support the Vulkan-only "set = N" qualifier, so strip it.
    std::string source = std::regex_replace(computeShaderData->source, std::regex(R"(set\s*=\s*\d+\s*,?)"), "");

    // Same version/define handling as compileShader: keep the first line as the
    // #version line and inject the OPENGL define after it.
    std::istringstream sourceStream(source);
    std::string line;
    std::string version;
    std::string body;
    bool firstLine = true;
    while (std::getline(sourceStream, line))
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

    unsigned int computeID = glCreateShader(GL_COMPUTE_SHADER);
    {
        const char* computeSrc = finalSource.c_str();
        glShaderSource(computeID, 1, &computeSrc, nullptr);
        glCompileShader(computeID);
        int result;
        glGetShaderiv(computeID, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(computeID, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)_malloca(length * sizeof(char));
            glGetShaderInfoLog(computeID, length, &length, message);
            std::cout << "Failed to compile compute shader!" << std::endl;
            std::cout << message << std::endl;
            glDeleteShader(computeID);
            return;
        }
    }
    unsigned int programID = glCreateProgram();
    glAttachShader(programID, computeID);
    glLinkProgram(programID);
    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        int length;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length);
        glGetProgramInfoLog(programID, length, &length, message);
        std::cout << "Failed to link compute shader program!" << std::endl;
        std::cout << message << std::endl;
        glDeleteProgram(programID);
        return;
    }
    glValidateProgram(programID);
    // Clean up shaders
    glDeleteShader(computeID);
    std::static_pointer_cast<GlProgram>(computeShaderData->program)->programID = programID;

    computeShaderData->descriptorReflection = reflectComputeGLSL(finalSource);
}
void OpenGl::loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData)
{
    std::static_pointer_cast<GlUniformBuffer>(materialData->uniformBuffer)->uniformID = createUniformBuffer(sizeof(MaterialUBO));
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
    std::static_pointer_cast<GlVertexBuffer>(mesh->vertexBuffer)->vertexID = vertexArray;
    std::static_pointer_cast<GlIndexBuffer>(mesh->indexBuffer)->indexID = indexBuffer;
}

void OpenGl::loadTexture(TextureData* textureData)
{
	GlImage* image = static_cast<GlImage*>(textureData->image.get());

	const bool is3D = textureData->depth > 1;
	GLenum target = is3D ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	image->target = target;

	GLenum internalFormat, pixelFormat, type;
	getGLFormats(textureData->format, internalFormat, pixelFormat, type);
	image->internalFormat = internalFormat;

	GLCall(glGenTextures(1, &image->textureID));
	GLCall(glBindTexture(target, image->textureID));

	GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT));
	if (is3D)
	{
		GLCall(glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_REPEAT));
	}
	GLCall(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if (is3D)
	{
		GLCall(glTexImage3D(target, 0, internalFormat, textureData->width, textureData->height, textureData->depth, 0, pixelFormat, type, textureData->pixels));
	}
	else
	{
		GLCall(glTexImage2D(target, 0, internalFormat, textureData->width, textureData->height, 0, pixelFormat, type, textureData->pixels));
	}

	GLCall(glBindTexture(target, 0));
}

void OpenGl::unloadShader(ShaderData* shaderData)
{
	GLCall(glDeleteProgram(std::static_pointer_cast<GlProgram>(shaderData->program)->programID));
    std::static_pointer_cast<GlProgram>(shaderData->program)->programID = 0;
}

void OpenGl::unloadComputeShader(ComputeShaderData* computeShaderData)
{
    GLCall(glDeleteProgram(std::static_pointer_cast<GlProgram>(computeShaderData->program)->programID));
    std::static_pointer_cast<GlProgram>(computeShaderData->program)->programID = 0;
}

void OpenGl::unloadMaterial(MaterialData* materialData)
{
	GLCall(glDeleteBuffers(1, &std::static_pointer_cast<GlUniformBuffer>(materialData->uniformBuffer)->uniformID));
	std::static_pointer_cast<GlUniformBuffer>(materialData->uniformBuffer)->uniformID = 0;
}

void OpenGl::unloadMesh(MeshData* mesh)
{
	auto vertexID = &std::static_pointer_cast<GlVertexBuffer>(mesh->vertexBuffer)->vertexID;
	auto indexID = &std::static_pointer_cast<GlIndexBuffer>(mesh->indexBuffer)->indexID;
	GLCall(glDeleteBuffers(1, vertexID));
	GLCall(glDeleteBuffers(1, indexID));
    *vertexID = 0;
    *indexID = 0;
}

void OpenGl::unloadTexture(TextureData* textureData)
{
	GlImage* image = static_cast<GlImage*>(textureData->image.get());
	if (image->textureID != 0)
	{
		GLCall(glDeleteTextures(1, &image->textureID));
		image->textureID = 0;
	}
}

void OpenGl::bindTextureToMaterial(MaterialData* materialData, TextureData* textureData, int binding)
{
	// NOTE: Vulkan writes a combined image sampler at binding + 1 in the
	// material's descriptor set. OpenGL has no descriptor sets, so the closest
	// equivalent is binding the texture to the matching texture unit (sampler
	// uniforms read from the unit equal to their layout(binding = N) value).
	GlImage* image = static_cast<GlImage*>(textureData->image.get());
	const int textureBinding = binding + 1;
	glActiveTexture(GL_TEXTURE0 + textureBinding);
	GLCall(glBindTexture(image->target, image->textureID));
}

void OpenGl::setTextureUsage(TextureData* textureData, TextureUsage usage)
{
	// NOTE: OpenGL has no explicit image layout transitions. Synchronize any
	// pending compute writes so the image can be sampled/read afterwards.
	textureData->usage = usage;
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void OpenGl::createDescriptorSet(Program* program, DescriptorSet* descriptorSet)
{
	// NOTE: OpenGL has no descriptor-set objects. A GlDescriptorSet is just a
	// CPU-side binding container that is populated by GlComputeManager::setBuffer
	// /setImage, so there is nothing to allocate here.
	if (descriptorSet != nullptr)
	{
		GlDescriptorSet* glSet = static_cast<GlDescriptorSet*>(descriptorSet);
		glSet->buffers.clear();
		glSet->images.clear();
	}
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
