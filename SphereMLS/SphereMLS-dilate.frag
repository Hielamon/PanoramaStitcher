#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 2) out uint Mask;

in vec2 TexCoord;

struct Material
{
	sampler2D diffuse_maps;
	usampler2D ambient_maps;
};

uniform Material material;

float xoffset = 1.0 / 2000.f;
float yoffset = 1.0 / 1500.f;

void main()
{	
	//vec2 TexCoord = gl_FragCoord.xy;
	uint mask = texture(material.ambient_maps, TexCoord).r;
	vec4 diffColor = texture(material.diffuse_maps, TexCoord);
	//vec4 resultColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 resultColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	//xoffset *= 0.5f;
	//yoffset *= 0.5f;

	vec2 offset[4] = vec2[](
		vec2(-xoffset, 0.0f),
		vec2(xoffset, 0.0f),
		vec2(0.0f, yoffset),
		vec2(0.0f, -yoffset)
	);

	uint falseValue = uint(0);

	Mask = mask;

	if(mask != falseValue)
	{
		resultColor = diffColor;
	}
	else
	{
		int validCount = 0;
		vec4 validColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		for(int i = 0; i < 4; i++)
		{
			vec2 texcoord_near = TexCoord + offset[i];
			uint mask_near = texture(material.ambient_maps, texcoord_near).r;
			if(mask_near != falseValue)
			{
				validCount++;
				validColor = texture(material.diffuse_maps, texcoord_near);
				break;
			}
		}

		if(validCount > 0)
		{
			float countInv = 1.0f / float(validCount);
			resultColor = validColor;

			Mask = uint(255);
		}
	}

	//FragColor = diffColor;
	FragColor = resultColor;
}