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
	:img(img), refinementCnt(0)
{
	this->parameters.SetDefaults();
}

int ArrowDetection::GetBitmapWidth() const
{
	return this->img->cols;
}

int ArrowDetection::GetBitmapHeight() const
{
	return this->img->rows;
}

const std::vector<cv::Vec4f >& ArrowDetection::GetLines() const
{
	return this->lines;
}

void ArrowDetection::DoStep1()
{
	std::vector< Vec4f > lines;
	auto lsd = createLineSegmentDetector(LineSegmentDetectorModes::LSD_REFINE_STD, 1);
	lsd->detect(*this->img, this->lines);
	this->usedLines.resize(this->lines.size(), false);
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

bool ArrowDetection::DoRefinement()
{
	tHoughOnLineSegments::BinResult binResult;
	this->spHoughOnLs->GetAngleAndDistanceMaxMinSortedByCount(
		this->refinementCnt, &binResult);

	++this->refinementCnt;
	auto refinedLine = this->DoLineRefinement(binResult);

	if (refinedLine.IsAcceptable())
	{
		this->AddUsedLineSegments(refinedLine);
		this->houghRefined.emplace_back(refinedLine);
		return true;
	}

	return false;
}

ArrowDetection::tHoughRefiner ArrowDetection::DoLineRefinement(const ArrowDetection::tHoughOnLineSegments::BinResult& binResult)
{
	float centerX = this->img->cols / 2.0f;
	float centerY = this->img->rows / 2.0f;

	std::vector<size_t> vecIndex;
	size_t index = 0;
	ArchImgProc::CHoughOnLineSegments<float, size_t>::FindItemsInRange(
		[&](size_t& idx, float& x1, float& y1, float& x2, float& y2)->bool
	{
		for (;;)
		{
			if (index >= this->lines.size())
			{
				return false;
			}

			if (this->usedLines[index] == false)
			{
				x1 = this->lines[index].operator[](0) - centerX;
				y1 = this->lines[index].operator[](1) - centerY;
				x2 = this->lines[index].operator[](2) - centerX;
				y2 = this->lines[index].operator[](3) - centerY;
				idx = index;
				++index;
				return true;
			}

			++index;
		}
	},
		binResult.angleMin, binResult.angleMax, binResult.distanceMin, binResult.distanceMax,
		vecIndex);

	ArchImgProc::CHoughLineRefiner<float, Vec4f> refiner(lines, vecIndex, this->img->cols, this->img->rows,
		[&](size_t idx)->bool
	{
		return !this->usedLines[idx];
	});

	refiner.Refine();
	return refiner;
}

void ArrowDetection::AddUsedLineSegments(const ArrowDetection::tHoughRefiner& refiner)
{
	for (size_t i = 0; i < refiner.GetResultLineSegmentsCount(); ++i)
	{
		tHoughRefiner::ResultLineSegment lsResult;
		refiner.GetResultLineSegment(i, &lsResult);
		for (auto it = lsResult.origIndices.cbegin(); it != lsResult.origIndices.cend(); ++it)
		{
			this->usedLines[*it] = true;
		}
	}
}