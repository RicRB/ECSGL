#version 330 core

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTextCoord;

out vec2 v_TextCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat3 u_Normal;

void main()
{
   gl_Position = u_Projection * u_View * u_Model * aPosition; 
   v_TextCoord = aTextCoord;
   v_FragPos = vec3(u_Model * aPosition);
   v_Normal = u_Normal * aNormal;
};