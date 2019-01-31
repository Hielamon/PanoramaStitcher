#pragma once
#include <SPhoenix\Manipulator.h>

#include <direct.h>
#include <io.h>

namespace SP
{
	class SphereStitcherManipulator : public MonitorManipulator
	{
	public:
		SphereStitcherManipulator(const std::shared_ptr<MonitorWindow> &pMonitorWindow,
									const std::string &saveFolder)
			: MonitorManipulator(pMonitorWindow), mbSave(false)
		{
			
			mSaveFolderBase = saveFolder;
			mSaveFrameIndex = 0;
		}

		~SphereStitcherManipulator() {}

		//Some tasks need to be processed for every frame
		//Such as the movement of cameras
		virtual void doFrameTasks()
		{

			if (mbSave && mSaveFrameIndex == 0)
			{
				int existedCount = 0;

				do
				{
					std::stringstream ioStr;
					ioStr << mSaveFolderBase << "-" << std::setw(4) << std::setfill('0') << existedCount;
					mSaveFolder = ioStr.str();

					existedCount++;

				} while (_access(mSaveFolder.c_str(), 0) != -1);

				_mkdir(mSaveFolder.c_str());
			}

			if (mbSave)
			{
				std::stringstream ioStr;
				ioStr << mSaveFolder << "/ShotFrame-" << std::setw(4) << 
					std::setfill('0') << mSaveFrameIndex << ".jpg";

				{
					std::string saveFullPath = ioStr.str();
					std::shared_ptr<unsigned char> pData;
					int width, height, channel;
					mpMonitorWindow->getDefaultCamera()->readColorBuffer(pData, width, height, channel);
					SOIL_save_image(saveFullPath.c_str(), SOIL_SAVE_TYPE_BMP,
									width, height, channel, pData.get());
				}

				mSaveFrameIndex++;
				mbSave = false;
			}

			MonitorManipulator::doFrameTasks();
			return;
		}

	protected:
		virtual void keyCallBackImpl(GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			switch (key)
			{
			case GLFW_KEY_B:
				if (action == GLFW_PRESS)
				{
					//if (mKeyState[GLFW_KEY_RIGHT_SHIFT])
					{
						mbSave = true;
						std::cout << "save current fisheye frames" << std::endl;
					}
					/*else
					{
						mbSave = false;
						std::cout << "Stop to save fisheye frames" << std::endl;
					}*/
				}
				break;
			default:
				break;
			}

			MonitorManipulator::keyCallBackImpl(window, key, scancode, action, mods);
		}

	private:
		bool mbSave;
		std::string	mSaveFolder, mSaveFolderBase;
		int mSaveFrameIndex;
	};
}

