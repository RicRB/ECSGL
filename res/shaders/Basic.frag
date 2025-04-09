
struct Material
{
	int diffuseIsMap;
	vec3 diffuse;
	sampler2D diffuseMap;

	int specularIsMap;
	vec3 specular;
	sampler2D specularMap;
};
struct DirLight 
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct PointLight
{
	vec3 position;
	
	float constant;
    float linear;
    float quadratic; 

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct Light
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
layout(location = 0) out vec4 color;
in vec2 v_TextCoord;
in vec3 v_FragPos;
in vec3 v_Normal;
uniform mat4 u_Model;
uniform mat4 u_View;
uniform vec4 u_GlobalLightColor;
uniform vec4 u_LightPosition;

uniform Material u_Material;
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];

vec3 CalcDirLight(vec3 diffuseMaterial, vec3 specularMaterial, DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32); // TODO: change 32 for material shininess
    // combine results
    vec3 ambient  = light.ambient  * diffuseMaterial;
    vec3 diffuse  = light.diffuse  * diff * diffuseMaterial;
    vec3 specular = light.specular * spec * specularMaterial;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(vec3 diffuseMaterial, vec3 specularMaterial, PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
	vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32);
    // attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * diffuseMaterial;
    vec3 diffuse  = light.diffuse  * diff * diffuseMaterial;
    vec3 specular = light.specular * spec * specularMaterial;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{
	vec3 viewPos = u_View[3].xyz;
	vec3 viewDir = normalize(viewPos - v_FragPos);
	vec3 norm = normalize(v_Normal);

	// GET DIFFUSE MATERIAL
	vec3 diffuseMaterial;
	if(u_Material.diffuseIsMap == 1)
	{
		diffuseMaterial = texture(u_Material.diffuseMap, v_TextCoord).rgb;
	}
	else
	{
		diffuseMaterial = u_Material.diffuse;
	}
	// GET SPECULAR MATERIAL
	vec3 specularMaterial;
	if(u_Material.specularIsMap == 1)
	{
		specularMaterial = texture(u_Material.specularMap, v_TextCoord).rgb;
	}
	else
	{
		specularMaterial = u_Material.specular;
	}
	// Calculate Directional lights :D
	vec4 dirLightColor = vec4(0.0);
	for(int i = 0; i < MAX_DIR_LIGHTS; i++)
	{
		dirLightColor += vec4(CalcDirLight(diffuseMaterial, specularMaterial, dirLights[i], norm, viewDir), 1.0);
	}
	// calculate point lights 
	vec4 pointLightColor = vec4(0.0);
	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		pointLightColor += vec4(CalcPointLight(diffuseMaterial, specularMaterial, pointLights[i], norm, v_FragPos, viewDir), 1.0);
	}
	color = pointLightColor + dirLightColor;;
};