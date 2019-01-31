#pragma once

#include <SPhoenix\OmniCamera.h>

class CustomOmniCamera : public SP::OmniCamera
{
public:
	CustomOmniCamera(int faceSide = 50, int faceTexSide = 1080,
					 int offsetX = 0, int offsetY = 0,
					 ViewPlaneType planeType = UV_PLANE)
	: OmniCamera(faceSide, faceTexSide, offsetX, offsetY, planeType) ,
		mbCustomBufferSize(false), mCustomBWidth(-1), mCustomBHeight(-1)
	{
		mpMaterialFBO = std::make_shared<SP::MaterialFBO>();
		mpMaterialFBOMask = std::make_shared<SP::MaterialFBO>();

		std::string vertShaderFileOmni = "SPhoenixScene-OmniCameraCube.vert";
		std::string fragShaderFileOmni = "SPhoenixScene-OmniCameraCube.frag";
		std::shared_ptr<SP::ShaderProgram> pShaderOmni
			= std::make_shared<SP::ShaderProgram>(vertShaderFileOmni,
											  fragShaderFileOmni);

		std::map<GLuint, std::shared_ptr<SP::Mesh>> mID2Mesh = mpPlaneScene->getAllMeshes();
		mpPlaneScene->reset();
		std::map<GLuint, std::shared_ptr<SP::Mesh>>::iterator iter;
		for (iter = mID2Mesh.begin(); iter != mID2Mesh.end(); iter++)
		{
			mpPlaneScene->addMesh(iter->second, pShaderOmni);
		}
	}
	
	~CustomOmniCamera() {}

	std::shared_ptr<SP::MaterialFBO> getMaterialFBO()
	{
		return mpMaterialFBO;
	}

	std::shared_ptr<SP::MaterialFBO> getMaterialFBOMask()
	{
		return mpMaterialFBOMask;
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

	virtual void setup(int winWidth, int winHeight)
	{
		if (mbCustomBufferSize)
		{
			OmniCamera::setup(mCustomBWidth, mCustomBHeight);
		}
		else
		{
			OmniCamera::setup(winWidth, winHeight);
		}

		_createMultiSampleMask();
		_createNormalMask();

		mpMaterialCubeFBO->setCubeTextureBuffer2(mCubeMaskTexture);

		mpMaterialFBO->clearUploaded();
		mpMaterialFBO->setTexbuffer(SP::TextureType::Tex_DIFFUSE, mColorTexture);
		mpMaterialFBO->uploadToDevice();

		mpMaterialFBOMask->clearUploaded();
		mpMaterialFBOMask->setTexbuffer(SP::TextureType::Tex_DIFFUSE, mMaskTexture);
		mpMaterialFBOMask->uploadToDevice();
	}

	virtual void renderSceneArray(const std::vector<std::shared_ptr<SP::Scene>>
								  &vpScene)
	{
		if (!mbDoRender) return;

		//return;
		if (!mbSetup)
		{
			SP_CERR("The current camera has not been uploaded befor rendering");
			return;
		}

		if (vpScene.size() == 0)
		{
			SP_CERR("The current vpScene for OmniCamera rendering is empty");
			return;
		}

		for (size_t i = 0; i < 6; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mCubeMSFBO);
			glViewport(0, 0, mFaceTexSide, mFaceTexSide);

			//Bind the vbo point
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mvCubeViewUBO[i]);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//draw the scenee
			for (size_t j = 0; j < vpScene.size(); j++)
			{
				const std::shared_ptr<SP::Scene> &pScene = vpScene[j];

				if (pScene.use_count() == 0) continue;
				//pScene->filterVisible(mCubeProjMatrix, mvCubeViewMatrix[i], mZNear, mZFar);
				pScene->draw();
			}

			//Copy the buffers from mCubeMSFBO to mvCubeFBO[i]
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mCubeMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mvCubeFBO[i]);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(0, 0, mFaceTexSide, mFaceTexSide,
							  0, 0, mFaceTexSide, mFaceTexSide,
							  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
							  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);

			glBlitFramebuffer(0, 0, mFaceTexSide, mFaceTexSide,
							  0, 0, mFaceTexSide, mFaceTexSide,
							  GL_COLOR_BUFFER_BIT, GL_NEAREST);


