#version 410 core

layout (location = 0) in vec3 aPos;

out vec3 texCoords;
out vec4 fragPosEye;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	texCoords = aPos;
	fragPosEye = view * model * vec4(aPos, 1.f);
	vec4 pos = projection * view * model * vec4(aPos, 1.f);
	gl_Position = pos.xyww;
};