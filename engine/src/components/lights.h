#pragma once
#include <glm/glm.hpp>
struct Light
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
	Light()
		: ambient(glm::vec3(0.5f)), diffuse(glm::vec3(1.0f)), specular(glm::vec3(0.2f)), intensity(2.0f) {}
};
struct PointLight
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
	float constant;
	float linear;
	float quadratic;
	PointLight()
		: ambient(glm::vec3(0.5f)), diffuse(glm::vec3(1.0f)), specular(glm::vec3(0.2f)), intensity(2.0f), constant(1.0f), linear(0.09f), quadratic(0.032f) {}
	PointLight(glm::vec3 amb, glm::vec3 dif, glm::vec3 spec, float inten, float con, float lin, float quad)
		: ambient(amb), diffuse(dif), specular(spec), intensity(inten), constant(con), linear(lin), quadratic(quad) {}
};
struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float intensity;
	DirectionalLight()
		: direction(glm::vec3(0.0f)), ambient(glm::vec3(0.5f)), diffuse(glm::vec3(1.0f)), specular(glm::vec3(0.2f)), intensity(2.0f) {}
	DirectionalLight(glm::vec3 dir, glm::vec3 amb, glm::vec3 dif, glm::vec3 spec, float inten)
		: direction(dir), ambient(amb), diffuse(dif), specular(spec), intensity(inten) {}
};