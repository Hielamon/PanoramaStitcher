#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added
//#define NORMAL_ATTR 1, if HAVE_NORMAL, this is necessary
//#define TEXCOORD_ATTR 2, if HAVE_TEXCOORD and HAVE_TEXTURE, this is necessary
//#define COLOR_ATTR 2, if HAVE_COLOR but not HAVE_TEXCOORD, this is necessary

layout (location = 0) in vec3 aVertice;

#if defined(HAVE_NORMAL)
layout (location = NORMAL_ATTR) in vec3 aNormal;
#endif

layout (location = MMATRIX_ATTR) in mat4 aModelMatrix;



out vec2 FragPos;

#if defined(HAVE_NORMAL)
out vec3 Normal;
#endif

void main()
{
	vec4 worldPos = vec4(aVertice, 1.0f);
	gl_Position = worldPos;
	FragPos = vec2(aVertice);

#if defined(HAVE_NORMAL)
	Normal = aNormal;
#endif
}

