#include "material.h"

MaterialData::MaterialData()
{
	descriptorSet = DescriptorSet::Create();
	uniformBuffer = UniformBuffer::Create();
}
