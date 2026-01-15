#pragma once
#include <memory>
struct DescriptorSet {
	static std::shared_ptr<DescriptorSet> Create();
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
struct DescriptorSetReflection {
    struct Binding {
        uint32_t set;
        uint32_t binding;
        DescriptorType type;
        std::string name;
    };
    std::vector<Binding> bindings;
    std::unordered_map<std::string, size_t> nameToIndex;
};
