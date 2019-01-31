#pragma once

#include <SPhoenix\Camera.h>
#include <OpencvCommon.h>

//Include some redundant codes from previous fisheye stitcher works
#include "CameraModel.h"
#include "FishCamera.h"

namespace SP
{
	//C. Geyer and K. Daniilidis, ¡°A unifying theory for central panoramic
	//systems and practical implications, ¡± in ECCV, 2000
	class UnifyFECamera : public Camera
	{
	public:
		UnifyFECamera(int width, int height, int offsetX, int offsetY)
			: Camera(width, height, offsetX, offsetY), mbCustomBufferSize(false)
		{
			mpMaterialFBO = std::make_shared<MaterialFBO>();
			mpMaterialFBOMask = std::make_shared<MaterialFBO>();
		}
		~UnifyFECamera() {}

		std::shared_ptr<MaterialFBO> getMaterialFBO()
		{
			return mpMaterialFBO;
		}

		std::shared_ptr<MaterialFBO> getMaterialFBOMask()
		{
			return mpMaterialFBOMask;
		}

		void readFromTUMSettings(std::string fileName)
		{
			std::ifstream fSettings(fileName, std::ios::in);
			if (!fSettings.is_open())
			{
				std::cout << "Failed to open settings file at: " << fileName << std::endl;
				exit(-1);
			}

			std::string line;
			while (std::getline(fSettings, line))
			{
				std::stringstream ioStr;
				ioStr << line;
				std::string label;
				ioStr >> label;
				int colonPos = label.find(':');
				if (colonPos != std::string::npos)
				{
					label = label.substr(0, colonPos);
					int lineColonPos = line.find(':');
					std::string resLine = line.substr(lineColonPos + 1);
					ioStr.clear();
					ioStr << resLine;
					char tempChar;
					if (label == "distortion_coeffs")
					{
						int length = 4;
						for (size_t i = 0; i < length; i++)
						{
							ioStr >> tempChar;
							ioStr >> mDistCoeffs[i];
						}
					}
					else if (label == "intrinsics")
					{
						mUnifyCoeffs[0] = glm::radians(185.f);
						ioStr >> tempChar;
						ioStr >> mUnifyCoeffs[1];
						int length = 4;
						for (size_t i = 0; i < length; i++)
						{
							ioStr >> tempChar;
							ioStr >> mImageCoeffs[i];
						}
					}
					else if (label == "resolution")
					{
						int length = 2;
						for (size_t i = 0; i < length; i++)
						{
							ioStr >> tempChar;
							ioStr >> mImageSize[i];
						}
					}
				}
			}

			if (mbSetup)
			{
				_uploadUnifyUBO();
			}
		}

		void readFromHLSetting(std::string fileName)
		{
			cv::FileStorage fs;
			fs.open(fileName, cv::FileStorage::READ);
			if (fs.isOpened())
			{
				//Load the estimated state which have been saved
				bool is_ring;
				std::vector<int> index;
				fs["is_ring"] >> is_ring;
				fs["index"] >> index;

				int numCamera = index.size();

				if (numCamera == 0) return;

				CircleFish::FishCamera camera;
				camera.LoadFromXML(fs, 0);

				std::vector<double *> &param = camera.pModel->vpParameter;

				float fov = camera.pModel->fov - 0.05;
				float u0 = *(param[0]);
				float v0 = *(param[1]);
				float f = *(param[2]);
				float m = *(param[3]);
				float l = *(param[4]);

				mUnifyCoeffs = glm::vec2(fov, l);
				mImageCoeffs = glm::vec4(f*(m + l), f*(m + l), u0, v0);
				mDistCoeffs = glm::vec4(0.0f);
				mImageSize = glm::vec2(camera.imgW, camera.imgH);
			}
			else
			{
				std::cerr << "Cannot open the setting file " << fileName << std::endl;
				exit(-1);
			}
			
			if (mbSetup)
			{
				_uploadUnifyUBO();
			}
		}

