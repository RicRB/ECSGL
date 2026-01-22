#pragma once
#include <memory>
struct Fence {
public:
	virtual ~Fence() = default;
	virtual void wait() = 0;
	virtual bool isSignaled() = 0;
	virtual void reset() = 0;
	virtual bool isValid() = 0;
	static std::unique_ptr<Fence> createUnique();
};