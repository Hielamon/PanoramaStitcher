#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

in vec2 FragPos;
const float PI = 3.1415926535897932384626433832795;

void main()
{	
	float theta = FragPos.x * PI;
	float phi = (0.5f - FragPos.y * 0.5f) * PI;
	vec3 normal = vec3(-sin(phi) * sin(theta), cos(phi), -sin(phi) * cos(theta));
	normal = normalize(normal);
	FragColor = vec4(normal*0.5f + vec3(0.5f), 1.0f);
	//FragColor = vec4(normal.y * 0.5f + 0.5f, 0.0f, 0.0f, 1.0f);
	//FragColor = vec4(normal.x * 0.5f + 0.5f,
	//				 normal.y * 0.5f + 0.5f,
	//				 normal.z * 0.5f + 0.5f, 
	//				 1.0f);
	FragColor = vec4(normal.x * 0.5f + 0.5f/*0.0f*/,
					 normal.y * 0.5f + 0.5f/*0.0f*/,
					 normal.z * 0.5f + 0.5f/*0.0f*/, 
					 1.0f);
}