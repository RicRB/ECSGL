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
	void drawFrame(
		const glm::mat4 viewMatrix,
		const glm::mat4 projectionMatrix,
		const GlobalUBO* globalUniformObject,
		const int entityCount,
		const std::vector<glm::mat4x4>& modelMatrices,
		const std::vector<MeshData*>& meshes,
		const std::vector<MaterialData*>& materials,
		const std::vector<ShaderData>& shaders
	) override;
	void beginDrawImGui() override;
	void endDrawImGui() override;
	void cleanup() override;
	void compileShader(const char* name, const std::string& vertexSource, const std::string& fragmentSource, ShaderData* shaderData) override;
	void loadMaterial(const char* name, const ShaderData& shaderData, MaterialData* materialData) override;
	void loadMesh(MeshData* mesh) override;
	void loadTexture(TextureData* textureData) override;
	void unloadShader(ShaderData* shaderData) override;
	void unloadMaterial(MaterialData* materialData) override;
	void unloadMesh(MeshData* mesh) override;
	void unloadTexture(TextureData* textureData) override;
	void bindTextureToMaterial(MaterialData* materialData, TextureData* textureData, int binding) override;

private:
	GLuint createUniformBuffer(const size_t size);
	void updateUniformBuffer(GLuint uniformBuffer, const void* data, const size_t size);

};