#pragma once

#include "FishEyeInitWarper.h"
#include "TransparentMesh.h"
#include "SequenceMatcher.h"

#include <SPhoenix\AxisMesh.h>
#include "CustomOmniCamera.h"
#include "SphereStitcherManipulator.h"
#include <opencv2/stitching/detail/exposure_compensate.hpp>
#include <opencv2/stitching/detail/seam_finders.hpp>
#include <opencv2/stitching/detail/blenders.hpp>
#include <opencv2/stitching/detail/util.hpp>
#include <opencv2/stitching/detail/motion_estimators.hpp>

class SphereStitcher
{
public:
	SphereStitcher(int subdivision = 0, SurfaceType surfaceType = CUBE_SURFACE)
	{
		mSurfaceType = surfaceType;
		mSubdivision = subdivision;

		//int showW = 1440, showH = 900;
		int showW = 640, showH = 640;
		mpWindow = std::make_shared<SP::MonitorWindow>("SphereStitcher", showW, showH, true);
		mpWindow->getDefaultCamera()->setProjectionMatrix(glm::radians(90.f));
		std::shared_ptr<SP::SphereStitcherManipulator> pManipulator
			= std::make_shared<SP::SphereStitcherManipulator>(mpWindow, "ScreenShot");
		mpWindow->setManipulator(pManipulator);

		mpScene = std::make_shared<SP::Scene>();
		std::shared_ptr<SP::Mesh> pMesh;

		std::shared_ptr<SP::Material> pMaterial = std::make_shared<SP::Material>();
		pMaterial->setTexWarpType(GL_CLAMP_TO_EDGE);
		std::shared_ptr<SP::Texture> pImageTexture =
			std::make_shared<SP::Texture>(SP::TextureType::Tex_DIFFUSE);
		std::shared_ptr<SP::Texture> pMaskTexture =
			std::make_shared<SP::Texture>(SP::TextureType::Tex_AMBIENT);
		pMaterial->addTexture(pImageTexture);
		pMaterial->addTexture(pMaskTexture);

		switch (surfaceType)
		{
		case UV_SURFACE:
		{
		}
			break;
		case CUBE_SURFACE:
		{
			SP::CubeGrid cubeGrid(subdivision);

			mvVertice = cubeGrid.getSphereVertice();

			std::vector<glm::vec3> vVertice = cubeGrid.getFlatSphereVertice();
			std::vector<GLuint> vIndice;
			std::vector<glm::vec2> vTexCoordOnFlat = cubeGrid.vTexCoordOnFlat;

			std::shared_ptr<SP::VertexArray> pVA = std::make_shared<SP::VertexArray>(vVertice);
			pVA->setNormals(vVertice);
			
			pVA->setTexCoords(vTexCoordOnFlat);
			//glm::mat4 rotInvZ = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
			pVA->addInstance(/*rotInvZ*/);

			std::string vertShaderFile = "SphereMLS-Stitcher.vert";
			std::string fragShaderFile = "SphereMLS-Stitcher.frag";
			std::shared_ptr<SP::ShaderProgram> pShader
				= std::make_shared<SP::ShaderProgram>(vertShaderFile, fragShaderFile);
			//
			pMesh = std::make_shared<SP::Mesh>(pVA, pMaterial);
			//std::shared_ptr<TMesh> pTMesh = std::make_shared<TMesh>(*pMesh);
			mpScene->addMesh(pMesh, pShader);

			std::shared_ptr<SP::AxisMesh> pAxisMesh = 
				std::make_shared<SP::AxisMesh>(0.1f);
			mpScene->addMesh(pAxisMesh);
		}
			break;
		case ICO_SURFACE:
			break;
		default:
			break;
		}

		if (pMesh.use_count() != 0)
		{
			mpMesh = pMesh;
		}

		mpWindow->setScene(mpScene);

		/*mpCamera = std::make_shared<SP::UnifyFECamera>(1, 1, 0, 0);
		mpCamera->setCustomBufferSize(OmniTexWidth, OmniTexHeight);
		mpCamera->setShowCanvas(false);
		mpWindow->addCamera(mpCamera);*/

		int faceSide = 200, faceTexSide = 1024;
		mpOmniCamera = std::make_shared<CustomOmniCamera>(
			faceSide, faceTexSide,
			0, 0,
			SP::OmniCamera::ViewPlaneType(mSurfaceType));
		mpOmniCamera->setShowCanvas(false);
		mpOmniCamera->setDoRender(false);
		//mpOmniCamera->setCustomBufferSize(2000, 1000);
		mpWindow->addCamera(mpOmniCamera);


		mpCamera = std::make_shared<SP::UnifyFECamera>(1, 1, 0, 0);
		mpCamera->setShowCanvas(false);
		mpCamera->setDoRender(false);
		//mpCamera->setCustomBufferSize(image.cols, image.rows);
		mpWindow->addCamera(mpCamera);
		
	}
	~SphereStitcher() {}