			/*int w, h, c;
			std::shared_ptr<unsigned char> pMaskData;
			readCubeMaskBuffer(i, pMaskData, w, h, c);
			cv::Mat cubeMask = cv::Mat(h, w, CV_8UC1, pMaskData.get());

			std::shared_ptr<unsigned char> pColorData;
			readCubeColorBuffer(i, pColorData, w, h, c);
			cv::Mat cubeColor = cv::Mat(h, w, CV_8UC3, pColorData.get());

			cv::Mat testline;*/
		}

		if (mpPlaneScene.use_count() != 0)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, VIEWUBO_BINDING_POINT, mViewUBO);
			glViewport(0, 0, mBWidth, mBHeight);

			glBindFramebuffer(GL_FRAMEBUFFER, mMSFBO);
			if (mbClearPerFrame)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			mpPlaneScene->draw();

			//Copy the color buffer from mMSFBO to mFBO
			glBindFramebuffer(GL_READ_FRAMEBUFFER, mMSFBO);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			glBlitFramebuffer(0, 0, mBWidth, mBHeight,
							  0, 0, mBWidth, mBHeight,
							  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
							  GL_STENCIL_BUFFER_BIT, GL_NEAREST);

			glReadBuffer(GL_COLOR_ATTACHMENT2);
			glDrawBuffer(GL_COLOR_ATTACHMENT2);

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
								  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								  GL_STENCIL_BUFFER_BIT, GL_NEAREST);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
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


	virtual void readCubeMaskBuffer(int cubeFace, std::shared_ptr<unsigned char> &pData,
								int &width, int &height, int &channel)
	{
		width = mFaceTexSide;
		height = mFaceTexSide;
		channel = 1;

		GLuint *dataRaw = new GLuint[width * height * channel];
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mvCubeFBO[cubeFace]);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
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

	virtual void readCubeColorBuffer(int cubeFace, std::shared_ptr<unsigned char> &pData,
								 int &width, int &height, int &channel)
	{
		width = mFaceTexSide;
		height = mFaceTexSide;
		channel = 3;

		unsigned char *data = new unsigned char[width * height * channel];
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mvCubeFBO[cubeFace]);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

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


private:
	bool mbCustomBufferSize;
	int mCustomBWidth, mCustomBHeight;

	GLuint mCubeMSMaskTexture, mCubeMaskTexture;

	GLuint mMSMaskTexture, mMaskTexture;

	std::shared_ptr<SP::MaterialFBO> mpMaterialFBO;
	std::shared_ptr<SP::MaterialFBO> mpMaterialFBOMask;

private:
	void _createMultiSampleMask()
	{
		int bufferW = mBWidth;
		int bufferH = mBHeight;

		{
			glBindFramebuffer(GL_FRAMEBUFFER, mCubeMSFBO);
			//MSAA for anti-aliasing
			glGenTextures(1, &mCubeMSMaskTexture);
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mCubeMSMaskTexture);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mNumSamples, GL_R32UI,
									mFaceTexSide, mFaceTexSide, GL_TRUE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
								   GL_TEXTURE_2D_MULTISAMPLE,
								   mCubeMSMaskTexture, 0);

			GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT1};
			glDrawBuffers(3, buffers);
		}
		
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

		{
			glGenTextures(1, &mCubeMaskTexture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMaskTexture);

			for (size_t i = 0; i < 6; i++)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
							 GL_R32UI, mFaceTexSide, mFaceTexSide, 0,
							 GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			//since texture coordinates that are exactly between two faces might 
			//not hit an exact face (due to some hardware limitations) so by using
			// GL_CLAMP_TO_EDGE OpenGL always return their edge values whenever
			// we sample between faces.
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}

		for (size_t i = 0; i < 6; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, mvCubeFBO[i]);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, mCubeMaskTexture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
								   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mCubeMaskTexture, 0);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

			GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(3, buffers);


			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
				GL_FRAMEBUFFER_COMPLETE)
			{
				SP_CERR("The Raw FrameBuffer of face[" << i << "] is \
								not complete");
				exit(-1);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

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
};
