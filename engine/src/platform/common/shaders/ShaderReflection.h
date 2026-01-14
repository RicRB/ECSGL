#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
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
struct ReflectionData {
    struct Binding {
        uint32_t set;
        uint32_t binding;
        DescriptorType type;
        std::string name;
    };
    std::vector<Binding> bindings;
	std::unordered_map<std::string, size_t> nameToIndex;
};


class ShaderReflection {
public:
    static ReflectionData reflectSPIRV(const std::vector<uint32_t>& spirvBinary);
};