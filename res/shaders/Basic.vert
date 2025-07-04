#version 450

struct PointLight
{
	vec3 position;
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
	float intensity;
};

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	PointLight pointLights[5];
	DirLight dirLights[5];
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
	float time;
};
layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextCoord;

out vec2 v_TextCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat3 u_Normal;

void main()
{
   gl_Position = mvp * aPosition; 
   v_TextCoord = aTextCoord;
   v_FragPos = vec3(u_Model * aPosition);
   v_Normal = u_Normal * aNormal;
};