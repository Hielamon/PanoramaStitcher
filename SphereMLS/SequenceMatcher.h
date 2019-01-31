#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <iostream>
#include <list>

struct PairInfo
{
	std::vector<cv::Point2f> points1, points2;
	std::vector<uchar> mask;
	int index1, index2;
	int pairs_num;
	int inliers_num;
};

class SequenceMatcher
{
public:
	//��������
	enum FeatureType
	{
		F_ORB, F_SIFT, F_SURF
	};

	SequenceMatcher(const FeatureType &featuretype = F_ORB, float scale = 1.0);

	~SequenceMatcher();
	
	//ʹ������ƥ��ķ����ҵ�һ�����ʵ�ͼƬƥ�����У����������ƥ�����е���Ϣ
	void process(const std::vector<cv::Mat>&images, std::list<PairInfo> &pairinfos);

	void processTwoImageOverCommonMask(const cv::Mat &image1, const cv::Mat &image2,
									   const cv::Mat &mask1, const cv::Mat &mask2,
									   PairInfo &pairInfo, int index1, int index2);

	//ʹ������ƥ��ķ����ҵ�һ�����ʵ�ͼƬƥ�����У����������ƥ�����е���Ϣ
	void processTest(const std::vector<cv::Mat>&images, std::list<PairInfo> &pairinfos);

	//��pairinfosƥ�����еĵ�һ��ͼ�����һ��ͼ��ƥ����Ϣ��push_back ��pairinfos��
	void getRingPair(std::list<PairInfo> &pairinfos);

	//��pairinfosƥ�����еĵ�һ��ͼ�����һ��ͼ��ƥ����Ϣ��push_back ��pairinfos��
	void getRingPairTest(std::list<PairInfo> &pairinfos);

	std::vector<std::vector<cv::KeyPoint> > m_keypoints_arr;
	std::vector<cv::Mat> m_descriptors_arr;

protected:
	FeatureType m_featuretype;
	cv::Ptr<cv::Feature2D>  m_featurator;
	cv::Ptr<cv::DescriptorMatcher> m_descriptorMatcher;

	std::vector<std::vector<PairInfo> > m_all_pairinfos;

	

	float m_scale;

	//��ȡimages��ÿ��ͼƬ���������Լ���������
	void _getFeatures(const std::vector<cv::Mat>&images, std::vector<std::vector<cv::KeyPoint> > &keypoints_arr, std::vector<cv::Mat> &descriptors_arr);

	//�ҵ�des1 �� descriptors2�е�����ڶ�Ӧ�� des2��������des2 �� descriptors1�е�����ڶ�Ӧ��Ϊdes1
	bool _getCorresponds(const cv::Mat &descriptors1, const cv::Mat &descriptors2, std::vector<cv::DMatch>& matches, double ratio = 0.5);

	//�õ��ڵ����
	float _getInlierRate(const std::vector<char>& inliers);

};


void DrawPairInfos(std::vector<cv::Mat> &images, std::list<PairInfo> &pairinfos, std::string name = "pairs_", bool onlyPoints = false, double scale = 1.0);

void ShowPairInfos(std::vector<cv::Mat> &images, std::list<PairInfo> &pairinfos, double scale);