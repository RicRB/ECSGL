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
struct Material
{
	int diffuseIsMap;
	vec3 diffuse;
	sampler2D diffuseMap;

	int specularIsMap;
	vec3 specular;
	sampler2D specularMap;
};


layout(location = 0) out vec4 color;
layout(location = 0) in vec2 v_TextCoord;
layout(location = 1) in vec3 v_FragPos;
layout(location = 2) in vec3 v_Normal;
//uniform mat4 u_Model;
//uniform mat4 u_View;
//uniform vec4 u_GlobalLightColor;
//uniform vec4 u_LightPosition;

//uniform Material u_Material;

vec3 CalcDirLight(vec3 diffuseMaterial, vec3 specularMaterial, DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32); // TODO: change 32 for material shininess
    // combine results
    vec3 ambient  = light.ambient.xyz  * diffuseMaterial;
    vec3 diffuse  = light.diffuse.xyz  * diff * diffuseMaterial;
    vec3 specular = light.specular.xyz * spec * specularMaterial;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(vec3 diffuseMaterial, vec3 specularMaterial, PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
    // attenuation
    float distance    = length(light.position.xyz - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient.xyz  * diffuseMaterial;
    vec3 diffuse  = light.diffuse.xyz  * diff * diffuseMaterial;
    vec3 specular = light.specular.xyz * spec * specularMaterial;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{
	vec3 viewDir = normalize(cameraPosition.xyz - v_FragPos);
	vec3 norm = normalize(v_Normal);

	// GET DIFFUSE MATERIAL
	vec3 diffuseMaterial;
	if(false)//u_Material.diffuseIsMap == 1)
	{
		//diffuseMaterial = texture(u_Material.diffuseMap, v_TextCoord).rgb;
	}
	else
	{
		diffuseMaterial = diffuseColor.xyz;
	}
	// GET SPECULAR MATERIAL
	vec3 specularMaterial;
	if(false)//u_Material.specularIsMap == 1)
	{
		//specularMaterial = texture(u_Material.specularMap, v_TextCoord).rgb;
	}
	else
	{
		specularMaterial = specularColor.xyz;
	}
	// Calculate Directional lights :D
	vec4 dirLightColor = vec4(0.0);
	for(int i = 0; i < dirLightCount; i++)
	{
		dirLightColor += vec4(CalcDirLight(diffuseMaterial, specularMaterial, dirLights[i], norm, viewDir), 1.0);
	}
	// calculate point lights 
	vec4 pointLightColor = vec4(0.0);
	for(int i = 0; i < pointLightCount; i++)
	{
		pointLightColor += vec4(CalcPointLight(diffuseMaterial, specularMaterial, pointLights[i], norm, v_FragPos, viewDir), 1.0);
	}
	color = pointLightColor + dirLightColor;
	//color = vec4(1.0,1.0,1.0,1.0);
}