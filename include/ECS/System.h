#pragma once
class Registry;
// functions as base class to store systems
class System
{
public:
	System() = default;
	virtual ~System() = default;
	virtual void Start(Registry& registry) = 0;
	virtual void Update(Registry& registry) = 0;
};