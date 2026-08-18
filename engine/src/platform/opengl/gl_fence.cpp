#include "gl_fence.h"

void GlFence::wait()
{
	// glFinish blocks until all queued GL commands complete, which is the
	// OpenGL equivalent of vkWaitForFences with UINT64_MAX.
	glFinish();
}

bool GlFence::isSignaled()
{
	if (m_sync == nullptr)
	{
		// No dispatch has been tracked yet, so there is nothing to wait on.
		return true;
	}

	GLenum result = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
	return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
}

void GlFence::reset()
{
	if (m_sync != nullptr)
	{
		glDeleteSync(m_sync);
		m_sync = nullptr;
	}
}

bool GlFence::isValid()
{
	// A GL fence is always usable; validity only gates reset() before a
	// re-dispatch in ComputeDispatcher.
	return true;
}

void GlFence::signalGPUCompletion()
{
	if (m_sync != nullptr)
	{
		glDeleteSync(m_sync);
	}
	m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}