	void renderStitcher(const std::vector<cv::Mat> &vWarpedImage,
						const std::vector<cv::Mat> &vWarpedMask)
	{
		int num_image = vWarpedImage.size();
		if (vWarpedMask.size() != num_image)
			return;

		//Update the material
		std::shared_ptr<SP::Material> pMaterial = mpMesh->getMaterial();
		pMaterial->reset();

		//pMaterial->setTexWarpType(GL_CLAMP_TO_EDGE);
		
		for (size_t i = 0; i < num_image; i++)
		{
			const cv::Mat &image = vWarpedImage[i];
			const cv::Mat &mask = vWarpedMask[i];
			unsigned char *pImage = reinterpret_cast<unsigned char *>(image.data);
			std::shared_ptr<unsigned char> pImageData(pImage,
												[](unsigned char *d)
			{
				//do nothing
			});

			std::shared_ptr<SP::Texture> pImageTexture =
				std::make_shared<SP::Texture>(pImageData, image.cols, image.rows, 3, SP::TextureType::Tex_DIFFUSE);
			

			unsigned char *pMask = reinterpret_cast<unsigned char *>(mask.data);
			std::shared_ptr<unsigned char> pMaskData(pMask,
													 [](unsigned char *d)
			{
				//do nothing
			});
			std::shared_ptr<SP::Texture> pMaskTexture =
				std::make_shared<SP::Texture>(pMaskData, mask.cols, mask.rows, 1, SP::TextureType::Tex_AMBIENT);

			pMaterial->addTexture(pImageTexture);
			pMaterial->addTexture(pMaskTexture);
		}

		pMaterial->uploadToDevice();

		mpWindow->run();
	}

