#pragma once
// functions as base class to store systems
class System
{
public:
	System() = default;
	virtual ~System() = default;
	virtual void Start() = 0;
	virtual void Update() = 0;
};