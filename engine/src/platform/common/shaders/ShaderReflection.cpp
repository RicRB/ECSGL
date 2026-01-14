#include "ShaderReflection.h"
#include <spirv_cross/spirv_cross.hpp>

ReflectionData ShaderReflection::reflectSPIRV(const std::vector<uint32_t>& spirvBinary)
{
    spirv_cross::Compiler compiler(spirvBinary);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    ReflectionData reflection;

    // Reflect storage buffers (SSBOs)
    for (auto& resource : resources.storage_buffers) {
        ReflectionData::Binding binding;
        binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.type = DescriptorType::StorageBuffer;
        binding.name = resource.name;
        reflection.bindings.push_back(binding);
    }
	// Reflect uniform buffers (UBOs)
    for (auto& resource : resources.uniform_buffers) {
        ReflectionData::Binding binding;
        binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.type = DescriptorType::UniformBuffer;
        binding.name = resource.name;
        reflection.bindings.push_back(binding);
	}
	// Reflect sampled images
    for (auto& resource : resources.sampled_images) {
        ReflectionData::Binding binding;
        binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.type = DescriptorType::CombinedImageSampler;
        binding.name = resource.name;
        reflection.bindings.push_back(binding);
	}
	// Reflect storage images
    for (auto& resource : resources.storage_images) {
        ReflectionData::Binding binding;
        binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.type = DescriptorType::StorageImage;
        binding.name = resource.name;
        reflection.bindings.push_back(binding);
    }
	// Reflect samplers
    for (auto& resource : resources.separate_samplers) {
        ReflectionData::Binding binding;
        binding.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        binding.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        binding.type = DescriptorType::Sampler;
        binding.name = resource.name;
        reflection.bindings.push_back(binding);
	}

	// Sort bindings by set and binding number
    std::sort(reflection.bindings.begin(), reflection.bindings.end(),
        [](const ReflectionData::Binding& a, const ReflectionData::Binding& b) {
            if (a.set != b.set) return a.set < b.set;
            return a.binding < b.binding;
        });

	for (size_t i = 0; i < reflection.bindings.size(); ++i) {
        reflection.nameToIndex[reflection.bindings[i].name] = i;
    }

    return reflection;
}