	//Do the sphere MLS, pairInfo.points1 is warped to align then points2
	//The image must be the output result from FishEyeInitWarper
	void renderMLSResult(const cv::Mat &normal, cv::Mat &image, cv::Mat &mask, PairInfo &pairInfo, int dilateTimes = 1)
	{
		std::vector<cv::Point3f> vNormal1(pairInfo.inliers_num);
		std::vector<cv::Point3f> vNormal2(pairInfo.inliers_num);
		std::vector<cv::Mat> vCov(pairInfo.inliers_num);

		{
		for (size_t i = 0, j = 0; i < pairInfo.inliers_num; j++)
			if (pairInfo.mask[j])
			{
				cv::Point2f &pt1 = pairInfo.points1[j];
				cv::Point2f &pt2 = pairInfo.points2[j];
				cv::Point3f normal1 = normal.at<cv::Vec3f>(pt1);
				cv::Point3f normal2 = normal.at<cv::Vec3f>(pt2);
				vNormal1[i] = normal1;
				vNormal2[i] = normal2;

				cv::Mat &cov = vCov[i];
				cov = cv::Mat(3, 3, CV_32FC1);
				cov.at<float>(0, 0) = normal1.x * normal2.x;
				cov.at<float>(0, 1) = normal1.x * normal2.y;
				cov.at<float>(0, 2) = normal1.x * normal2.z;
				cov.at<float>(1, 0) = normal1.y * normal2.x;
				cov.at<float>(1, 1) = normal1.y * normal2.y;
				cov.at<float>(1, 2) = normal1.y * normal2.z;
				cov.at<float>(2, 0) = normal1.z * normal2.x;
				cov.at<float>(2, 1) = normal1.z * normal2.y;
				cov.at<float>(2, 2) = normal1.z * normal2.z;
				i++;
			}
		}

		for (size_t i = 0; i < mvVertice.size(); i++)
		{
			std::vector<float> vWeight(pairInfo.inliers_num);
			cv::Point3f vert(mvVertice[i].x, mvVertice[i].y, mvVertice[i].z);
			cv::Mat cov(3, 3, CV_32FC1, cv::Scalar(0));
			for (size_t j = 0; j < pairInfo.inliers_num; j++)
			{
				vWeight[j] = std::max(0.001f, vert.dot(vNormal1[j]));
				cov += vCov[j] * vWeight[j];
			}

			cv::SVD svd;
			cv::Mat w, ut, u, vt, v;
			svd.compute(cov, w, u, vt);
			v = vt.t();
			ut = u.t();
			float dvut = cv::determinant(v*ut);
			cv::Mat w_ = cv::Mat::eye(3, 3, CV_32FC1);
			w_.at<float>(2, 2) = dvut;
			cv::Mat R = v * w_ * ut;
			cv::Mat vert_ = R * cv::Mat(vert);
			mvVertice[i] = glm::vec3(vert_.at<float>(0, 0),
									 vert_.at<float>(1, 0),
									 vert_.at<float>(2, 0));
		}

		std::shared_ptr<SP::Scene> pSceneTmp = std::make_shared<SP::Scene>();
		{
			SP::CubeGrid cubeGrid(mSubdivision);
			std::vector<glm::vec2> vTexCoordOnFlat = cubeGrid.vTexCoordOnFlat;

			std::string vertShaderFile = "SphereMLS-Stitcher.vert";
			std::string fragShaderFile = "SphereMLS-Stitcher.frag";
			std::shared_ptr<SP::ShaderProgram> pShader;
			pShader = std::make_shared<SP::ShaderProgram>(vertShaderFile, fragShaderFile);

			std::shared_ptr<SP::Material> pMaterial = std::make_shared<SP::Material>();
			pMaterial->setTexWarpType(GL_CLAMP_TO_EDGE);
			{
				unsigned char *pImage = reinterpret_cast<unsigned char *>(image.data);
				std::shared_ptr<unsigned char> pImageData(pImage,
														  [](unsigned char *d)
				{
					//do nothing
				});

				std::shared_ptr<SP::Texture> pImageTexture =
					std::make_shared<SP::Texture>(pImageData, image.cols, image.rows, 3, SP::TextureType::Tex_DIFFUSE);

				unsigned char *pMask = reinterpret_cast<unsigned char *>(mask.data);
				std::shared_ptr<unsigned char> pMaskData(pMask,
														 [](unsigned char *d)
				{
					//do nothing
				});
				std::shared_ptr<SP::Texture> pMaskTexture =
					std::make_shared<SP::Texture>(pMaskData, mask.cols, mask.rows, 1, SP::TextureType::Tex_AMBIENT);

				pMaterial->addTexture(pImageTexture);
				pMaterial->addTexture(pMaskTexture);
			}
			
			{
				cubeGrid.vVertice = mvVertice;
				std::vector<glm::vec3> vVertice = cubeGrid.getFlatSphereVertice();
				std::shared_ptr<SP::VertexArray> pVA = std::make_shared<SP::VertexArray>(vVertice);
				pVA->setNormals(vVertice);
				pVA->setTexCoords(vTexCoordOnFlat);

				glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
				glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.01f, 1.01f, 1.01f));
				//pVA->addInstance(S);
				//pVA->addInstance(T);
				pVA->addInstance();

				std::shared_ptr<SP::Mesh> pMesh = std::make_shared<SP::Mesh>(pVA, pMaterial);
				/*std::shared_ptr<SP::Material> pMLSMaterial =
					std::make_shared<SP::Material>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				pMesh->setMaterial(pMLSMaterial);*/
				
				pSceneTmp->addMesh(pMesh, pShader);
			}
			

