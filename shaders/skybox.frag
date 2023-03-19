#version 410 core
in vec3 texCoords;
in vec4 fragPosEye;
out vec4 fragColor;
uniform samplerCube skybox;
uniform int fogEnable;

void main()
{
	fragColor = texture(skybox, texCoords);
	vec3 fogColor = vec3(0.4f, 0.4f, 0.4f);
	
};