#pragma once

struct alignas(16) PointLightUniform {
	glm::vec4 position;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float intensity;
	float constant;
	float linear;
	float quadratic;
};

struct alignas(16) DirectionLightUniform {
	glm::vec4 direction; // Direction of the light
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float intensity;
};


struct alignas(256) GlobalUBO {
	PointLightUniform pointLights[5]; // 10 point lights
	DirectionLightUniform dirLights[5]; // 10 directional lights
	uint32_t dirLightCount;
	uint32_t pointLightCount;
};