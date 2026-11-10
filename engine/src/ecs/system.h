#pragma once

class System {
public:
	System() = default;
	virtual ~System() = default;
	// called once 
	virtual void start() {}
	// Called once per frame
	virtual void update() {}
	virtual void onGui() {}
	// Called when the system is cleaned up
	virtual void cleanup() {}
	// Returns the name of the system
	virtual const char* getName() const = 0;
};