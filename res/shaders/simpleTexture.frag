#version 450

struct PointLight
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float intensity;
	float constant;
    float linear;
    float quadratic; 
};
struct DirLight 
{
	vec4 direction;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};
struct s_GlobalUniformBufferObject {
	PointLight pointLights[4];
	DirLight dirLights[4];
	uint dirLightCount;
	uint pointLightCount;
};
struct s_MaterialUniformBufferObject {
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 ambientColor;
};
struct s_constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPosition;
};
#ifdef VULKAN
layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	PointLight pointLights[4];
	DirLight dirLights[4];
	uint dirLightCount;
	uint pointLightCount;
};
layout(set = 1, binding = 0) uniform MaterialUniformBufferObject {
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 ambientColor;
};

layout( push_constant ) uniform constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPosition;
};

layout(set = 1, binding = 1) uniform sampler2D colorTexture;
#endif
#ifdef OPENGL
layout(std140, binding = 0) uniform GlobalUniformBufferObject {
	PointLight pointLights[4];
	DirLight dirLights[4];
	uint dirLightCount;
	uint pointLightCount;
};
layout(std140, binding = 1) uniform MaterialUniformBufferObject {
	vec4 diffuseColor;
	vec4 specularColor;
	vec4 ambientColor;
};

layout(std140, binding = 2) uniform constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPosition;
};

layout(binding = 1) uniform sampler2D colorTexture;
#endif
struct Material
{
	int diffuseIsMap;
	vec3 diffuse;
	sampler2D diffuseMap;

	int specularIsMap;
	vec3 specular;
	sampler2D specularMap;
};

layout(location = 0) in vec2 v_TexCoord;
layout(location = 1) in vec3 v_FragPos;
layout(location = 2) in vec3 v_Normal;

layout(location = 0) out vec4 color;
//uniform mat4 u_Model;
//uniform mat4 u_View;
//uniform vec4 u_GlobalLightColor;
//uniform vec4 u_LightPosition;

//uniform Material u_Material;

void main()
{
	color = texture(colorTexture, v_TexCoord);
}