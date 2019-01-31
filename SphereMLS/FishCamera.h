#pragma once
#include <OpencvCommon.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <sstream>
#include <string>
#include <numeric>
#include <fstream>

#include "CameraModel.h"
#include "Rotation.h"


namespace CircleFish
{
	class FishCamera
	{
	public:
		FishCamera(const std::shared_ptr<CameraModel> & _pModel, const std::shared_ptr<Rotation> & _pRot);

		~FishCamera();

		std::shared_ptr<CameraModel> pModel;
		std::shared_ptr<Rotation> pRot;

		int imgW, imgH;

		bool SaveToXML(cv::FileStorage &fs, int index = -1);

		bool LoadFromXML(cv::FileStorage &fs, int index = -1);

		//private:
		FishCamera();
	};

	//Mapping the Sphere Plane 2D point to Unit Sphere 3D point
	void mapSP2S(cv::Point2d &sp_pt, cv::Point3d &s_pt, int sphere_height, double pi_sh);

	//Mapping the Unit Sphere 3D point to Sphere Plane 2D point
	void mapS2SP(cv::Point3d &s_pt, cv::Point2d &sp_pt, int sphere_height, double pi_sh);

	//Mapping the Sphere Plane 2D point to Origin Image 2D point
	void mapSP2I(cv::Point2d &sp_pt, cv::Point2d &img_pt, int sphere_height, double pi_sh, const FishCamera &C);

	//Mapping Origin Image 2D point to the Sphere Plane 2D point
	void mapI2SP(cv::Point2d &img_pt, cv::Point2d &sp_pt, int sphere_height, double pi_sh, const FishCamera &C);

	//Build the map from sphere plane ROI to the origin image
	void buildMap(cv::Rect roi, int sphere_height, const FishCamera &C, cv::Mat &map);


}
