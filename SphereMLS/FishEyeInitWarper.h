#pragma once
#include <OpencvCommon.h>
#include <SPhoenix\MonitorWindow.h>

#include "FishEyeCamera.h"
#include "FishEyeShader.h"

enum SurfaceType
{
	UV_SURFACE, CUBE_SURFACE, ICO_SURFACE
};

class FishEyeInitWarper
{
public:
	FishEyeInitWarper(int edgeSize = 500, SurfaceType surfaceType = CUBE_SURFACE)
	{
		mpWindow = std::make_shared<SP::MonitorWindow>("", 1, 1, false);
		mpWindow->getDefaultCamera()->setDoRender(false);
		mpScene = std::make_shared<SP::Scene>();
		OmniTexWidth = OmniTexHeight = 0;

		mbDrawGridLine = false;
		mDilateTimes = 0;
		mSurfaceType = surfaceType;

		std::shared_ptr<SP::Mesh> pMesh;

		std::shared_ptr<SP::Material> pMaterial = std::make_shared<SP::Material>();
		std::shared_ptr<SP::Texture> pTexture =
			std::make_shared<SP::Texture>(SP::TextureType::Tex_DIFFUSE);
		pMaterial->addTexture(pTexture);

		switch (surfaceType)
		{
		case UV_SURFACE:
		{
			OmniTexWidth = 2 * edgeSize;
			OmniTexHeight = edgeSize;
			glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
			std::shared_ptr<SP::Plane> pPlane
				= std::make_shared<SP::Plane>(2.0f, 2.0f, T);
			pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);

			std::string vertShaderFile = "SphereMLS-UVTexture.vert";
			std::string fragShaderFile = "SphereMLS-UVTexture.frag";
			std::shared_ptr<SP::UnifyFEShaderProgram> pShader
				= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);

			pMesh = pPlane;
			pMesh->setMaterial(pMaterial);
			mpScene->addMesh(pPlane, pShader);
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

			std::shared_ptr<SP::VertexArray> pVA
				= std::make_shared<SP::VertexArray>(vVertice, vIndice);
			pVA->addInstance();
			pVA->setNormals(vNormal);
			std::vector<glm::vec2> vTexCoords(vVertice.size(), glm::vec2(0.0, 0.0));
			pVA->setTexCoords(vTexCoords);

			pMesh = std::make_shared<SP::Mesh>(pVA);

			std::string vertShaderFile = "SphereMLS-CubeIcoTexture.vert";
			std::string fragShaderFile = "SphereMLS-CubeIcoTexture.frag";
			std::shared_ptr<SP::UnifyFEShaderProgram> pShader
				= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);
			pMesh->setMaterial(pMaterial);
			mpScene->addMesh(pMesh, pShader);

