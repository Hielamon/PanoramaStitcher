#include <OpencvCommon.h>

int main(int arg, char *argv[])
{
	std::string path = "D:\\Funny-Works\\Academic-Codes\\SLAM\\MultiCol-SLAM\\indoor_dynamic\\imgs";
	std::vector<std::string> cameraFolder = { "cam0", "cam1", "cam2" };
	std::string imageName = "img899.png";
	for (size_t i = 0; i < cameraFolder.size(); i++)
	{
		std::string imagePath = path + "\\" + cameraFolder[i] + "\\" + imageName;
		cv::Mat img = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
		cv::imshow(cameraFolder[i], img);
	}
	cv::waitKey(0);

	return 0;
}