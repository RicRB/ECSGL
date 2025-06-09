#include "glm/glm.hpp"
struct PhysicsBody
{
public:
	glm::vec3 m_velocity;
	float m_mass;
	PhysicsBody(glm::vec3 velocity = glm::vec3(0), float mass = 1.0f) : m_velocity(velocity), m_mass(m_mass) {};
};