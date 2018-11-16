#pragma once
#include <SPhoenix/Camera.h>
#include <SPhoenix/Plane.h>
#include "DemoScene.h"

namespace SP
{
	class DemoCamera : public Camera
	{
	public:
		DemoCamera(int width = 0, int height = 0, int offsetX = 0, int offsetY = 0)
		: Camera(width, height, offsetX, offsetY)
		{
			mpMaterialFBO = std::make_shared<MaterialFBO>();
			mpAttachedScene = std::make_shared<DemoScene>();
			mvpHidenScene.push_back(mpAttachedScene);
		}

		std::shared_ptr<DemoScene> getDemoAttachedScene()
		{
			return std::dynamic_pointer_cast<DemoScene>(mpAttachedScene);
		}

		std::shared_ptr<MaterialFBO> getMaterialFBO()
		{
			return mpMaterialFBO;
		}

		std::shared_ptr<Plane> getViewPlane()
		{
			return mpViewPlane;
		}

		void addToHidenScenes(const std::shared_ptr<Scene> &pScene)
		{
			mvpHidenScene.push_back(pScene);
		}

		void setCameraColor(glm::vec4 cameraColor)
		{
			mCameraColor = cameraColor;
		}

		glm::vec4 getCameraColor()
		{
			return mCameraColor;
		}

		virtual void createCameraShape(glm::vec4 color = glm::vec4(1.0f),
									   glm::mat4 scale = glm::mat4(1.0f))
		{
			Camera::createCameraShape(color, scale);
			getDemoAttachedScene()->addOpaqueMesh(mpCameraShape);

			//Create the view plane
			float tanHalfFovy = std::tan(mFovy*0.5);
			float tanHalfFovx = tanHalfFovy * mAspect;
			glm::mat4 viewT = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
			mpViewPlane = std::make_shared<Plane>(tanHalfFovx * 2, tanHalfFovy * 2, viewT, 0);
			mpViewPlane->setMaterial(mpMaterialFBO);
			mpViewPlane->setTexCoord(0.0f, 0.0f, 1.0f, 1.0f);
		}

		//Preparation for some OpenGL context settings, Can be only 
		//called from inherited class of the GLWindowBase class
		virtual void setup(int winWidth, int winHeight)
		{
			
			/*float bufferWInv = 1.0f / winWidth, bufferHInv = 1.0f / winHeight;
			mpViewPlane->setTexCoord(mCOffsetX * bufferWInv, mCOffsetY * bufferHInv,
									 mCWidth * bufferWInv, mCHeight * bufferHInv);*/

			Camera::setup(winWidth, winHeight);

			mpMaterialFBO->clearUploaded();
			mpMaterialFBO->setTexbuffer(TextureType::Tex_DIFFUSE, mColorTexture);
			mpMaterialFBO->uploadToDevice();
		}

		//Can be only called from inherited class of the GLWindowBase class
		/*virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
									  &vpScene)
		{
			std::map<GLuint, std::shared_ptr<Mesh>> &mID2Mesh = mpAttachedScene->getAllMeshes();
			std::for_each(mID2Mesh.begin(), mID2Mesh.end(),
						  [&](std::pair<const GLuint, std::shared_ptr<Mesh>> &pair)
			{
				if (pair.second != mpCameraShape)
					pair.second->setAccessible(false);
			});

			Camera::renderSceneArray(vpScene);

			std::for_each(mID2Mesh.begin(), mID2Mesh.end(),
						  [&](std::pair<const GLuint, std::shared_ptr<Mesh>> &pair)
			{
				if (pair.second != mpCameraShape)
					pair.second->setAccessible(true);
			});
		}*/

		//Can be only called from inherited class of the GLWindowBase class
		virtual void renderSceneArray(const std::vector<std::shared_ptr<Scene>>
									  &vpScene)
		{
			if (!mbDoRender) return;

			if (!mbSetup)
			{
				SP_CERR("The current camera has not been uploaded befor rendering");
				return;
			}

			if (vpScene.size() == 0)
			{
				SP_CERR("The current vpScene is empty");
				return;
			}

			//Bind the vbo point
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);

			for (size_t i = 0; i < mvpHidenScene.size(); i++)
			{
				std::map<GLuint, std::shared_ptr<Mesh>> &mID2Mesh = mvpHidenScene[i]->getAllMeshes();
				std::for_each(mID2Mesh.begin(), mID2Mesh.end(),
							  [&](std::pair<const GLuint, std::shared_ptr<Mesh>> &pair)
				{
					if (pair.second != mpCameraShape)
						pair.second->setAccessible(false);
				});
			}

			if (mbShowCanvas)
			{
				

				glViewport(mViewX, mViewY, mViewWidth, mViewHeight);

				glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

				if (mbClearPerFrame)
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//draw the scene
				for (size_t i = 0; i < vpScene.size(); i++)
				{
					const std::shared_ptr<Scene> &pScene = vpScene[i];
					if (pScene.use_count() == 0) continue;
					pScene->filterVisible(mProjMatrix, mViewMatrix, mZNear, mZFar);
					pScene->draw();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				//Copy the color buffer from mMSFBO to mFBO
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				glBlitFramebuffer(0, 0, mBWidth, mBHeight,
								  0, 0, mBWidth, mBHeight,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				//Copy the color buffer from mFBO to default FBO
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_BACK_LEFT);

				glBlitFramebuffer(mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
								  mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			}

			{
				//Bind the vbo point
				glViewport(0, 0, mBWidth, mBHeight);

				glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

				if (mbClearPerFrame)
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

				//draw the scene
				for (size_t i = 0; i < vpScene.size(); i++)
				{
					const std::shared_ptr<Scene> &pScene = vpScene[i];
					if (pScene.use_count() == 0) continue;
					pScene->filterVisible(mProjMatrix, mViewMatrix, mZNear, mZFar);
					pScene->draw();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);

				//Copy the color buffer from mMSFBO to mFBO
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				glBlitFramebuffer(0, 0, mBWidth, mBHeight,
								  0, 0, mBWidth, mBHeight,
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

				
			}

			for (size_t i = 0; i < mvpHidenScene.size(); i++)
			{
				std::map<GLuint, std::shared_ptr<Mesh>> &mID2Mesh = mvpHidenScene[i]->getAllMeshes();
				std::for_each(mID2Mesh.begin(), mID2Mesh.end(),
							  [&](std::pair<const GLuint, std::shared_ptr<Mesh>> &pair)
				{
					if (pair.second != mpCameraShape)
						pair.second->setAccessible(true);
				});
			}
		}

		~DemoCamera() {}

	private:
		std::shared_ptr<MaterialFBO> mpMaterialFBO;
		std::shared_ptr<Plane> mpViewPlane;

		std::vector<std::shared_ptr<Scene>> mvpHidenScene;
		glm::vec4 mCameraColor;
	};


}
