#include "compute_shader.h"

ComputeShaderData::ComputeShaderData()
{
	program = Program::Create();
	descriptorSet = DescriptorSet::Create();
}
