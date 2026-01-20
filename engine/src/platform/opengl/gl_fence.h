#pragma once
#include "renderer/fence.h"

class GlFence : public Fence {
private:

public:
	bool isSignaled() override;
};