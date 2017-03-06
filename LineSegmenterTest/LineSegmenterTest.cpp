// LineSegmenterTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../ArchImgProc/archimgproc.h"

using namespace ArchImgProc;

static void TestBitmap1()
{
	auto bm = CreateBitmapStd(PixelType::Gray8, 10, 10);
}


int main()
{
	TestBitmap1();

    return 0;
}

