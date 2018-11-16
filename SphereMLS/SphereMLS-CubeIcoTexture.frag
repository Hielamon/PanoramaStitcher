#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;
layout (location = 2) out uint Mask;

//const float PI = 3.1415926535897932384626433832795;

layout (std140) uniform UnifyFEUBO
{
	float width, height; //2 * 4
	float fov, ksai; // 2 * 4
	float fx, fy, u0, v0; // 4 * 4
	float k1, k2, p1, p2; // 4 * 4
};

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

struct Material
{
	sampler2DArray diffuse_maps;
};

in vec2 FragPos;
uniform Material material;

bool computeTexCoord(vec3 normal, inout vec3 texcoord)
{
	float halfFOV = 0.5f * fov;
	float zmin = cos(halfFOV);
	if(-normal.z < zmin) return false;

	float x = normal.x / (-normal.z + ksai);
	float y = normal.y / (-normal.z + ksai);

	float r2 = x*x + y*y;
	
	float x_ = x*(1 + (k1 + r2*k2)*r2) + 2*p1*x*y + p2*(r2 + 2*x*x);
	float y_ = y*(1 + (k1 + r2*k2)*r2) + 2*p2*x*y + p1*(r2 + 2*y*y);

	float u = fx * x_ + u0;
	float v = fy * y_ + v0;

	float texX = u / width;
	float texY = 1.0f - v / height;

	if(texX > 1.0f || texX < 0.0f || texY > 1.0f || texY < 0.0f) return false;

	texcoord = vec3(texX, texY, 0.0f);
	return true;
}

void main()
{	
	vec3 normal = vec3(0.0f, 0.0f, 1.0f);
#if defined(HAVE_NORMAL)
	normal = normalize(Normal);
#endif
	
	vec3 TexCoord;
	//vec4 resultColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	//vec4 resultColor = vec4(0.5f, 0.5f, 0.5f, 0.5f);
	vec4 resultColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	if(computeTexCoord(normal, TexCoord))
	{
		resultColor= vec4(vec3(texture(material.diffuse_maps, TexCoord).x), 1.0f);
		Mask = uint(255);
	}
	else
	{
		Mask = uint(0);
	}

	Mask = uint(255);
	
	FragColor = resultColor;
	//FragColor = vec4(fov, 0.0f, 0.0f, 1.0f);
}