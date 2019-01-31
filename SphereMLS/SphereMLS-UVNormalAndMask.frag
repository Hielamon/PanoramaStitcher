#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;
layout (location = 2) out uint Mask;
layout (location = 3) out vec4 SNormal;

const float PI = 3.1415926535897932384626433832795;

layout (std140) uniform UnifyFEUBO
{
	float width, height; //2 * 4
	float fov, ksai; // 2 * 4
	float fx, fy, u0, v0; // 4 * 4
	float k1, k2, p1, p2; // 4 * 4
};

layout (std140) uniform ViewUBO
{
	mat4 projMatrix; // 16 * 4
	mat4 viewMatrix; // 16 * 4
	vec3 viewPos;    // 16
};

in vec2 FragPos;

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
	float theta = FragPos.x * PI;
	float phi = (0.5f - FragPos.y * 0.5f) * PI;
	vec3 normal = vec3(sin(phi) * sin(theta), cos(phi), -sin(phi) * cos(theta));
	normal = normalize(normal);
	SNormal = vec4(normal, 1.0f);

	mat3 R = mat3(viewMatrix);
	normal = R * normal;

	vec3 TexCoord;
	if(computeTexCoord(normal, TexCoord))
	{
		Mask = uint(255);
	}
	else
	{
		Mask = uint(0);
	}

	FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}