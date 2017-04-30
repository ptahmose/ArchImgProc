#include "stdafx.h"
#include "ArrowDetection.h"
#include <opencv2/imgcodecs.hpp>
#include "../ArchImgProc/Bitmap/IBitmap.h"

using namespace std;
using namespace cv;


/*static*/std::shared_ptr<cv::Mat> ArrowDetection::LoadAndPreprocess(const std::string& strFilename)
{
	Mat src = imread(strFilename.c_str(), 1);
	Mat dst(src.rows / 2, src.cols / 2, src.type());
	pyrDown(src, dst);

	std::shared_ptr<cv::Mat> spGreyMat = std::make_shared<cv::Mat>(dst.rows, dst.cols, CV_8U);
	ArchImgProc::Utils::ConvertBgr24ToGray8_HSV(ArchImgProc::PixelType::BGR24, dst.ptr(), dst.cols, dst.rows, dst.step[0], spGreyMat->ptr(), spGreyMat->step[0]);
	return spGreyMat;
}

ArrowDetection::ArrowDetection(std::shared_ptr<cv::Mat> img)
	:img(img)
{
	this->parameters.SetDefaults();
}

void ArrowDetection::DoStep1()
{
	std::vector< Vec4f > lines;
	auto lsd = createLineSegmentDetector(LineSegmentDetectorModes::LSD_REFINE_STD, 1);
	lsd->detect(*this->img, this->lines);
}

void ArrowDetection::DoStep2()
{
	float centerX = this->img->cols / 2.0f;
	float centerY = this->img->rows / 2.0f;
	this->spHoughOnLs = std::make_shared < ArchImgProc::CHoughOnLineSegments<float, size_t>>(
		ArchImgProc::CUtils::CalcDistance(0.f, 0.f, centerX, centerY),
		this->parameters.noHoughDistanceBins,
		this->parameters.noHoughAngleBins);

	for (auto it = this->lines.cbegin(); it != this->lines.cend(); ++it)
	{
		this->spHoughOnLs->Add(
			std::distance(this->lines.cbegin(), it),
			it->operator[](0) - centerX,
			it->operator[](1) - centerY,
			it->operator[](2) - centerX,
			it->operator[](3) - centerY);
	}

	this->spHoughOnLs->Sort();
}