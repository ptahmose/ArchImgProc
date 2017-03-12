// LineSegmenterTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ArchImgProc/archimgproc.h"
#include "WriteOutData.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace ArchImgProc;
using namespace cv;

static void TestBitmap1()
{
	auto bm = CreateBitmapStd(PixelType::Gray8, 10, 10);
	//auto bm2= LoadBitmapFromFile(LR"(W:\Temp\Archery\MATLAB\ShowResultpk73.png)");
	auto bm2 = LoadBitmapFromFile(LR"(W:\TESTWRITE_gaussfiltered.png)");

	auto lineSegments = LineSegmentDetection::DoLSD(bm2.get(), 0.6f, 0.6f);

	SaveBitmapToFileAsPng(bm2, LR"(W:\TESTWRITE.PNG)");

	CWriteOutData::WriteLineSegmentsAsSvg<float>(lineSegments.cbegin(), lineSegments.cend(), LR"(W:\test.svg)");
}

static void TestBitmap2()
{
	Mat src = imread(R"(W:\Temp\Archery\MATLAB\ShowResultpk73.png)", 1);
	//auto dst = src.clone();
	//GaussianBlur(src, dst, Size(0, 0),3,3);
	Mat dst(src.rows / 2, src.cols / 2, src.type());
	pyrDown(src, dst);

	

	//Mat greyMat;
	//cv::cvtColor(dst, greyMat, COLOR_BGR2GRAY);
	Mat greyMat(dst.rows, dst.cols, CV_8U);
	ArchImgProc::Utils::ConvertBgr24ToGray8_HSV(ArchImgProc::PixelType::BGR24, dst.ptr(), dst.cols, dst.rows, dst.step[0], greyMat.ptr(), greyMat.step[0]);

	imshow("Gaussian", greyMat);

	std::vector< Vec4f > lines;
	auto lsd = createLineSegmentDetector(LineSegmentDetectorModes::LSD_REFINE_STD,1);
	lsd->detect(greyMat, lines);
	auto it = lines.cbegin();
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (it == lines.cend())
		{
			return false;
		}

		x1 = it->val[0];
		y1 = it->val[1];
		x2 = it->val[2];
		y2 = it->val[3];
		++it;
		return true;
	},
		greyMat.cols, greyMat.rows,
		LR"(W:\test_OCV.svg)");

	auto lsd2 = LineSegmentDetection::DoLSD(ArchImgProc::PixelType::Gray8, greyMat.ptr(), greyMat.step[0], greyMat.cols, greyMat.rows);
	CWriteOutData::WriteLineSegmentsAsSvg<float>(lsd2.cbegin(), lsd2.cend(), LR"(W:\test_LSD.svg)");
}

int main()
{
	CoInitialize(nullptr);

	//TestBitmap1();

	TestBitmap2();


	waitKey(0);

	CoUninitialize();

	return 0;
}