		void setCustomBufferSize(int CBWidth, int CBHeight)
		{
			bool bReSetup = CBWidth != mCustomBWidth ||
				CBHeight != mCustomBHeight;

			mCustomBWidth = CBWidth;
			mCustomBHeight = CBHeight;
			if (mCustomBHeight > 0 && mCustomBWidth > 0 &&
				mCustomBWidth < MAX_VIEWPORT_SIZE &&
				mCustomBHeight < MAX_VIEWPORT_SIZE)
			{
				mbCustomBufferSize = true;
			}

			if (mbCustomBufferSize && mbSetup && bReSetup)
			{
				clearSetup();
				setup(mCustomBWidth, mCustomBHeight);
			}
		}

		//Preparation for some OpenGL context settings, Can be only 
		//called from inherited class of the GLWindowBase class
		virtual void setup(int winWidth, int winHeight)
		{
			if (mbCustomBufferSize)
			{
				Camera::setup(mCustomBWidth, mCustomBHeight);
			}
			else
			{
				Camera::setup(winWidth, winHeight);
			}

			_createMultiSampleMask();
			_createNormalMask();
			_createMultiSampleSNormal();
			_createNormalSNormal();

			//Using uniform buffers
			glGenBuffers(1, &mUnifyFEUBO);
			glBindBuffer(GL_UNIFORM_BUFFER, mUnifyFEUBO);
			glBufferData(GL_UNIFORM_BUFFER, 48, NULL, GL_STATIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			_uploadUnifyUBO();

			mpMaterialFBO->clearUploaded();
			mpMaterialFBO->setTexbuffer(TextureType::Tex_DIFFUSE, mColorTexture);
			mpMaterialFBO->uploadToDevice();

			mpMaterialFBOMask->clearUploaded();
			mpMaterialFBOMask->setTexbuffer(TextureType::Tex_DIFFUSE, mMaskTexture);
			mpMaterialFBOMask->uploadToDevice();
		}

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
			glBindBufferBase(GL_UNIFORM_BUFFER, 2, mUnifyFEUBO);
			glViewport(0, 0, mBWidth, mBHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			if (mbClearPerFrame)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//draw the scene
			for (size_t i = 0; i < vpScene.size(); i++)
			{
				const std::shared_ptr<Scene> &pScene = vpScene[i];
				if (pScene.use_count() == 0) continue;
				//pScene->filterVisible(mProjMatrix, mViewMatrix, mZNear, mZFar);
				pScene->draw();
			}
			glBindBufferBase(GL_UNIFORM_BUFFER, 2, 0);

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

			//Blit the meshid buffer, in here we use it for retriving the 
			//Triangle id map
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, mBWidth, mBHeight,
							  0, 0, mBWidth, mBHeight,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);

			//Blit the mask buffer
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glDrawBuffer(GL_COLOR_ATTACHMENT2);
			glBlitFramebuffer(0, 0, mBWidth, mBHeight,
							  0, 0, mBWidth, mBHeight,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);

			//Blit the normal buffer
			glReadBuffer(GL_COLOR_ATTACHMENT3);
			glDrawBuffer(GL_COLOR_ATTACHMENT3);
			glBlitFramebuffer(0, 0, mBWidth, mBHeight,
							  0, 0, mBWidth, mBHeight,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);


			if (mbShowCanvas)
			{
				//Copy the color buffer from mFBO to default FBO
				glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				glDrawBuffer(GL_BACK_LEFT);

				glBlitFramebuffer(0, 0, mBWidth, mBHeight,
								  mCOffsetX, mCOffsetY, mCWidth + mCOffsetX, mCHeight + mCOffsetY,
								  GL_COLOR_BUFFER_BIT , GL_LINEAR);
			}

		}

		virtual void readMaskBuffer(std::shared_ptr<unsigned char> &pData,
									 int &width, int &height, int &channel)
		{
			width = mBWidth;
			height = mBHeight;
			channel = 1;

			GLuint *dataRaw = new GLuint[width * height * channel];
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glReadPixels(0, 0, width, height, GL_RED_INTEGER, GL_UNSIGNED_INT, dataRaw);

			unsigned char *data = new unsigned char[width * height * channel];

			int length = width * height * channel;
			for (size_t i = 0; i < length; i++)
			{
				data[i] = dataRaw[i];
				/*if(dataRaw[i] != 0)
					std::cout << dataRaw[i] << " ";*/
			}

			unsigned char *dataFlip = new unsigned char[width * height * channel];

			int rowWidth = width * channel;
			for (size_t i = 0, c1 = 0, c2 = rowWidth*(height - 1);
				 i < height; i++, c1 += rowWidth, c2 -= rowWidth)
			{
				memcpy(dataFlip + c1, data + c2, rowWidth);
			}

			std::shared_ptr<unsigned char> pData_(dataFlip, [](unsigned char *d)
			{
				delete[] d;
			});

			pData = pData_;
		}

