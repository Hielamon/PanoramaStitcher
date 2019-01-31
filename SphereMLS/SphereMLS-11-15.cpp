#include <SPhoenix\MonitorWindow.h>
#include <SPhoenix\Manipulator.h>
#include <SPhoenix\Plane.h>
#include <SPhoenix\AxisMesh.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "FishEyeInitWarp.h"
#include "TransparentMesh.h"

using namespace SP;

std::vector<std::string> vSaveFilePreifx = { "UVSphere", "CubeSphere", "IcoSphere" };
SurfaceType surfaceType = CUBE_SURFACE;
int edgeSize =500;

int main(int argc, char *argv[])
{
	int OmniTexWidth = 0, OmniTexHeight = 0;
	int width = 1440, height = 900;
	std::shared_ptr<Scene> pScene = std::make_shared<Scene>();

	std::string fishImgPath = "D:\\Academic-Research\\Datas\\SLAMDatas\\TUM-SLAM-Omni\\T1\\T1_orig\\images\\1424198581.180664660.png";
	std::shared_ptr<Material> pMaterial = std::make_shared<Material>();
	std::shared_ptr<Texture> pTexture =
		std::make_shared<Texture>(fishImgPath, TextureType::Tex_DIFFUSE);
	pMaterial->addTexture(pTexture);

	switch (surfaceType)
	{
	case UV_SURFACE:
	{
		OmniTexWidth = 2 * edgeSize;
		OmniTexHeight = edgeSize;
		glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
		std::shared_ptr<Plane> pPlane
			= std::make_shared<Plane>(2.0f, 2.0f, T);

		pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);
		pPlane->setMaterial(pMaterial);

		std::string vertShaderFile = "SphereMLS-UVTexture.vert";
		std::string fragShaderFile = "SphereMLS-UVTexture.frag";
		std::shared_ptr<UnifyFEShaderProgram> pShader
			= std::make_shared<UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pPlane, pShader);
	}
	break;
	case CUBE_SURFACE:
	{
		OmniTexWidth = 4 * edgeSize;
		OmniTexHeight = 3 * edgeSize;

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

		/*(6 * 2 * 3);*/
		std::vector<GLuint> vIndice = 
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
		std::vector<glm::vec2> vTexCoords(vVertice.size(), glm::vec2(0.0, 0.0));
		pVA->setTexCoords(vTexCoords);

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMaterial);
			
		std::string vertShaderFile = "SphereMLS-CubeIcoTexture.vert";
		std::string fragShaderFile = "SphereMLS-CubeIcoTexture.frag";
		std::shared_ptr<UnifyFEShaderProgram> pShader
			= std::make_shared<UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pMesh, pShader);

		std::string vertShaderFile2D = "SphereMLS-CubeIco2D.vert";
		std::string fragShaderFile2D = "SphereMLS-CubeIco2D.frag";
		std::shared_ptr<ShaderProgram> pShader2D
			= std::make_shared<ShaderProgram>(vertShaderFile2D, fragShaderFile2D);

		std::vector<GLuint> vLineIndice = 
		{
			0, 1, 2, 6, 7, 11, 12, 13,
			
			2, 7, 0, 12, 1, 13, 5, 10, 6, 11
		};
		std::shared_ptr<VertexArray> pVALine
			= std::make_shared<VertexArray>(vVertice, vLineIndice, LINES);
		pVALine->addInstance();

		std::shared_ptr<Mesh> pMeshLine = std::make_shared<Mesh>(pVALine);
		glm::mat4 slightT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		pMeshLine->setInstanceMMatrix(slightT, 0);
		//pScene->addMesh(pMeshLine, pShader2D);
	}
	break;
	case ICO_SURFACE:
	{
		OmniTexWidth = 5.5f * edgeSize;
		OmniTexHeight = std::sqrt(3.0f) * 0.5f * edgeSize * 3.0f;

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
		std::vector<glm::vec2> vTexCoords(vVertice.size(), glm::vec2(0.0, 0.0));
		pVA->setTexCoords(vTexCoords);

		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMaterial);

		std::string vertShaderFile = "SphereMLS-CubeIcoTexture.vert";
		std::string fragShaderFile = "SphereMLS-CubeIcoTexture.frag";
		std::shared_ptr<UnifyFEShaderProgram> pShader
			= std::make_shared<UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);
		pScene->addMesh(pMesh, pShader);

		std::string vertShaderFile2D = "SphereMLS-CubeIco2D.vert";
		std::string fragShaderFile2D = "SphereMLS-CubeIco2D.frag";
		std::shared_ptr<ShaderProgram> pShader2D
			= std::make_shared<ShaderProgram>(vertShaderFile2D, fragShaderFile2D);

		std::vector<GLuint> vLineIndice =
		{
			0, 5, 1, 11, 2, 17, 3, 18, 4, 19, 10, 20, 16, 21,
			5, 17, 0, 18, 1, 19, 2, 20, 3, 21, 4, 16, 
			5, 10, 11, 16
		};
		std::shared_ptr<VertexArray> pVALine
			= std::make_shared<VertexArray>(vVertice, vLineIndice, LINES);
		pVALine->addInstance();

		std::shared_ptr<Mesh> pMeshLine = std::make_shared<Mesh>(pVALine);
		glm::mat4 slightT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.f, -1.0f));
		pMeshLine->setInstanceMMatrix(slightT, 0);
		pScene->addMesh(pMeshLine, pShader2D);
	}
	break;
	default:
		break;
	}

	std::shared_ptr<MonitorWindow> pWindow
		= std::make_shared<MonitorWindow>(vSaveFilePreifx[surfaceType], width, height);

	std::shared_ptr<MonitorManipulator> pManipulator
		= std::make_shared<MonitorManipulator>(pWindow);
	pWindow->setManipulator(pManipulator);

	pWindow->setScene(pScene);

	int minWidth = width * 0.25, minHeight = height * 0.25;
	std::shared_ptr<UnifyFECamera> pCamera =
		std::make_shared<UnifyFECamera>(minWidth, minHeight, width - minWidth, 0);

	std::string settingPath = "D:\\Academic-Research\\Datas\\SLAMDatas\\TUM-SLAM-Omni\\T1\\calibrationOrig.yaml";
	pCamera->readFromTUMSettings(settingPath);
	pCamera->setCustomBufferSize(OmniTexWidth, OmniTexHeight);
	pCamera->setShowCanvas(false);

	pWindow->getDefaultCamera()->setShowCanvas(false);

	pWindow->addCamera(pCamera);
	pWindow->runOnce();

	/*std::string saveFullPath = vSaveFilePreifx[surfaceType] + ".png";
	std::shared_ptr<unsigned char> pData;
	int w, h, c;
	pCamera->readColorBuffer(pData, w, h, c);
	SOIL_save_image(saveFullPath.c_str(), SOIL_SAVE_TYPE_BMP,
					w, h, c, pData.get());*/

	/*std::string saveFullPath = vSaveFilePreifx[surfaceType] + "-Mask.png";
	std::shared_ptr<unsigned char> pData;
	int w, h, c;
	pCamera->readMaskBuffer(pData, w, h, c);
	SOIL_save_image(saveFullPath.c_str(), SOIL_SAVE_TYPE_BMP,
					w, h, c, pData.get());*/

	std::shared_ptr<Scene> pSceneShow = std::make_shared<Scene>();

	{
		std::shared_ptr<MaterialFBO> pMaterialFBOMask = pCamera->getMaterialFBOMask();
		std::shared_ptr<MaterialFBO> pMaterialFBO = pCamera->getMaterialFBO();
		GLuint maskBuffer;
		pMaterialFBOMask->getTexbuffer(1, maskBuffer);
		pMaterialFBO->setTexbuffer(0, maskBuffer);

		std::shared_ptr<Plane> pPlane = std::make_shared<Plane>(2, 2);
		pPlane->setMaterial(pMaterialFBO);
		pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);

		std::string vertShaderFile2D = "SphereMLS-dilate.vert";
		std::string fragShaderFile2D = "SphereMLS-dilate.frag";
		std::shared_ptr<UnifyFEShaderProgram> pShader2D
			= std::make_shared<UnifyFEShaderProgram>(vertShaderFile2D, fragShaderFile2D);

		pSceneShow->addMesh(pPlane, pShader2D);
	}

	/*pWindow->getDefaultCamera()->setShowCanvas(true);
	pCamera->setDoRender(true);
	pCamera->setShowCanvas(true);*/

	pWindow->setScene(pSceneShow);
	for (size_t i = 0; i < 5; i++)
	{
		pWindow->runOnce();
	}
	/*{
		std::string saveFullPath = vSaveFilePreifx[surfaceType] + "-Dilate100.png";
		std::shared_ptr<unsigned char> pData;
		int w, h, c;
		pCamera->readColorBuffer(pData, w, h, c);
		SOIL_save_image(saveFullPath.c_str(), SOIL_SAVE_TYPE_BMP,
						w, h, c, pData.get());
	}
	{
		std::string saveFullPath = vSaveFilePreifx[surfaceType] + "-Mask100.png";
		std::shared_ptr<unsigned char> pData;
		int w, h, c;
		pCamera->readMaskBuffer(pData, w, h, c);
		SOIL_save_image(saveFullPath.c_str(), SOIL_SAVE_TYPE_BMP,
						w, h, c, pData.get());
	}
	return 0;*/

	pWindow->getDefaultCamera()->setShowCanvas(true);
	pCamera->setDoRender(false);
	pSceneShow->reset();

	switch (surfaceType)
	{
	case UV_SURFACE:
		break;
	case CUBE_SURFACE:
	{
		std::shared_ptr<MaterialFBO> pMaterialFBO = pCamera->getMaterialFBO();
		pMaterialFBO->setShininessStrength(0.0);
		
		CubeGrid cubeGrid(4);
		
		std::vector<glm::vec3> vVertice = cubeGrid.getFlatSphereVertice();
		std::vector<GLuint> vIndice;
		std::vector<glm::vec2> vTexCoordOnFlat = cubeGrid.vTexCoordOnFlat;

		std::shared_ptr<VertexArray> pVA = std::make_shared<VertexArray>(vVertice);
		//pVA->setNormals(vVertice);
		//pVA->setColors()
		
		pVA->setTexCoords(vTexCoordOnFlat);
		glm::mat4 rotInvZ = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
		pVA->addInstance(rotInvZ);

		std::string vertShaderFile = "SPhoenixScene-AbsNormal.vert";
		std::string fragShaderFile = "SPhoenixScene-AbsNormal.frag";
		std::shared_ptr<ShaderProgram> pShader
			= std::make_shared<ShaderProgram>(vertShaderFile, fragShaderFile);
		//
		std::shared_ptr<Mesh> pMesh = std::make_shared<Mesh>(pVA, pMaterialFBO);
		//std::shared_ptr<TMesh> pTMesh = std::make_shared<TMesh>(*pMesh);
		pSceneShow->addMesh(pMesh, pShader);

		std::shared_ptr<AxisMesh> pAxisMesh = std::make_shared<AxisMesh>(0.1f);
		pSceneShow->addMesh(pAxisMesh);

		//std::shared_ptr<Plane> pPlane = std::make_shared<Plane>(4, 3);
		//pMaterial->clearUploaded();
		//pPlane->setMaterial(pMaterialFBO);
		//pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);
		//pSceneShow->addMesh(pPlane, pShader);
	}
		break;
	case ICO_SURFACE:
		break;
	default:
		break;
	}

	pWindow->setScene(pSceneShow);
	pWindow->run();

	return 0;

}