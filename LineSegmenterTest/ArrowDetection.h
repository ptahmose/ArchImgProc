#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include "../ArchImgProc/Detection/ArrowDetect.h"


class ArrowDetection
{
public:
	struct Parameters
	{
		int noHoughDistanceBins;
		int noHoughAngleBins;

		void SetDefaults()
		{
			this->noHoughDistanceBins = 100;
			this->noHoughAngleBins = 100;
		}
	};
private:
	Parameters parameters;

	std::shared_ptr<cv::Mat> img;

	std::vector<cv::Vec4f > lines;

	std::shared_ptr<ArchImgProc::CHoughOnLineSegments<float, size_t>> spHoughOnLs;
public:
	static std::shared_ptr<cv::Mat> LoadAndPreprocess(const std::string& strFilename);

	ArrowDetection(std::shared_ptr<cv::Mat> img);

	void DoStep1();

	void DoStep2();
};
