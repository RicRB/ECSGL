#pragma once
#include <memory>
#include <string>
#include <unordered_map>
struct DescriptorSet {
	static std::shared_ptr<DescriptorSet> Create();
    static std::unique_ptr<DescriptorSet> CreateUnique();
};

enum class DescriptorType {
    Sampler,
    CombinedImageSampler,
    SampledImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,
};

struct DescriptorBinding
{
    uint32_t set;
    uint32_t binding;
    DescriptorType type;
    std::string name;
};

struct DescriptorSetReflection {
    std::vector<DescriptorBinding> bindings;
    std::unordered_map<std::string, size_t> nameToIndex;
};
