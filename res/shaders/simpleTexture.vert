#version 450

#ifdef VULKAN
layout( push_constant ) uniform constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPos;
};
#endif
#ifdef OPENGL
layout( std140, binding = 2 ) uniform constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPosition;
};
#endif
layout(location= 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec2 v_TexCoord;
layout(location = 1) out vec3 v_FragPos;
layout(location = 2) out vec3 v_Normal;

void main()
{
   v_TexCoord = aTexCoord;
   gl_Position = vec4(aPosition, 1.0);
   v_Normal = vec3(1.0,1.0,1.0);
}