			std::shared_ptr<SP::AxisMesh> pAxisMesh =
				std::make_shared<SP::AxisMesh>(0.5f);
			pSceneTmp->addMesh(pAxisMesh);
		}

		mpWindow->setScene(pSceneTmp);
		mpWindow->getDefaultCamera()->setDoRender(false);
		mpOmniCamera->setDoRender(true);
		mpOmniCamera->setCustomBufferSize(image.cols, image.rows);

		mpWindow->runOnce();
		mpOmniCamera->setDoRender(false);
		
		int w, h, c;
		std::shared_ptr<unsigned char> pMaskData;
		mpOmniCamera->readMaskBuffer(pMaskData, w, h, c);
		cv::Mat warpedMask = cv::Mat(h, w, CV_8UC1, pMaskData.get());
		warpedMask.copyTo(mask);

		//Do dilate
		if (dilateTimes > 0)
		{
			std::shared_ptr<SP::Scene> pSceneShow = std::make_shared<SP::Scene>();
			std::shared_ptr<SP::MaterialFBO> pMaterialFBOMask = mpOmniCamera->getMaterialFBOMask();
			std::shared_ptr<SP::MaterialFBO> pMaterialFBO = mpOmniCamera->getMaterialFBO();
			GLuint maskBuffer;
			pMaterialFBOMask->getTexbuffer(1, maskBuffer);
			pMaterialFBO->setTexbuffer(0, maskBuffer);

			std::shared_ptr<SP::Plane> pPlane = std::make_shared<SP::Plane>(2, 2);
			pPlane->setMaterial(pMaterialFBO);
			pPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);

			std::string vertShaderFile2D = "SphereMLS-dilate-debug.vert";
			std::string fragShaderFile2D = "SphereMLS-dilate-debug.frag";
			std::shared_ptr<SP::ShaderProgram> pShader2D
				= std::make_shared<SP::ShaderProgram>(vertShaderFile2D, fragShaderFile2D);

			pSceneShow->addMesh(pPlane, pShader2D);

			mpWindow->setScene(pSceneShow);
			mpCamera->setDoRender(true);
			mpCamera->setCustomBufferSize(image.cols, image.rows);

			mpWindow->runOnce();

			std::shared_ptr<SP::MaterialFBO> pMaterialFBOMask_ = mpCamera->getMaterialFBOMask();
			std::shared_ptr<SP::MaterialFBO> pMaterialFBO_ = mpCamera->getMaterialFBO();
			GLuint colorBuffer_, maskBuffer_;
			pMaterialFBOMask_->getTexbuffer(1, maskBuffer_);
			pMaterialFBO_->getTexbuffer(1, colorBuffer_);
			pMaterialFBO->setTexbuffer(0, maskBuffer_);
			pMaterialFBO->setTexbuffer(1, colorBuffer_);

			for (size_t i = 0; i < dilateTimes - 1; i++)
			{
				mpWindow->runOnce();
			}

			mpCamera->setDoRender(false);

			std::shared_ptr<unsigned char> pColorData;
			mpCamera->readColorBuffer(pColorData, w, h, c);
			cv::Mat warpedColor = cv::Mat(h, w, CV_8UC3, pColorData.get());
			warpedColor.copyTo(image);

			/*std::shared_ptr<unsigned char> pMaskData1;
			mpCamera->readMaskBuffer(pMaskData1, w, h, c);
			cv::Mat warpedMask1 = cv::Mat(h, w, CV_8UC1, pMaskData1.get());
			warpedMask1.copyTo(mask);*/
		}
		else
		{
			std::shared_ptr<unsigned char> pColorData;
			mpOmniCamera->readColorBuffer(pColorData, w, h, c);
			cv::Mat warpedColor = cv::Mat(h, w, CV_8UC3, pColorData.get());
			warpedColor.copyTo(image);
		}

		mpWindow->setScene(mpScene);
		mpWindow->getDefaultCamera()->setDoRender(true);
	}

	void renderPTISResult()
	{

	}


	void renderMLSResult0001(const cv::Mat &normal, cv::Mat &image, cv::Mat &mask, PairInfo &pairInfo)
	{
		std::vector<cv::Point3f> vNormal1(pairInfo.inliers_num);
		std::vector<cv::Point3f> vNormal2(pairInfo.inliers_num);
		std::vector<cv::Mat> vCov(pairInfo.inliers_num);

		for (size_t i = 0, j = 0; i < pairInfo.inliers_num; j++)
		{
			if (pairInfo.mask[j])
			{
				cv::Point2f &pt1 = pairInfo.points1[j];
				cv::Point2f &pt2 = pairInfo.points2[j];
				cv::Point3f normal1 = normal.at<cv::Vec3f>(pt1);
				cv::Point3f normal2 = normal.at<cv::Vec3f>(pt2);
				vNormal1[i] = normal1;
				vNormal2[i] = normal2;

				cv::Mat &cov = vCov[i];
				cov = cv::Mat(3, 3, CV_32FC1);
				cov.at<float>(0, 0) = normal1.x * normal2.x;
				cov.at<float>(0, 1) = normal1.x * normal2.y;
				cov.at<float>(0, 2) = normal1.x * normal2.z;
				cov.at<float>(1, 0) = normal1.y * normal2.x;
				cov.at<float>(1, 1) = normal1.y * normal2.y;
				cov.at<float>(1, 2) = normal1.y * normal2.z;
				cov.at<float>(2, 0) = normal1.z * normal2.x;
				cov.at<float>(2, 1) = normal1.z * normal2.y;
				cov.at<float>(2, 2) = normal1.z * normal2.z;
				i++;
			}
		}

		for (size_t i = 0; i < mvVertice.size(); i++)
		{
			std::vector<float> vWeight(pairInfo.inliers_num);
			cv::Point3f vert(mvVertice[i].x, mvVertice[i].y, mvVertice[i].z);
			cv::Mat cov(3, 3, CV_32FC1, cv::Scalar(0));
			for (size_t j = 0; j < pairInfo.inliers_num; j++)
			{
				vWeight[j] = std::max(0.05f, vert.dot(vNormal1[j]));
				cov += vCov[j] * vWeight[j];
			}

			cv::SVD svd;
			cv::Mat w, ut, u, vt, v;
			svd.compute(cov, w, u, vt);
			v = vt.t();
			ut = u.t();
			float dvut = cv::determinant(v*ut);
			cv::Mat w_ = cv::Mat::eye(3, 3, CV_32FC1);
			w_.at<float>(2, 2) = dvut;
			cv::Mat R = v * w_ * ut;
			cv::Mat vert_ = R * cv::Mat(vert);
			mvVertice[i] = glm::vec3(vert_.at<float>(0, 0),
									 vert_.at<float>(1, 0),
									 vert_.at<float>(2, 0));
		}

		std::shared_ptr<SP::Scene> pSceneTmp = std::make_shared<SP::Scene>();
		{
			SP::CubeGrid cubeGrid(mSubdivision);
			std::vector<glm::vec2> vTexCoordOnFlat = cubeGrid.vTexCoordOnFlat;

			std::string vertShaderFile = "SphereMLS-Stitcher.vert";
			std::string fragShaderFile = "SphereMLS-Stitcher.frag";
			std::shared_ptr<SP::ShaderProgram> pShader;
			//pShader = std::make_shared<SP::ShaderProgram>(vertShaderFile, fragShaderFile);

			std::shared_ptr<SP::Material> pMaterial = std::make_shared<SP::Material>();
			{
				unsigned char *pImage = reinterpret_cast<unsigned char *>(image.data);
				std::shared_ptr<unsigned char> pImageData(pImage,
														  [](unsigned char *d)
				{
					//do nothing
				});

				std::shared_ptr<SP::Texture> pImageTexture =
					std::make_shared<SP::Texture>(pImageData, image.cols, image.rows, 3, SP::TextureType::Tex_DIFFUSE);

				unsigned char *pMask = reinterpret_cast<unsigned char *>(mask.data);
				std::shared_ptr<unsigned char> pMaskData(pMask,
														 [](unsigned char *d)
				{
					//do nothing
				});
				std::shared_ptr<SP::Texture> pMaskTexture =
					std::make_shared<SP::Texture>(pMaskData, mask.cols, mask.rows, 1, SP::TextureType::Tex_AMBIENT);

				pMaterial->addTexture(pImageTexture);
				pMaterial->addTexture(pMaskTexture);
			}


			{
				std::vector<glm::vec3> vVertice = cubeGrid.getFlatSphereVertice();
				std::shared_ptr<SP::VertexArray> pVA = std::make_shared<SP::VertexArray>(vVertice);
				pVA->setNormals(vVertice);
				pVA->setTexCoords(vTexCoordOnFlat);
				pVA->addInstance();

				std::shared_ptr<SP::Mesh> pMesh = std::make_shared<SP::Mesh>(pVA, pMaterial);
				std::shared_ptr<SP::Material> pOriginMaterial =
					std::make_shared<SP::Material>(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
				pMesh->setMaterial(pOriginMaterial);

				pSceneTmp->addMesh(pMesh, pShader);
			}
			{
				cubeGrid.vVertice = mvVertice;
				std::vector<glm::vec3> vVertice = cubeGrid.getFlatSphereVertice();
				std::shared_ptr<SP::VertexArray> pVA = std::make_shared<SP::VertexArray>(vVertice);
				pVA->setNormals(vVertice);
				pVA->setTexCoords(vTexCoordOnFlat);

				glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
				glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(1.01f, 1.01f, 1.01f));
				pVA->addInstance(S);
				//pVA->addInstance(T);

				std::shared_ptr<SP::Mesh> pMesh = std::make_shared<SP::Mesh>(pVA, pMaterial);
				std::shared_ptr<SP::Material> pMLSMaterial =
					std::make_shared<SP::Material>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				pMesh->setMaterial(pMLSMaterial);

				pSceneTmp->addMesh(pMesh, pShader);
			}


			std::shared_ptr<SP::AxisMesh> pAxisMesh =
				std::make_shared<SP::AxisMesh>(0.5f);
			pSceneTmp->addMesh(pAxisMesh);
		}

		mpWindow->setScene(pSceneTmp);
		mpWindow->run();
	}

	void blendCompute(std::vector<cv::Mat>& vWarpedImage, std::vector<cv::Mat>& vWarpedMask,cv::Mat &result, cv::Mat &resultMask)
	{
		int num_images = vWarpedImage.size();
		if (num_images == 0) return;
		cv::Size resultSize = vWarpedImage[0].size();
		cv::Size m_findSeam_size(1000, 1000);
		double seam_aspect_blend = m_findSeam_size.height / (double)resultSize.height;
		std::vector<cv::Size> blend_sizes;
		std::vector<cv::Point> blend_corners;
		for (size_t i = 0; i < num_images; i++)
		{
			blend_sizes.push_back(vWarpedImage[i].size());
			blend_corners.push_back(cv::Point(0, 0));
		}

		std::vector<cv::UMat> seam_warped(num_images);
		std::vector<cv::UMat> seam_warped_f(num_images);
		std::vector<cv::UMat> seam_masks_warped(num_images);
		std::vector<cv::Point> seam_corners;

		for (size_t i = 0; i < num_images; i++)
		{
			m_findSeam_size = cv::Size(vWarpedImage[i].cols*seam_aspect_blend, vWarpedImage[i].rows*seam_aspect_blend);

			cv::resize(vWarpedImage[i], seam_warped[i], m_findSeam_size);
			cv::resize(vWarpedMask[i], seam_masks_warped[i], m_findSeam_size);
			seam_warped[i].convertTo(seam_warped_f[i], CV_32F);
			seam_corners.push_back(blend_corners[i] * seam_aspect_blend);
		}
		int m_expos_comp_type = cv::detail::ExposureCompensator::GAIN_BLOCKS;
		int m_blend_type = cv::detail::Blender::MULTI_BAND;
		double m_blend_strength = 10;

		cv::Ptr<cv::detail::ExposureCompensator> compensator = cv::detail::ExposureCompensator::createDefault(m_expos_comp_type);
		compensator->feed(seam_corners, seam_warped, seam_masks_warped);

		cv::Ptr<cv::detail::SeamFinder> seam_finder = cv::makePtr<cv::detail::GraphCutSeamFinder>(cv::detail::GraphCutSeamFinderBase::COST_COLOR);
		seam_finder->find(seam_warped_f, seam_corners, seam_masks_warped);

		std::vector<cv::Mat> seam_masks_mat(num_images);
		for (size_t i = 0; i < num_images; i++)
		{
			seam_masks_warped[i].copyTo(seam_masks_mat[i]);
		}

		seam_warped.clear();
		seam_warped_f.clear();
		
		cv::Ptr<cv::detail::Blender> blender = cv::detail::Blender::createDefault(m_blend_type);
		{
			cv::Size dst_sz = cv::detail::resultRoi(blend_corners, blend_sizes).size();
			double blend_width = sqrt(static_cast<double>(dst_sz.area())) * m_blend_strength / 100.f;
			if (blend_width < 1.f)
				blender = cv::detail::Blender::createDefault(cv::detail::Blender::NO);
			else if (m_blend_type == cv::detail::Blender::MULTI_BAND)
			{
				cv::detail::MultiBandBlender* mb = dynamic_cast<cv::detail::MultiBandBlender*>(blender.get());
				int bands = static_cast<int>(ceil(log(blend_width) / log(2.)) - 1.);
				mb->setNumBands(bands);
			}
			else if (m_blend_type == cv::detail::Blender::FEATHER)
			{
				cv::detail::FeatherBlender* fb = dynamic_cast<cv::detail::FeatherBlender*>(blender.get());
				fb->setSharpness(1.f / blend_width);
			}
			blender->prepare(blend_corners, blend_sizes);
		}

		cv::Mat blend_mask_warped, blend_warped_s;
		cv::Mat dilated_mask, seam_mask;
		for (size_t i = 0; i < num_images; i++)
		{
			blend_mask_warped = vWarpedMask[i];
			//compensator->apply(i, blend_corners[i], vWarpedImage[i], blend_mask_warped);
			vWarpedImage[i].convertTo(blend_warped_s, CV_16S);
			dilate(seam_masks_warped[i], dilated_mask, cv::Mat());
			resize(dilated_mask, seam_mask, blend_mask_warped.size());
			blend_mask_warped = seam_mask/* & blend_mask_warped*/;
			blender->feed(blend_warped_s, blend_mask_warped, blend_corners[i]);
		}
		blender->blend(result, resultMask);
		result.convertTo(result, CV_8UC3);
	}

	void computeAvgPixelError()
	{

	}


private:

	std::shared_ptr<SP::MonitorWindow> mpWindow;
	std::shared_ptr<CustomOmniCamera> mpOmniCamera;
	std::shared_ptr<SP::Mesh> mpMesh;
	std::shared_ptr<SP::Scene> mpScene;
	std::shared_ptr<SP::UnifyFECamera> mpCamera;

	std::vector<glm::vec3> mvVertice;

	SurfaceType mSurfaceType;
	int mSubdivision;
};
