#version 450

layout( push_constant ) uniform constants {
	mat4 mvp;
	mat3x4 model;
	vec4 cameraPos;
};
layout(location= 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextCoord;

layout(location = 0) out vec2 v_TextCoord;
layout(location = 1) out vec3 v_FragPos;
layout(location = 2) out vec3 v_Normal;

void main()
{
   gl_Position = mvp * vec4(aPosition, 1.0); 
   v_TextCoord = aTextCoord;
   v_FragPos = v_FragPos = vec3(
    dot(model[0], vec4(aPosition, 1.0)),
    dot(model[1], vec4(aPosition, 1.0)),
    dot(model[2], vec4(aPosition, 1.0))
);
   mat3 inverseModel = transpose(inverse(mat3(model)));
   v_Normal = normalize(inverseModel * aNormal);
}