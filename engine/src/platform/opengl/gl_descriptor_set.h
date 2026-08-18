#pragma once
#include "renderer/descriptor_set.h"
#include "renderer/buffer.h"
#include <GL/glew.h>
#include <unordered_map>

// NOTE: OpenGL has no descriptor-set objects. A GlDescriptorSet is just a
// CPU-side container that records which buffers/images were attached to a
// compute program (by GL binding point). The bindings are applied with
// glBindBufferBase / glBindTextureUnit / glBindImageTexture right before
// glDispatchCompute (see GlComputeManager).
struct GlDescriptorSet : public DescriptorSet {
	struct BufferBinding {
		GLuint bufferID = 0;
		BufferType type = BufferType::SSBO;
	};
	struct ImageBinding {
		GLuint textureID = 0;
		GLenum target = GL_TEXTURE_2D;
		GLenum internalFormat = GL_RGBA8;
		DescriptorType type = DescriptorType::StorageImage;
	};
	std::unordered_map<uint32_t, BufferBinding> buffers;
	std::unordered_map<uint32_t, ImageBinding> images;
};
