#pragma once
#include <memory>
struct DescriptorSet {
	static std::shared_ptr<DescriptorSet> Create();
};