#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;
layout (location = 2) out uint Mask;


const float PI = 3.1415926535897932384626433832795;


in vec3 Normal;
uniform samplerCube cube_map1;
uniform usamplerCube cube_map2;//mask

void main()
{	
	vec3 normal = normalize(Normal);

	//vec2 texCoord = vec2((FragPos.x + 1.0f)*0.5f, (FragPos.y + 1.0f)*0.5f);
	//FragColor = texture(material.diffuse_maps, texCoord);
	//FragColor = texture(cube_map1, normal);
	//float mask = texture(cube_map2, normal).r;
	//if(mask != 0.0f)
	//	FragColor = vec4(texture(cube_map2, normal).r, 0.0f, 0.0f, 1.0f);
	//else
	//	FragColor = vec4(normal*0.5f + vec3(0.5f), 1.0f);
	Mask = uint(texture(cube_map2, normal).r);
	//Mask = uint(255);
	FragColor = texture(cube_map1, normal);
}