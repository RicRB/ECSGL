#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include "renderer/descriptor_set.h"

//
class ShaderReflection {
public:
    static DescriptorSetReflection reflectSPIRV(const std::vector<uint32_t>& spirvBinary);
};