		virtual void readSNormalBuffer(std::shared_ptr<GLfloat> &pData,
									   int &width, int &height, int &channel)
		{
			width = mBWidth;
			height = mBHeight;
			channel = 3;

			GLfloat *data = new GLfloat[width * height * channel];
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT3);
			glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, data);

			GLfloat *dataFlip = new GLfloat[width * height * channel];

			int rowWidth = width * channel;
			for (size_t i = 0, c1 = 0, c2 = rowWidth*(height - 1);
				 i < height; i++, c1 += rowWidth, c2 -= rowWidth)
			{
				memcpy(dataFlip + c1, data + c2, rowWidth * sizeof(GLfloat));
			}



			std::shared_ptr<GLfloat> pData_(dataFlip, [](GLfloat *d)
			{
				delete[] d;
			});

			pData = pData_;
		}

	protected:
		/*float fov, ksai, fx, fy, u0, v0;
		float k1, k2, p1, p2;*/
		//fov, ksai
		glm::vec2 mUnifyCoeffs;

		//fx, fy, u0, v0
		glm::vec4 mImageCoeffs;

		//k1, k2, p1, p2
		glm::vec4 mDistCoeffs;

		//width, height
		glm::vec2 mImageSize;

		//The UBO for sharing the view matrix and projection matrix between shaders
		GLuint mUnifyFEUBO;

		bool mbCustomBufferSize;
		int mCustomBWidth, mCustomBHeight;

		GLuint mMSMaskTexture, mMaskTexture;
		GLuint mMSSNormalTexture, mSNormalTexture;

		std::shared_ptr<MaterialFBO> mpMaterialFBO;
		std::shared_ptr<MaterialFBO> mpMaterialFBOMask;

	private:
		void _uploadUnifyUBO()
		{
			glBindBuffer(GL_UNIFORM_BUFFER, mUnifyFEUBO);

			glBufferSubData(GL_UNIFORM_BUFFER, 0, 8, glm::value_ptr(mImageSize));
			glBufferSubData(GL_UNIFORM_BUFFER, 8, 8, glm::value_ptr(mUnifyCoeffs));
			glBufferSubData(GL_UNIFORM_BUFFER, 16, 16, glm::value_ptr(mImageCoeffs));
			glBufferSubData(GL_UNIFORM_BUFFER, 32, 16, glm::value_ptr(mDistCoeffs));
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void _createMultiSampleMask()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			//MSAA for anti-aliasing
			glGenTextures(1, &mMSMaskTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMSMaskTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_R32UI,
									bufferW, bufferH, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
								   GL_TEXTURE_2D_MULTISAMPLE, mMSMaskTexture, 0);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, buffers);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The MultSamples FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void _createNormalMask()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

			glGenTextures(1, &mMaskTexture);
			glBindTexture(GL_TEXTURE_2D, mMaskTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, bufferW, bufferH, 0,
						 GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
								   GL_TEXTURE_2D, mMaskTexture, 0);

			glBindTexture(GL_TEXTURE_2D, 0);

			GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, buffers);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
				GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The Raw FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void _createMultiSampleSNormal()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);

			//MSAA for anti-aliasing
			glGenTextures(1, &mMSSNormalTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMSSNormalTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_RGB32F,
									bufferW, bufferH, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
								   GL_TEXTURE_2D_MULTISAMPLE, mMSSNormalTexture, 0);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(4, buffers);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The MultSamples FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void _createNormalSNormal()
		{
			int bufferW = mBWidth;
			int bufferH = mBHeight;

			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

			glGenTextures(1, &mSNormalTexture);
			glBindTexture(GL_TEXTURE_2D, mSNormalTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, bufferW, bufferH, 0,
						 GL_RGB, GL_FLOAT, NULL);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
								   GL_TEXTURE_2D, mSNormalTexture, 0);

			glBindTexture(GL_TEXTURE_2D, 0);

			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers(4, buffers);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
				GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The Raw FrameBuffer is not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};
}