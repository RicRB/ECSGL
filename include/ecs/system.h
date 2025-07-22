#pragma once

class System {
public:
	System() = default;
	virtual ~System() = default;
	// called once 
	virtual void start() = 0;
	// Called once per frame
	virtual void update() = 0;
	// Called when the system is cleaned up
	virtual void cleanup() = 0;
	// Returns the name of the system
	virtual const char* getName() const = 0;
};