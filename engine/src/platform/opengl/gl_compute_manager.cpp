#include "gl_compute_manager.h"
#include "gl_buffer.h"
#include "gl_image.h"
#include "gl_fence.h"
#include "gl_program.h"
#include "gl_def.h"
#include <cstring>

void GlComputeManager::setBuffer(DescriptorSet* set, BufferData* buffer, int binding)
{
	GlDescriptorSet* glSet = static_cast<GlDescriptorSet*>(set);
	GlBuffer* glBuffer = static_cast<GlBuffer*>(buffer);
	glSet->buffers[binding] = {
		glBuffer->bufferID,
		glBuffer->desc.type
	};
}

void GlComputeManager::setBuffer(ComputeShaderData& shaderData, DescriptorSet* set, BufferData* buffer, std::string_view name)
{
	DescriptorBinding& binding = shaderData.descriptorReflection.bindings[shaderData.descriptorReflection.nameToIndex[name.data()]];
	this->setBuffer(set, buffer, binding.binding);
}

void GlComputeManager::setImage(DescriptorSet* set, Image* image, DescriptorBinding& binding)
{
	GlDescriptorSet* glSet = static_cast<GlDescriptorSet*>(set);
	GlImage* glImage = static_cast<GlImage*>(image);
	glSet->images[binding.binding] = {
		glImage->textureID,
		glImage->target,
		glImage->internalFormat,
		binding.type
	};
}

void GlComputeManager::setImage(ComputeShaderData& shaderData, DescriptorSet* set, Image* image, std::string_view name)
{
	DescriptorBinding& binding = shaderData.descriptorReflection.bindings[shaderData.descriptorReflection.nameToIndex[name.data()]];
	this->setImage(set, image, binding);
}

void GlComputeManager::dispatchInternal(ComputeShaderData& computeShaderData, DescriptorSet* set, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
	GLuint programID = std::static_pointer_cast<GlProgram>(computeShaderData.program)->programID;
	GLCall(glUseProgram(programID));

	GlDescriptorSet* glSet = static_cast<GlDescriptorSet*>(set);

	// Bind SSBO / UBO buffers to their GL binding points.
	for (const auto& [binding, buffer] : glSet->buffers)
	{
		GLenum target = buffer.type == BufferType::Uniform ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;
		GLCall(glBindBufferBase(target, binding, buffer.bufferID));
	}

	// Bind samplers (texture units) and storage images (image units).
	for (const auto& [binding, image] : glSet->images)
	{
		if (image.type == DescriptorType::CombinedImageSampler || image.type == DescriptorType::SampledImage)
		{
			// The shader declares layout(binding = N) for the sampler, so the
			// texture is placed on the matching texture unit.
			glActiveTexture(GL_TEXTURE0 + binding);
			GLCall(glBindTexture(image.target, image.textureID));
		}
		else if (image.type == DescriptorType::StorageImage)
		{
			// NOTE: access is conservatively READ_WRITE; OpenGL requires the
			// sized internal format to be stated here (stored at load time).
			// layered must be TRUE for 3D textures and FALSE for 2D.
			GLboolean layered = (image.target == GL_TEXTURE_3D) ? GL_TRUE : GL_FALSE;
			GLCall(glBindImageTexture(binding, image.textureID, 0, layered, 0, GL_READ_WRITE, image.internalFormat));
		}
	}

	GLCall(glDispatchCompute(threadsX, threadsY, threadsZ));

	// NOTE: maps the Vulkan compute->host pipeline barrier to a GL memory
	// barrier so subsequent reads of the written buffers/images see the data.
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	GLCall(glUseProgram(0));
}

void GlComputeManager::dispatchComputeShader(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
	dispatchInternal(computeShaderData, set, threadsX, threadsY, threadsZ);

	// Synchronous dispatch: wait for completion (like the Vulkan
	// endSingleTimeCommands which does vkQueueWaitIdle) and leave the fence
	// in the signaled state.
	if (fence != nullptr)
	{
		GlFence* glFence = static_cast<GlFence*>(fence);
		glFence->reset();
	}
}

void GlComputeManager::dispatchComputeShaderAsync(ComputeShaderData& computeShaderData, DescriptorSet* set, Fence* fence, uint32_t threadsX, uint32_t threadsY, uint32_t threadsZ)
{
	dispatchInternal(computeShaderData, set, threadsX, threadsY, threadsZ);

	// Asynchronous dispatch: record a sync after the queued dispatch so
	// Fence::isSignaled()/wait() report GPU completion.
	if (fence != nullptr)
	{
		GlFence* glFence = static_cast<GlFence*>(fence);
		glFence->signalGPUCompletion();
	}
}