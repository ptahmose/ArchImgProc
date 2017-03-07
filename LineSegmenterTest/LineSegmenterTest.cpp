// LineSegmenterTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ArchImgProc/archimgproc.h"

using namespace ArchImgProc;

static void TestBitmap1()
{
	auto bm = CreateBitmapStd(PixelType::Gray8, 10, 10);
	auto bm2= LoadBitmapFromFile(LR"(W:\Temp\Archery\MATLAB\ShowResultpk73.png)");

	auto lineSegments = LineSegmentDetection::DoLSD(bm2.get());
}


int main()
{
	CoInitialize(nullptr);
	TestBitmap1();

	CoUninitialize();
    return 0;
}

