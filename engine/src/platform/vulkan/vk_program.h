#pragma once
#include "renderer/program.h"
#include <vulkan/vulkan.h>

struct VkProgram : public Program {
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	VkDescriptorSetLayout descriptorSetLayout = nullptr;
};