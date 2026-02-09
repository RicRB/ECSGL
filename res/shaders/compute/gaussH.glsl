#version 450

const int GROUP_SIZE = 64;

layout(local_size_x = 64, local_size_y = 1) in;
layout(rgba16, binding = 0) uniform readonly image2D input_image;
layout(rgba16, binding = 1) uniform restrict writeonly image2D output_image;

const int M = 3;
const int N = 7;

const float coeffs[N] = float[N](
    0.0701593,
    0.1310749,
    0.1907128,
    0.2161059,
    0.1907128,
    0.1310749,
    0.0701593
);

const int CACHE_SIZE = GROUP_SIZE + 2 * M;

const int LOAD = (CACHE_SIZE + (GROUP_SIZE - 1)) / GROUP_SIZE;

shared vec4 cache[CACHE_SIZE];

void main()
{
	ivec2 size = imageSize(input_image);
	ivec2 pixel_coord = ivec2(gl_GlobalInvocationID.xy);

	int origin = int(gl_WorkGroupID.x) * GROUP_SIZE - M;

	for (int i = 0; i < LOAD; ++i)
	{
		int local = int(gl_LocalInvocationID.x) * LOAD + i;
		if (local < CACHE_SIZE)
		{
			int pc = origin + local;

			if (pc >= 0 && pc < size.x)
				cache[local] = imageLoad(input_image, ivec2(pc, pixel_coord.y));
		}
	}

	memoryBarrierShared();
	barrier();

	if (pixel_coord.x < size.x && pixel_coord.y < size.y)
	{
		vec4 sum = vec4(0.0);

		for (int i = 0; i < N; ++i)
		{
			ivec2 pc = pixel_coord + ivec2(i - M, 0);
			if (pc.x < 0) pc.x = 0;
			if (pc.x >= size.x) pc.x = size.x - 1;

			int local = pc.x - origin;

			sum += coeffs[i] * cache[local];
		}

		imageStore(output_image, pixel_coord, sum);
	}
}