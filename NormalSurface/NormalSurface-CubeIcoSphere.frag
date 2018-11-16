#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

in vec2 FragPos;

void main()
{	
	vec3 normal = vec3(0.0f, 0.0f, 1.0f);
#if defined(HAVE_NORMAL)
	normal = normalize(Normal);
#endif
	
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

	//FragColor = vec4(normal.x * 0.5f + 0.5f,
	//				 0.0f,
	//				 0.0f, 
	//				 1.0f);

	//FragColor = vec4(0.0f,
	//				 normal.y * 0.5f + 0.5f,
	//				 0.0f, 
	//				 1.0f);

	FragColor = vec4(0.0f,
					 0.0f,
					 normal.z * 0.5f + 0.5f, 
					 1.0f);
}