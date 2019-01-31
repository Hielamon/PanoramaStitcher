#include "FishCamera.h"


namespace CircleFish
{

	FishCamera::FishCamera() {}

	FishCamera::FishCamera(const std::shared_ptr<CameraModel> & _pModel, const std::shared_ptr<Rotation> & _pRot)
	{
		pModel = _pModel;
		pRot = _pRot;
	}

	FishCamera::~FishCamera() {}

	bool FishCamera::SaveToXML(cv::FileStorage &fs, int index)
	{
		//Save the CameraModel info
		if (!fs.isOpened() || pModel.use_count() == 0 ||
			pRot.use_count() == 0) return false;

		std::string indexStr = "";
		std::stringstream ioStr;
		if (index >= 0)
		{
			ioStr.str("");
			ioStr << "_C" << index;
			indexStr = ioStr.str();
		}


		fs << "CModelName" + indexStr << pModel->getTypeName();
		fs << "imgW" + indexStr << imgW << "imgH" + indexStr << imgH;

		fs << "f" + indexStr << pModel->f << "u0" + indexStr << pModel->u0 << "v0" + indexStr << pModel->v0;
		fs << "fov" + indexStr << pModel->fov << "maxRadius" + indexStr << pModel->maxRadius;
		int extraParamNum = pModel->vpParameter.size() - 3;
		assert(extraParamNum == 0 || extraParamNum == 2);
		fs << "extraParamNum" + indexStr << extraParamNum;

		for (size_t i = 3; i < pModel->vpParameter.size(); i++)
		{
			ioStr.str("");
			ioStr << "arg_" << i << indexStr;
			fs << ioStr.str() << *(pModel->vpParameter[i]);
		}

		//Save the Rotation
		fs << "RotationMat" + indexStr << pRot->R;

		return true;
	}

	bool FishCamera::LoadFromXML(cv::FileStorage &fs, int index)
	{
		if (!fs.isOpened()) return false;

		std::string indexStr = "";
		std::stringstream ioStr;
		if (index >= 0)
		{
			ioStr.str("");
			ioStr << "_C" << index;
			indexStr = ioStr.str();
		}

		//Load the CameraModel info
		std::string CModelName;
		fs["CModelName" + indexStr] >> CModelName;
		fs["imgW" + indexStr] >> imgW;
		fs["imgH" + indexStr] >> imgH;

		double f, u0, v0, fov, maxRadius;
		fs["f" + indexStr] >> f; fs["u0" + indexStr] >> u0; fs["v0" + indexStr] >> v0;
		fs["fov" + indexStr] >> fov; fs["maxRadius" + indexStr] >> maxRadius;

		int extraParamNum;
		fs["extraParamNum" + indexStr] >> extraParamNum;
		assert(extraParamNum == 0 || extraParamNum == 2);
		double args[2] = { 0 , 0 };
		for (size_t i = 0; i < extraParamNum; i++)
		{
			ioStr.str("");
			ioStr << "arg_" << i + 3 << indexStr;
			fs[ioStr.str()] >> args[i];
		}

		pModel = createCameraModel(CModelName, u0, v0, f, fov, maxRadius, args[0], args[1]);

		//Load the Rotation
		cv::Mat R;
		fs["RotationMat" + indexStr] >> R;
		pRot = std::make_shared<Rotation>(R);

		return true;
	}

	//Mapping the Sphere Plane 2D point to Unit Sphere 3D point
	void mapSP2S(cv::Point2d &sp_pt, cv::Point3d &s_pt, int sphere_height, double pi_sh)
	{
		double center = sphere_height * 0.5;
		double dcoord_x = sp_pt.x - center;
		double dcoord_y = -sp_pt.y + center;
		double theta = dcoord_x * pi_sh;
		double phi = dcoord_y * pi_sh;
		// Vector in 3D space
		double v_x = cos(phi)*sin(theta);
		double v_y = sin(phi);
		double v_z = cos(phi)*cos(theta);

		s_pt = cv::Point3d(v_x, v_y, v_z);
	}

	//Mapping the Unit Sphere 3D point to Sphere Plane 2D point
	void mapS2SP(cv::Point3d &s_pt, cv::Point2d &sp_pt, int sphere_height, double pi_sh)
	{
		double center = sphere_height * 0.5;

		double theta = std::atan2(s_pt.x, s_pt.z);
		double phi = std::asin(s_pt.y);

		double dcoord_x = theta / pi_sh;
		double dcoord_y = phi / pi_sh;

		sp_pt.x = dcoord_x + center;
		sp_pt.y = -dcoord_y + center;
	}

	//Mapping the Sphere Plane 2D point to Origin Image 2D point
	void mapSP2I(cv::Point2d &sp_pt, cv::Point2d &img_pt, int sphere_height, double pi_sh, const FishCamera &C)
	{
		cv::Point3d s_pt;
		mapSP2S(sp_pt, s_pt, sphere_height, pi_sh);

		double *R_ptr = reinterpret_cast<double *>((C.pRot->R).data);

		double _x = R_ptr[0] * s_pt.x + R_ptr[1] * s_pt.y + R_ptr[2] * s_pt.z;
		double _y = R_ptr[3] * s_pt.x + R_ptr[4] * s_pt.y + R_ptr[5] * s_pt.z;
		double _z = R_ptr[6] * s_pt.x + R_ptr[7] * s_pt.y + R_ptr[8] * s_pt.z;

		s_pt = cv::Point3d(_x, _y, _z);
		C.pModel->mapS2I(s_pt, img_pt);
	}

	//Mapping Origin Image 2D point to the Sphere Plane 2D point
	void mapI2SP(cv::Point2d &img_pt, cv::Point2d &sp_pt, int sphere_height, double pi_sh, const FishCamera &C)
	{
		cv::Point3d s_pt;
		C.pModel->mapI2S(img_pt, s_pt);

		double *R_ptr = reinterpret_cast<double *>((C.pRot->R).data);

		double _x = R_ptr[0] * s_pt.x + R_ptr[3] * s_pt.y + R_ptr[6] * s_pt.z;
		double _y = R_ptr[1] * s_pt.x + R_ptr[4] * s_pt.y + R_ptr[7] * s_pt.z;
		double _z = R_ptr[2] * s_pt.x + R_ptr[5] * s_pt.y + R_ptr[8] * s_pt.z;

		s_pt = cv::Point3d(_x, _y, _z);
		mapS2SP(s_pt, sp_pt, sphere_height, pi_sh);
	}

	//Build the map from sphere plane ROI to the origin image
	void buildMap(cv::Rect roi, int sphere_height, const FishCamera &C, cv::Mat &map)
	{
		map = cv::Mat(roi.height, roi.width, CV_32FC2, cv::Scalar(0));
		int center = sphere_height * 0.5;
		double fov = CV_PI;
		double pi_sh = fov / sphere_height;


		int x_start = roi.x, y_start = roi.y, x_end = roi.x + roi.width, y_end = roi.y + roi.height;
		double *R_ptr = reinterpret_cast<double *>((C.pRot->R).data);
		for (int i = y_start, index_y = 0; i < y_end; i++, index_y++)
		{
			float *map_row_ptr = reinterpret_cast<float *>(map.ptr(index_y));
			for (int j = x_start, index_x = 0; j < x_end; j++, index_x += 2)
			{
				cv::Point2d sp_pt(j, i), img_pt(0, 0);
				mapSP2I(sp_pt, img_pt, sphere_height, pi_sh, C);

				map_row_ptr[index_x] = img_pt.x;
				map_row_ptr[index_x + 1] = img_pt.y;
			}
		}

	}


}
