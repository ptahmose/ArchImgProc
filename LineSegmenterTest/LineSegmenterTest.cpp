// LineSegmenterTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ArchImgProc/archimgproc.h"
#include "WriteOutData.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../ArchImgProc/Detection/ArrowDetect.h"

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

//struct OrigLineSegment
//{
//	size_t index;
//	int iteration;
//};
//
//struct RefinedLineSegment
//{
//	float x1, y1, x2, y2;
//};

static void WriteHougLines(float length, float angleMin, float angleMax, float distMin, float distMax, const std::vector<Vec4f>& lines, int width, int height, float centerX, float centerY, const std::wstring& filename)
{
	std::vector<size_t> vecIndex;
	std::vector<Vec4f>::const_iterator iter = lines.cbegin();
	CHoughOnLineSegments<float, size_t>::FindItemsInRange(
		[&](size_t& idx, float& x1, float& y1, float& x2, float& y2)->bool
	{
		if (iter != lines.cend())
		{
			idx = std::distance(lines.cbegin(), iter);
			x1 = iter->val[0] - centerX;
			y1 = iter->val[1] - centerY;
			x2 = iter->val[2] - centerX;
			y2 = iter->val[3] - centerY;
			++iter;
			return true;
		}

		return false;
	},
		angleMin, angleMax, distMin, distMax, vecIndex);

	CHoughLineRefiner<float, Vec4f> refiner(lines, vecIndex, width, height,nullptr);
	refiner.Refine();

	size_t i = 0; 
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (i < refiner.GetResultLineSegmentsCount())
		{
			CHoughLineRefiner<float, Vec4f>::ResultLineSegment lsResult;
			bool b = refiner.GetResultLineSegment(i, &lsResult);
			assert(b == true);
			x1 = lsResult.p1.x; x2 = lsResult.p2.x;
			y1 = lsResult.p1.y; y2 = lsResult.p2.y;

			*pStrokewidth = 3;
			color = "red";
		}
		else
		{
			size_t j = i - refiner.GetResultLineSegmentsCount();
			if (j>=lines.size())
			{
				return false;
			}

			int foundInIteration = -1;
			refiner.EnumOriginalLineSegments(
				[&](size_t index, int iteration, float x1, float y1, float x2, float y2)->bool
			{
				if (j == index)
				{
					foundInIteration = iteration;
					return false;
				}

				return true;
			});

			x1 = lines[j].val[0]; y1 = lines[j].val[1]; x2 = lines[j].val[2]; y2 = lines[j].val[3];

			if (foundInIteration<0)
			{
				color = "black";
			}
			else
			{
				switch (foundInIteration)
				{
				case 0:
					color = "orange";
					break;
				case 1:
					color = "blue";
					break;
				case 2:
					color = "green";
					break;
				default:
					color = "yellow";
					break;
				}
			}
		}

		++i;
		return true;
	},
		width, height,
		filename.c_str());
	/*
	std::vector<OrigLineSegment> origLineSegments;
	refiner.EnumOriginalLineSegments(
		[&](size_t index, int iteration, float x1, float y1, float x2, float y2)->bool
	{
		origLineSegments.push_back(OrigLineSegment{ index, iteration });
		return true;
	});

	std::vector<RefinedLineSegment> refinedLineSegments;
	refiner.EnumOriginalLineSegments(
		[&](float x1, float y1, float x2, float y2)->bool
	{
		refinedLineSegments.push_back(RefinedLineSegment{ x1,y1,x2,y2 });
		return true;
	});

	size_t i = 0;
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (i >= lines.size())
		{
			size_t i2 = i - lines.size();
			if (i2 >= refinedLineSegments.size())
			{
				return false;
			}

			x1 = refinedLineSegments[i2].x1; x2 = refinedLineSegments[i2].x2;
			y1 = refinedLineSegments[i2].y1; y2 = refinedLineSegments[i2].y2;

			*pStrokewidth = 3;
			color = "red";
			++i;
			return true;
		}

		auto foundLs = std::find_if(origLineSegments.cbegin(), origLineSegments.cend(), [=](const OrigLineSegment& ols)->bool {return ols.index == i; });
		if (foundLs == origLineSegments.cend())
		{
			color = "black";
		}
		else
		{
			switch (foundLs->iteration)
			{
			case 0:
				color = "orange";
				break;
			case 1:
				color = "blue";
				break;
			case 2:
				color = "green";
				break;
			default:
				color = "yellow";
				break;
			}
		}

		x1 = lines[i].val[0]; y1 = lines[i].val[1]; x2 = lines[i].val[2]; y2 = lines[i].val[3];

		++i;
		return true;
	},
		width, height,
		filename.c_str());*/
}

