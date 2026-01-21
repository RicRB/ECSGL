#include "renderer/graphics_api.h"
#include "gl_def.h"
class OpenGl : public GraphicsAPI {
public:
private:
	GLuint globalUniformBuffer = 0;
	GLuint pushConstantBuffer = 0; // for push constants
public:
	void initialize(GLFWwindow* window) override;
	void initializeImGui() override;
	void drawFrame(FrameDrawData& frameDrawData) override;
	void beginDrawImGui() override;
	void endDrawImGui() override;
	void cleanup() override;
	void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) override;
	void compileComputeShader(const char* name, ComputeShaderData* computeShaderData);
	void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) override;
	void loadMesh(MeshData* mesh) override;
	void loadTexture(TextureData* textureData) override;
	void unloadShader(ShaderData* shaderData) override;
	void unloadComputeShader(ComputeShaderData* computeShaderData);
	void unloadMaterial(MaterialData* materialData) override;
	void unloadMesh(MeshData* mesh) override;
	void unloadTexture(TextureData* textureData) override;
	void bindTextureToMaterial(MaterialData* materialData, TextureData* textureData, int binding) override;
	void setTextureUsage(TextureData* textureData, TextureUsage usage) override;
	void createDescriptorSet(Program* program, DescriptorSet* descriptorSet) override;
private:
	GLuint createUniformBuffer(const size_t size);
	void updateUniformBuffer(GLuint uniformBuffer, const void* data, const size_t size);

};