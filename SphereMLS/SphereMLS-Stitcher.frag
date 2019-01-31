#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out uint MeshID;
layout (location = 2) out uint Mask;

#if defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)
in vec2 TexCoord;
#elif defined(HAVE_COLOR)
in vec4 VertexColor;
#endif

#if defined(HAVE_NORMAL)
in vec3 Normal;
#endif

struct Light
{
	vec3 position;
	vec3 lightColor;
};

//#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)
struct Material
{
#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)

#if defined(AMBIENT_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray ambient_maps;
#elif !defined(DIFFUSE_TEXTURE) || !defined(HAVE_TEXCOORD)
	vec4 uAmbient;
#endif

#if defined(DIFFUSE_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray diffuse_maps;
#else
	vec4 uDiffuse;
#endif

#if defined(SPECULAR_TEXTURE) && defined(HAVE_TEXCOORD)
	sampler2DArray specular_maps;
#else
	vec4 uSpecular;
#endif

#endif
	
	float uShininess;
	float uShininessStrength;
};

uniform Material material;
//#endif

uniform Light light;
uniform uint uMeshID;

in vec3 FragPos;
in vec3 ViewPos;

void main()
{	
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 result = vec3(0.0f, 0.0f, 0.0f);
	float alpha	= 1.0f;
	bool bInner = false;
#if defined(HAVE_NORMAL)
	float ambientFactor = 0.7f;

	vec3 normal = normalize(Normal);
	vec3 lightDir = normalize(ViewPos - FragPos);
	//vec3 lightDir = normalize(light.position - FragPos);
	float diffuseFactor = max(dot(normal, lightDir), 0.0f);
	if(dot(normal, lightDir) < 0.0f)
		bInner = true;
	//float diffuseFactor = dot(normal, lightDir);
	diffuseFactor = abs(dot(normal, lightDir));
	diffuseFactor *= (1.0f - ambientFactor);

	//Blinn-Phong specular
	vec3 viewDir = normalize(ViewPos - FragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specularFactor = pow(max(dot(halfwayDir, normal), 0.0f), /*32.0f*/material.uShininess);
	//specularFactor *= 0.0;
	specularFactor *= material.uShininessStrength;
#else
	float ambientFactor = 0.2f;
	float diffuseFactor = 0.8f;
	float specularFactor = 0.0f;
#endif

	vec3 diffuse = vec3(0.0f, 0.0f, 0.0f);
	vec3 ambient = diffuse;
	vec3 specular = vec3(0.0f, 0.0f, 0.0f);

#if (defined(HAVE_TEXTURE) && defined(HAVE_TEXCOORD)) || !defined(HAVE_COLOR)

#if defined(DIFFUSE_TEXTURE) && defined(HAVE_TEXCOORD)
	diffuse = vec3(1.0f, 1.0f, 1.0f);
	ivec3 diffuse_size = textureSize(material.diffuse_maps, 0);
	ivec3 ambient_size = textureSize(material.ambient_maps, 0);

	if(diffuse_size != diffuse_size)
		diffuse = vec3(1.0f, 0.0f, 0.0f);
	int layers = diffuse_size.z;
	float totalCount = 0.0f;
	vec3 layer_color = vec3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < layers; i++)
	{
		vec3 texCoord3D = vec3(TexCoord, float(i));
		float mask = texture(material.ambient_maps, texCoord3D).r;
		if( mask != 0.0f)
		{
			layer_color += vec3(texture(material.diffuse_maps, texCoord3D));
			totalCount += 1.0f;
		}
	}

	if(totalCount > 0.0f)
	{
		diffuse = layer_color * float(1.0f / totalCount);
		Mask = uint(255);
	}
	else
	{
		diffuse = vec3(0.0f, 0.0f, 0.0f);
		Mask = uint(0);
	}
	ambient = diffuse;
#else
	diffuse = vec3(material.uDiffuse);
	//if(bInner) 
	//{
	//	diffuse = vec3(1.0f, 0.0f, 0.0f);
	//}
		
	alpha = material.uDiffuse.w;
#endif

//#if defined(AMBIENT_TEXTURE) && defined(HAVE_TEXCOORD)
//	ambient = vec3(1.0f, 1.0f, 1.0f);
//	ivec3 ambient_size = textureSize(material.ambient_maps, 0);
//	int ambient_layers = ambient_size.z;
//	for(int i = 0; i < ambient_layers; i++)
//	{
//		ambient *= vec3(texture(material.ambient_maps, vec3(TexCoord, float(i))));
//	}
//#elif !defined(DIFFUSE_TEXTURE) || !defined(HAVE_TEXCOORD)
//	ambient = vec3(material.uAmbient);
//#endif

#if defined(SPECULAR_TEXTURE) && defined(HAVE_TEXCOORD)
	specular = vec3(1.0f, 1.0f, 1.0f);
	ivec3 specular_size = textureSize(material.specular_maps, 0);
	int specular_layers = specular_size.z;
	for(int i = 0; i < specular_layers; i++)
	{
		specular *= vec3(texture(material.specular_maps, vec3(TexCoord, float(i))));
	}
#else
	specular = vec3(material.uSpecular);
#endif

#else
//Use the vertex color
	diffuse = vec3(VertexColor);
	ambient = diffuse;
	alpha = VertexColor.a;
	//specular = diffuse;
#endif

	result = (ambientFactor * ambient + diffuseFactor * diffuse) * lightColor;
	result += specularFactor * specular * lightColor;
	//result = vec3(testV, material.uShininessStrength, 0.0f);
	//alpha = 0.5;
	FragColor = vec4(result/*.z, result.y, result.x*/, alpha);
	MeshID = uMeshID;
}