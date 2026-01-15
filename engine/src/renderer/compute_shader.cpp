#include "compute_shader.h"
#include "program.h"
ComputeShaderData::ComputeShaderData()
{
	program = Program::Create();
}
