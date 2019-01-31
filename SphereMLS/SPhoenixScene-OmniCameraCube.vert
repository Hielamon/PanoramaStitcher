#version 330 core
layout (location = 0) in vec3 aVertice;
layout (location = NORMAL_ATTR) in vec3 aNormal;

out vec3 Normal;

void main()
{
	vec4 worldPos = /*aModelMatrix * */vec4(aVertice, 1.0f);
	gl_Position = worldPos;
	Normal = aNormal;
}

