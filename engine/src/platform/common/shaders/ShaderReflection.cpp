#include "ShaderReflection.h"
#include "spirv_reflect/spirv_reflect.h"
#include <stdexcept>
#include <algorithm>
DescriptorSetReflection ShaderReflection::reflectSPIRV(const std::vector<uint32_t>& spirvBinary)
{
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(
        spirvBinary.size() * sizeof(uint32_t),
        spirvBinary.data(),
        &module);

    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        throw std::runtime_error("Failed to create SPIR-V reflection module.");
    }

    DescriptorSetReflection reflection;

    // Get all descriptor bindings
    uint32_t count = 0;
    result = spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        spvReflectDestroyShaderModule(&module);
        throw std::runtime_error("Failed to enumerate descriptor bindings.");
    }

    std::vector<SpvReflectDescriptorBinding*> bindings(count);
    result = spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
        spvReflectDestroyShaderModule(&module);
        throw std::runtime_error("Failed to get descriptor bindings.");
    }

    // Process each binding
    for (const auto* binding : bindings) {
        DescriptorSetReflection::Binding b;
        b.set = binding->set;
        b.binding = binding->binding;
        b.name = binding->name ? binding->name : "";

        // Map SPIRV-Reflect descriptor type to your DescriptorType enum
        switch (binding->descriptor_type) {
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            b.type = DescriptorType::UniformBuffer;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            b.type = DescriptorType::StorageBuffer;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            b.type = DescriptorType::CombinedImageSampler;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            b.type = DescriptorType::StorageImage;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
            b.type = DescriptorType::Sampler;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            b.type = DescriptorType::SampledImage;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            b.type = DescriptorType::UniformBufferDynamic;
            break;
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            b.type = DescriptorType::StorageBufferDynamic;
            break;
        default:
            // Skip unsupported descriptor types
            continue;
        }

        reflection.bindings.push_back(b);
    }

    // Sort bindings by set and binding number
    std::sort(reflection.bindings.begin(), reflection.bindings.end(),
        [](const DescriptorSetReflection::Binding& a, const DescriptorSetReflection::Binding& b) {
            if (a.set != b.set) return a.set < b.set;
            return a.binding < b.binding;
        });

    // Build name-to-index map
    for (size_t i = 0; i < reflection.bindings.size(); ++i) {
        if (!reflection.bindings[i].name.empty()) {
            reflection.nameToIndex[reflection.bindings[i].name] = i;
        }
    }

    // Clean up
    spvReflectDestroyShaderModule(&module);

    return reflection;
}
