#version 330 core

//#define MMATRIX_ATTR 3, this macro must be added
//#define NORMAL_ATTR 1, if HAVE_NORMAL, this is necessary
//#define TEXCOORD_ATTR 2, if HAVE_TEXCOORD and HAVE_TEXTURE, this is necessary
//#define COLOR_ATTR 2, if HAVE_COLOR but not HAVE_TEXCOORD, this is necessary

layout (location = 0) in vec3 aVertice;

void main()
{
	vec4 worldPos = vec4(aVertice.x, aVertice.y, -1.0f, 1.0f);
	gl_Position = worldPos;
}