static void HoughTest(const std::vector<Vec4f>& lines, int width, int height)
{
	float centerX = width / 2;
	float centerY = height / 2;

	CHoughOnLineSegments<float, size_t> hough(CUtils::CalcDistance(0.f, 0.f, centerX, centerY), 100, 100);
	for (size_t i = 0; i < lines.size(); ++i)
	{
		const auto& l = lines[i];
		/*if (i==135)
			assert(i != 135);*/
		hough.Add(i, l[0] - centerX, l[1] - centerY, l[2] - centerX, l[3] - centerY);
	}

	hough.Sort();

	for (int no = 0; no < 20; ++no)
	{
		float length, angleMin, angleMax, distMin, distMax;
		hough.GetAngleAndDistanceMaxMinSortedByCount(no, &length, &angleMin, &angleMax, &distMin, &distMax);

		wstringstream filename;
		filename << LR"(W:\TestRun_OCV_)" << no << LR"(.svg)";
		float a1 = CUtils::RadToDeg(angleMin);
		float a2 = CUtils::RadToDeg(angleMax);
		WriteHougLines(length, angleMin, angleMax, distMin, distMax, lines, width, height, centerX, centerY, filename.str());
	}

	/*std::vector<size_t> vecIndex;
	std::vector<Vec4f>::const_iterator iter = lines.cbegin();
	CHoughOnLineSegments<float, size_t>::FindItemsInRange(
		[&](size_t& idx, float& x1, float& y1, float& x2, float& y2)->bool
	{
		if (iter != lines.cend())
		{
			idx = std::distance(lines.cbegin(), iter);
			x1 = iter->val[0] - centerX;
			y1 = iter->val[1] - centerY;
			x2 = iter->val[2] - centerX;
			y2 = iter->val[3] - centerY;
			++iter;
			return true;
		}

		return false;
	},
		angleMin, angleMax, distMin, distMax, vecIndex);

	CHoughLineRefiner<float, Vec4f> refiner(lines, vecIndex, width, height);
	refiner.Refine();

	std::vector<OrigLineSegment> origLineSegments;
	refiner.EnumOriginalLineSegments(
		[&](size_t index, int iteration, float x1, float y1, float x2, float y2)->bool
	{
		origLineSegments.push_back(OrigLineSegment{ index, iteration });
		return true;
	});

	std::vector<RefinedLineSegment> refinedLineSegments;
	refiner.EnumOriginalLineSegments(
		[&](float x1, float y1, float x2, float y2)->bool
	{
		refinedLineSegments.push_back(RefinedLineSegment{ x1,y1,x2,y2 });
		return true;
	});

	size_t i = 0;
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (i >= lines.size())
		{
			size_t i2 = i - lines.size();
			if (i2 >= refinedLineSegments.size())
			{
				return false;
			}

			x1 = refinedLineSegments[i2].x1; x2 = refinedLineSegments[i2].x2;
			y1 = refinedLineSegments[i2].y1; y2 = refinedLineSegments[i2].y2;

			*pStrokewidth = 3;
			color = "red";
			++i;
			return true;
		}

		auto foundLs = std::find_if(origLineSegments.cbegin(), origLineSegments.cend(), [=](const OrigLineSegment& ols)->bool {return ols.index == i; });
		if (foundLs == origLineSegments.cend())
		{
			color = "black";
		}
		else
		{
			switch (foundLs->iteration)
			{
			case 0:
				color = "orange";
				break;
			case 1:
				color = "blue";
				break;
			case 2:
				color = "green";
				break;
			default:
				color = "yellow";
				break;
			}
		}

		x1 = lines[i].val[0]; y1 = lines[i].val[1]; x2 = lines[i].val[2]; y2 = lines[i].val[3];

		++i;
		return true;
	},
		width, height,
		LR"(W:\test_OCV_4.svg)");*/

#if false
	auto additional = refiner.Refine();

	bool resultUsed = false;
	auto it = lines.cbegin();
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (it == lines.cend())
		{
			/*	if (resultUsed == true)
				{
					return false;
				}

				x1 = result.pt1.x; y1 = result.pt1.y; x2 = result.pt2.x; y2 = result.pt2.y;
				color = "green";
				resultUsed = true;
				return true;*/
			return false;
		}

		size_t idx = std::distance(lines.cbegin(), it);
		if (std::find(vecIndex.cbegin(), vecIndex.cend(), idx) != vecIndex.cend())
		{
			color = "red";
		}
		else
		{
			if (std::find(additional.cbegin(), additional.cend(), idx) != additional.cend())
			{
				color = "blue";
			}
			else
			{
				color = "black";
			}
		}

		//float angle, distance;
		//CUtils::ConvertToHessianNormalForm(it->val[0] - centerX, it->val[1] - centerY, it->val[2] - centerX, it->val[3] - centerY, &angle, &distance);
		//if ((angleMin <= angle&&angleMax >= angle) && (distMin <= distance&&distMax >= distance))
		//{
		//	auto idx = std::distance(lines.cbegin(), it);
		//	color = "red";
		//}

		x1 = it->val[0]; y1 = it->val[1]; x2 = it->val[2]; y2 = it->val[3];

		++it;
		return true;
	},
		width, height,
		LR"(W:\test_OCV_3.svg)");

	std::vector<size_t> vecIndex2;
	vecIndex2.reserve(vecIndex.size() + additional.size());
	std::copy(vecIndex.cbegin(), vecIndex.cend(), std::back_inserter(vecIndex2));
	std::copy(additional.cbegin(), additional.cend(), std::back_inserter(vecIndex2));

	CHoughLineRefiner<float, Vec4f> refiner2(lines, vecIndex2, width, height);
	auto additional2 = refiner2.Refine();
	resultUsed = false;
	it = lines.cbegin();
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (it == lines.cend())
		{
			/*	if (resultUsed == true)
			{
			return false;
			}

			x1 = result.pt1.x; y1 = result.pt1.y; x2 = result.pt2.x; y2 = result.pt2.y;
			color = "green";
			resultUsed = true;
			return true;*/
			return false;
		}

		size_t idx = std::distance(lines.cbegin(), it);
		if (std::find(vecIndex2.cbegin(), vecIndex2.cend(), idx) != vecIndex2.cend())
		{
			color = "red";
		}
		else
		{
			if (std::find(additional2.cbegin(), additional2.cend(), idx) != additional2.cend())
			{
				color = "blue";
			}
			else
			{
				color = "black";
			}
		}

		//float angle, distance;
		//CUtils::ConvertToHessianNormalForm(it->val[0] - centerX, it->val[1] - centerY, it->val[2] - centerX, it->val[3] - centerY, &angle, &distance);
		//if ((angleMin <= angle&&angleMax >= angle) && (distMin <= distance&&distMax >= distance))
		//{
		//	auto idx = std::distance(lines.cbegin(), it);
		//	color = "red";
		//}

		x1 = it->val[0]; y1 = it->val[1]; x2 = it->val[2]; y2 = it->val[3];

		++it;
		return true;
	},
		width, height,
		LR"(W:\test_OCV_4.svg)");
