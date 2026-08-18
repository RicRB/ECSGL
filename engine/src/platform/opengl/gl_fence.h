#pragma once
#include "renderer/fence.h"
#include <GL/glew.h>

// NOTE: OpenGL has no explicit fence objects like Vulkan. A GlFence wraps a
// GLsync (GL_SYNC_GPU_COMMANDS_COMPLETE) so async compute dispatch can be
// polled/waited on. When no dispatch has been recorded yet the fence is
// treated as already signaled.
class GlFence : public Fence {
private:
	GLsync m_sync = nullptr;
public:
	void wait() override;
	bool isSignaled() override;
	void reset() override;
	bool isValid() override;

	// Record a sync right after a GPU dispatch so isSignaled()/wait() reflect
	// when the queued work has completed.
	void signalGPUCompletion();
};