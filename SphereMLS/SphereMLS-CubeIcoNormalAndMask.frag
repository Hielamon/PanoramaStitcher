#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;
layout (location = 2) out uint Mask;
layout (location = 3) out vec4 SNormal;

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

in vec3 Normal;

bool determinantMask(vec3 normal)
{
	float halfFOV = 0.5f * (fov - 0.05f);
	float zmin = cos(halfFOV);
	if(-normal.z < zmin) return false;

	float x = normal.x / (-normal.z + ksai);
	float y = normal.y / (-normal.z + ksai);

	float r2 = x*x + y*y;
	
	float x_ = x*(1 + (k1 + r2*k2)*r2) + 2*p1*x*y + p2*(r2 + 2*x*x);
	float y_ = y*(1 + (k1 + r2*k2)*r2) + 2*p2*x*y + p1*(r2 + 2*y*y);

	float u = fx * x_ + u0;
	float v = fy * y_ + v0;

	float wInv = 1.0f / width;
	float hInv = 1.0f / height;

	float texX = u * wInv;
	float texY = 1.0f - v * hInv;

	float xOffset = 100 * wInv;
	float yOffset = 100 * hInv;

	if(texX > (1.0f - xOffset) || texX < xOffset || 
	   texY > (1.0f - yOffset) || texY < yOffset) return false;

	return true;
}

void main()
{	
	vec3 normal = vec3(0.0f, 0.0f, 1.0f);
	normal = normalize(Normal);
	SNormal = vec4(normal, 1.0f);

	mat3 R = mat3(viewMatrix);
	normal = R * normal;

	if(determinantMask(normal))
	{
		Mask = uint(255);
	}
	else
	{
		Mask = uint(0);
	}

	FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}