#endif


#if false
	std::vector<size_t>::const_iterator itIndices = vecIndex.cbegin();
	float angleAverage = CUtils::CalculateAverage<float>(
		[&](float& v)->bool
	{
		if (itIndices == vecIndex.cend())
		{
			return false;
		}

		float angle;
		CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, &angle, (float*)nullptr);
		++itIndices;
		v = angle;
	});

	itIndices = vecIndex.cbegin();
	float distAverage = CUtils::CalculateAverage<float>(
		[&](float& v)->bool
	{
		if (itIndices == vecIndex.cend())
		{
			return false;
		}

		float distance;
		CUtils::ConvertToHessianNormalForm(lines[*itIndices].val[0] - centerX, lines[*itIndices].val[1] - centerY, lines[*itIndices].val[2] - centerX, lines[*itIndices].val[3] - centerY, (float*)nullptr, &distance);
		++itIndices;
		v = distance;
	});

	ArchImgProc::Point<float> p1, p2;
	//CUtils::ConvertFromHessianToPointAndVector(angleAverage, distAverage, &pointX, &pointY, &directionX, &directionY);
	CsgUtils::HesseNormalFormToTwoPoints(angleAverage, distAverage, p1, p2);
	p1.x += centerX; p2.x += centerX;
	p1.y += centerY; p2.y += centerY;
	// pointX/Y is now a point on the line, and directionX/Y is the direction vector


	// get a unit-vector in direction from p1 to p2
	auto dirX = p2.x - p1.x;
	auto dirY = p2.y - p1.y;
	auto lengthDir = sqrt(dirX*dirX + dirY*dirY);
	dirX /= lengthDir; dirY /= lengthDir;

	itIndices = vecIndex.cbegin();
	auto lsStartStop = CLineSearcher<float>::CreateLineSegmentsStartStop(p1, ArchImgProc::Point<float> { p1.x + dirX, p1.y + dirY },
		[&](ArchImgProc::Point<float>& p1Ls, ArchImgProc::Point<float>& p2LS)->bool
	{
		if (itIndices == vecIndex.cend())
		{
			return false;
		}

		p1Ls = ArchImgProc::Point<float>{ lines[*itIndices].val[0], lines[*itIndices].val[1] };
		p2LS = ArchImgProc::Point<float>{ lines[*itIndices].val[2] , lines[*itIndices].val[3] };
		++itIndices;
		return true;
	});

	CLineSearcher<float>::Sort(lsStartStop);
	auto linked = CLineSearcher<float>::LinkOverlapping(lsStartStop);
	auto linked2 = CLineSearcher<float>::LinkSmallGaps(linked, 10);

	auto result = CsgUtils::CalcIntersectionPoints(p1, p2, IntRect{ 0, 0, width, height });

	bool resultUsed = false;
	auto it = lines.cbegin();
	CWriteOutData::WriteLineSegmentsAsSvg<float>(
		[&](float& x1, float& y1, float& x2, float& y2, float* pStrokewidth, std::string& color)->bool
	{
		if (it == lines.cend())
		{
			if (resultUsed == true)
			{
				return false;
			}

			x1 = result.pt1.x; y1 = result.pt1.y; x2 = result.pt2.x; y2 = result.pt2.y;
			color = "green";
			resultUsed = true;
			return true;
		}

		float angle, distance;
		CUtils::ConvertToHessianNormalForm(it->val[0] - centerX, it->val[1] - centerY, it->val[2] - centerX, it->val[3] - centerY, &angle, &distance);
		if ((angleMin <= angle&&angleMax >= angle) && (distMin <= distance&&distMax >= distance))
		{
			auto idx = std::distance(lines.cbegin(), it);
			color = "red";
		}

		x1 = it->val[0]; y1 = it->val[1]; x2 = it->val[2]; y2 = it->val[3];

		++it;
		return true;
},
width, height,
LR"(W:\test_OCV_2.svg)");
#endif
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
	auto lsd = createLineSegmentDetector(LineSegmentDetectorModes::LSD_REFINE_STD, 1);
	lsd->detect(greyMat, lines);

	HoughTest(lines, greyMat.cols, greyMat.rows);

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

