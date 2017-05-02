#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include "../ArchImgProc/Detection/ArrowDetect.h"
#include "../ArchImgProc/Detection/LineSearcher.h"


class ArrowDetection
{
public:
	struct Parameters
	{
		int noHoughDistanceBins;
		int noHoughAngleBins;


		/// <summary>The deviation of the angle of a line-segment from the result-direction in order
		/// 		 to remove the line-segment from the line-segments available for further refinements
		/// 		 This quantiy is given in degrees.</summary>
		float maxDeviationOfAngle;

		void SetDefaults()
		{
			this->noHoughDistanceBins = 100;
			this->noHoughAngleBins = 100;
			this->maxDeviationOfAngle = 4.5;
		}
	};
private:

private:
	Parameters parameters;

	std::shared_ptr<cv::Mat> img;

	std::vector<cv::Vec4f > lines;
	std::vector<bool> usedLines;

	typedef ArchImgProc::CHoughOnLineSegments<float, size_t> tHoughOnLineSegments;
	typedef ArchImgProc::CHoughLineRefiner<float, cv::Vec4f> tHoughRefiner;

	std::shared_ptr<tHoughOnLineSegments> spHoughOnLs;

	int refinementCnt;
	std::vector<tHoughRefiner> houghRefined;
public:
	static std::shared_ptr<cv::Mat> LoadAndPreprocess(const std::string& strFilename);

	ArrowDetection(std::shared_ptr<cv::Mat> img);

	int GetBitmapWidth() const;
	int GetBitmapHeight() const;
	const std::vector<cv::Vec4f >& GetLines() const;

	void DoStep1();

	void DoStep2();

	bool DoRefinement();

	const std::vector<tHoughRefiner>& GetRefinedLines() const { return this->houghRefined; }
private:
	tHoughRefiner DoLineRefinement(const tHoughOnLineSegments::BinResult& binResult);
	void AddUsedLineSegments(const ArrowDetection::tHoughRefiner& refiner);
};
