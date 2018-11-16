#include <SPhoenix\MonitorWindow.h>
#include <SPhoenix\Manipulator.h>
#include <SPhoenix\Plane.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace SP;

enum SurfaceType
{
	UV_SURFACE, CUBE_SURFACE, ICO_SURFACE
};

std::vector<std::string> vSaveFilePreifx = {"UVSphere", "CubeSphere", "IcoSphere-z"};
SurfaceType surfaceType = ICO_SURFACE;
int edgeSize = 200;

int main(int argc, char *argv[])
{
	int width, height;
	std::shared_ptr<Scene> pScene = std::make_shared<Scene>();

	switch (surfaceType)
	{
	case UV_SURFACE:
	{
		width = 2 * edgeSize;
		height = edgeSize;
		glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
		std::shared_ptr<Plane> pPlane
			= std::make_shared<Plane>(2.0f, 2.0f, T);

		std::string vertShaderFile = "NormalSurface-UVSphere.vert";
		std::string fragShaderFile = "NormalSurface-UVSphere.frag";
		std::shared_ptr<ShaderProgram> pShader
			= std::make_shared<ShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pPlane, pShader);
	}
		break;
	case CUBE_SURFACE:
	{
		width = 4 * edgeSize;
		height = 3 * edgeSize;

		float yInterval = 2.0f / 3.0f, xInterval = 2.0f / 4.0f;
		std::vector<glm::vec3> vVertice(14);
		{
			int vertIdx = 0;
			for (int i = 0; i < 4; i++)
			{
				int jStart = 0, jEnd = 5;
				if (i == 0 || i == 3)
				{
					jStart = 1;
					jEnd = 3;
				}
				for (int j = jStart; j < jEnd; j++)
				{
					vVertice[vertIdx] = glm::vec3(j * xInterval - 1.0f,
												  1.0f - i * yInterval,
												  0.0f);
					vertIdx++;
				}
			}
		}
		std::vector<glm::vec3> vNormal(14);
		{
			vNormal[0] = glm::vec3(-1.0f, 1.0f, 1.0f);
			vNormal[1] = glm::vec3(1.0f, 1.0f, 1.0f);

			vNormal[2] = glm::vec3(-1.0f, 1.0f, 1.0f);
			vNormal[3] = glm::vec3(-1.0f, 1.0f, -1.0f);
			vNormal[4] = glm::vec3(1.0f, 1.0f, -1.0f);
			vNormal[5] = glm::vec3(1.0f, 1.0f, 1.0f);
			vNormal[6] = glm::vec3(-1.0f, 1.0f, 1.0f);

			vNormal[7] = glm::vec3(-1.0f, -1.0f, 1.0f);
			vNormal[8] = glm::vec3(-1.0f, -1.0f, -1.0f);
			vNormal[9] = glm::vec3(1.0f, -1.0f, -1.0f);
			vNormal[10] = glm::vec3(1.0f, -1.0f, 1.0f);
			vNormal[11] = glm::vec3(-1.0f, -1.0f, 1.0f);

			vNormal[12] = glm::vec3(-1.0f, -1.0f, 1.0f);
			vNormal[13] = glm::vec3(1.0f, -1.0f, 1.0f);
		}
		std::vector<GLuint> vIndice/*(6 * 2 * 3);*/
		{
			0, 3, 1, 1, 3, 4,

			2, 7, 3, 3, 7, 8,
			3, 8, 4, 4, 8, 9,
			4, 9, 5, 5, 9, 10,
			5, 10, 6, 6, 10, 11,

			8, 12, 9, 9, 12, 13
		};

		std::shared_ptr<VertexArray> pVA
			= std::make_shared<VertexArray>(vVertice, vIndice);
		pVA->addInstance();
		pVA->setNormals(vNormal);

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA);
			
		std::string vertShaderFile = "NormalSurface-CubeIcoSphere.vert";
		std::string fragShaderFile = "NormalSurface-CubeIcoSphere.frag";
		std::shared_ptr<ShaderProgram> pShader
			= std::make_shared<ShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pMesh, pShader);
	}
		break;
	case ICO_SURFACE:
	{
		width = 5.5f * edgeSize;
		height = std::sqrt(3.0f) * 0.5f * edgeSize * 3.0f;

		float yInterval = 2.0f / 3.0f, xInterval = 2.0f / 5.5f;
		float halfXInterval = xInterval * 0.5f;
		std::vector<glm::vec3> vVertice(22);
		{
			int vertIdx = 0;
			float vXStartOffset[4] = { halfXInterval, 0.0f, halfXInterval, xInterval };
			for (int i = 0; i < 4; i++)
			{
				int verticeN = 6;
				if (i == 0 || i == 3)
				{
					verticeN = 5;
				}
				float xStart = -1.0f + vXStartOffset[i];
				for (int j = 0; j < verticeN; j++)
				{
					vVertice[vertIdx] = glm::vec3(xStart,
												  1.0f - i * yInterval,
												  0.0f);
					vertIdx++;
					xStart += xInterval;
				}
			}
		}

		float a = std::sqrtf(50.0f - 10.0f * std::sqrtf(5.0f)) * 0.2 * 1.0f;
		float b = 0.5f * (std::sqrtf(5.0f) + 1.0f) * a;
		float a_2 = a * 0.5f, b_2 = b * 0.5f;
		std::vector<glm::vec3> vIcoVertice =
		{
			//Rectangle 1
			{ a_2,  b_2, 0.0f },
			{ a_2, -b_2, 0.0f },
			{ -a_2, -b_2, 0.0f },
			{ -a_2,  b_2, 0.0f },

			//Rectangle 2
			{ b_2, 0.0f, -a_2 },
			{ b_2, 0.0f,  a_2 },
			{ -b_2, 0.0f,  a_2 },
			{ -b_2, 0.0f, -a_2 },


			//Rectangle 2
			{ 0.0f,  a_2, -b_2 },
			{ 0.0f, -a_2, -b_2 },
			{ 0.0f, -a_2,  b_2 },
			{ 0.0f,  a_2,  b_2 }

		};

		std::vector<GLuint> vNormalIndices =
		{
			0, 0, 0, 0, 0,
			5, 4, 8, 3, 11, 5,
			1, 9, 7, 6, 10, 1,
			2, 2, 2, 2, 2
		};

		float rotAngle = std::atan2(std::sqrt(5.0f) - 1.0f, 2.0f);

		glm::mat4 IcoRot = glm::rotate(glm::mat4(1.0f), rotAngle, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat3 IcoRot3 = glm::mat3(IcoRot);
		std::vector<glm::vec3> vNormal(22);
		{
			for (size_t i = 0; i < 22; i++)
			{
				vNormal[i] = IcoRot3 * vIcoVertice[vNormalIndices[i]];
			}
		}
		std::vector<GLuint> vIndice/*(6 * 2 * 3);*/
		{
			0, 5, 6, 1, 6, 7, 2, 7, 8, 3, 8, 9, 4, 9, 10,

			5, 11, 6, 6, 11, 12, 6, 12, 7, 7, 12, 13, 7, 13, 8, 8, 13, 14, 8, 14, 9, 9, 14, 15, 9, 15, 10, 10, 15, 16,

			11, 17, 12, 12, 18, 13, 13, 19, 14, 14, 20, 15, 15, 21, 16
		};

		std::shared_ptr<VertexArray> pVA
			= std::make_shared<VertexArray>(vVertice, vIndice);
		pVA->addInstance();
		pVA->setNormals(vNormal);

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA);

		std::string vertShaderFile = "NormalSurface-CubeIcoSphere.vert";
		std::string fragShaderFile = "NormalSurface-CubeIcoSphere.frag";
		std::shared_ptr<ShaderProgram> pShader
			= std::make_shared<ShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pMesh, pShader);
	}
		break;
	default:
		break;
	}

	std::shared_ptr<MonitorWindow> pWindow
		= std::make_shared<MonitorWindow>(vSaveFilePreifx[surfaceType], width, height);

	pWindow->setScene(pScene);

	std::shared_ptr<MonitorManipulator> pManipulator
		= std::make_shared<MonitorManipulator>(pWindow);
	pWindow->setManipulator(pManipulator);
	pWindow->setDoShowText(false);
	pWindow->run();

	return 0;
}