			if (mbDrawGridLine)
			{
				std::string vertShaderFile2D = "SphereMLS-CubeIco2D.vert";
				std::string fragShaderFile2D = "SphereMLS-CubeIco2D.frag";
				std::shared_ptr<SP::ShaderProgram> pShader2D
					= std::make_shared<SP::ShaderProgram>(vertShaderFile2D, fragShaderFile2D);

				std::vector<GLuint> vLineIndice =
				{
					0, 1, 2, 6, 7, 11, 12, 13,

					2, 7, 0, 12, 1, 13, 5, 10, 6, 11
				};
				std::shared_ptr<SP::VertexArray> pVALine
					= std::make_shared<SP::VertexArray>(vVertice, vLineIndice, SP::LINES);
				pVALine->addInstance();

				std::shared_ptr<SP::Mesh> pMeshLine = std::make_shared<SP::Mesh>(pVALine);
				glm::mat4 slightT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.1f));
				pMeshLine->setInstanceMMatrix(slightT, 0);
				mpScene->addMesh(pMeshLine, pShader2D);
			}
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

			std::shared_ptr<SP::VertexArray> pVA
				= std::make_shared<SP::VertexArray>(vVertice, vIndice);
			pVA->addInstance();
			pVA->setNormals(vNormal);
			std::vector<glm::vec2> vTexCoords(vVertice.size(), glm::vec2(0.0, 0.0));
			pVA->setTexCoords(vTexCoords);

			pMesh = std::make_shared<SP::Mesh>(pVA);

			std::string vertShaderFile = "SphereMLS-CubeIcoTexture.vert";
			std::string fragShaderFile = "SphereMLS-CubeIcoTexture.frag";
			std::shared_ptr<SP::UnifyFEShaderProgram> pShader
				= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);
			pMesh->setMaterial(pMaterial);
			mpScene->addMesh(pMesh, pShader);

			if (mbDrawGridLine)
			{
				std::string vertShaderFile2D = "SphereMLS-CubeIco2D.vert";
				std::string fragShaderFile2D = "SphereMLS-CubeIco2D.frag";
				std::shared_ptr<SP::ShaderProgram> pShader2D
					= std::make_shared<SP::ShaderProgram>(vertShaderFile2D, fragShaderFile2D);

				std::vector<GLuint> vLineIndice =
				{
					0, 5, 1, 11, 2, 17, 3, 18, 4, 19, 10, 20, 16, 21,
					5, 17, 0, 18, 1, 19, 2, 20, 3, 21, 4, 16,
					5, 10, 11, 16
				};
				std::shared_ptr<SP::VertexArray> pVALine
					= std::make_shared<SP::VertexArray>(vVertice, vLineIndice, SP::LINES);
				pVALine->addInstance();

				std::shared_ptr<SP::Mesh> pMeshLine = std::make_shared<SP::Mesh>(pVALine);
				glm::mat4 slightT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, -0.1f));
				pMeshLine->setInstanceMMatrix(slightT, 0);
				mpScene->addMesh(pMeshLine, pShader2D);
			}

		}
		break;
		default:
			break;
		}

		if (pMesh.use_count() != 0)
		{
			mpMesh = pMesh;
		}

		mpWindow->setScene(mpScene);
		mpCamera = std::make_shared<SP::UnifyFECamera>(1, 1, 0, 0);
		mpCamera->setCustomBufferSize(OmniTexWidth, OmniTexHeight);
		mpCamera->setShowCanvas(false);
		mpWindow->addCamera(mpCamera);
	}
	~FishEyeInitWarper() {}

	void setDilateTimes(int dilateTimes = 0)
	{
		mDilateTimes = dilateTimes;
	}

	void setDoDrawGridLine(bool bDrawGridLine = false)
	{
		mbDrawGridLine = bDrawGridLine;
	}

	void setTUMFishEye(const std::string &TUMFishEyeSettingPath)
	{
		mpCamera->readFromTUMSettings(TUMFishEyeSettingPath);
	}

	void setHLFishEye(const std::string &HLFishEyeSettingPath)
	{
		mpCamera->readFromHLSetting(HLFishEyeSettingPath);
	}

	void renderNormalAndMask()
	{
		//produce the mask and surface normal 
		std::shared_ptr<SP::Scene> pSceneTemp = std::make_shared<SP::Scene>();
		{
			switch (mSurfaceType)
			{
			case UV_SURFACE:
			{
				std::string vertShaderFile = "SphereMLS-UVNormalAndMask.vert";
				std::string fragShaderFile = "SphereMLS-UVNormalAndMask.frag";
				std::shared_ptr<SP::UnifyFEShaderProgram> pShader
					= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);

				pSceneTemp->addMesh(mpMesh, pShader);
			}
				break;
			case CUBE_SURFACE:
			case ICO_SURFACE:
			{
				std::string vertShaderFile = "SphereMLS-CubeIcoNormalAndMask.vert";
				std::string fragShaderFile = "SphereMLS-CubeIcoNormalAndMask.frag";
				std::shared_ptr<SP::UnifyFEShaderProgram> pShader
					= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile, fragShaderFile);

				pSceneTemp->addMesh(mpMesh, pShader);
			}
				break;
			default:
				break;
			}
			
		}
		mpWindow->setScene(pSceneTemp);
		mpWindow->runOnce();
		mpWindow->setScene(mpScene);

		int w, h, c;
		std::shared_ptr<GLfloat> pNormalData;
		mpCamera->readSNormalBuffer(pNormalData, w, h, c);

		cv::Mat warpedNormal = cv::Mat(h, w, CV_32FC3, pNormalData.get());
		warpedNormal.copyTo(mWarpedNormal);

		std::shared_ptr<unsigned char> pDataMask;
		mpCamera->readMaskBuffer(pDataMask, w, h, c);
		cv::Mat warpedMask = cv::Mat(h, w, CV_8UC1, pDataMask.get());
		warpedMask.copyTo(mWarpedMask);
	}

	void renderFishEyeImage(const std::string &imagePath)
	{
		std::shared_ptr<SP::Material> pMaterial = mpMesh->getMaterial();
		std::shared_ptr<SP::Texture> pTexture =
			std::make_shared<SP::Texture>(imagePath, SP::TextureType::Tex_DIFFUSE);

		pMaterial->replaceTexture(pTexture, 0);

		mpWindow->runOnce();

		//Do the dilate implementation
		if (mDilateTimes > 0)
		{
			std::shared_ptr<SP::Scene> pSceneShow = std::make_shared<SP::Scene>();
			{
				std::shared_ptr<SP::MaterialFBO> pMaterialFBOMask = mpCamera->getMaterialFBOMask();
				std::shared_ptr<SP::MaterialFBO> pMaterialFBO = mpCamera->getMaterialFBO();
				GLuint maskBuffer;
				pMaterialFBOMask->getTexbuffer(1, maskBuffer);
				pMaterialFBO->setTexbuffer(0, maskBuffer);

				std::shared_ptr<SP::Plane> pPlane = std::make_shared<SP::Plane>(2, 2);
				pPlane->setMaterial(pMaterialFBO);
				pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);

				std::string vertShaderFile2D = "SphereMLS-dilate.vert";
				std::string fragShaderFile2D = "SphereMLS-dilate.frag";
				std::shared_ptr<SP::UnifyFEShaderProgram> pShader2D
					= std::make_shared<SP::UnifyFEShaderProgram>(vertShaderFile2D, fragShaderFile2D);

				pSceneShow->addMesh(pPlane, pShader2D);
			}
			mpWindow->setScene(pSceneShow);
			for (size_t i = 0; i < mDilateTimes; i++)
			{
				mpWindow->runOnce();
			}

			mpWindow->setScene(mpScene);
		}

		int w, h, c;
		std::shared_ptr<unsigned char> pData;
		mpCamera->readColorBuffer(pData, w, h, c);

		cv::Mat warpedImage = cv::Mat(h, w, CV_8UC3, pData.get());
		warpedImage.copyTo(mWarpedImage);

		
	}

	cv::Mat getWarpedImage()
	{
		return mWarpedImage;
	}

	cv::Mat getWarpedNormal()
	{
		return mWarpedNormal;
	}

	cv::Mat getWarpedMask()
	{
		return mWarpedMask;
	}

	cv::Mat getWarpedSurfaceMask()
	{
		int w, h, c;
		std::shared_ptr<unsigned char> pDataMask;
		mpCamera->readMaskBuffer(pDataMask, w, h, c);
		cv::Mat warpedMask = cv::Mat(h, w, CV_8UC1, pDataMask.get());
		warpedMask.copyTo(mWarpedSurfaceMask);

		return mWarpedSurfaceMask;
	}

	std::shared_ptr<SP::UnifyFECamera> getUnifyFECamera()
	{
		return mpCamera;
	}

private:
	cv::Mat mWarpedImage;
	cv::Mat mWarpedNormal;
	cv::Mat mWarpedSurfaceMask;
	cv::Mat mWarpedMask;

	std::shared_ptr<SP::MonitorWindow> mpWindow;
	std::shared_ptr<SP::Mesh> mpMesh;
	std::shared_ptr<SP::Scene> mpScene;
	std::shared_ptr<SP::UnifyFECamera> mpCamera;

	int OmniTexWidth, OmniTexHeight;
	bool mbDrawGridLine;
	int mDilateTimes;
	SurfaceType mSurfaceType;

};
