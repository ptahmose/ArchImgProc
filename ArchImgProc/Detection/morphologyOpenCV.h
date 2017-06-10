#pragma once

#include <opencv2/opencv.hpp>
#include <ellipseUtils.h>
#include <vector>

namespace ArchImgProc
{
	class COpenCVUtils
	{
	public:

		/// <summary>	Perform a morphological open - operation (in place). </summary>
		/// <param name="bm">						 	[in,out] The bitmap. </param>
		/// <param name="sizeOfStructuringElement_x">	X-size of the structuring element. </param>
		/// <param name="sizeOfStructuringElement_y">	Y-size of the structuring element. </param>
		static void Morphology_Open(ArchImgProc::IBitmapData* bm, int sizeOfStructuringElement_x, int sizeOfStructuringElement_y)
		{
			// see http://homepages.inf.ed.ac.uk/rbf/HIPR2/open.htm , http://docs.opencv.org/2.4/modules/imgproc/doc/filtering.html?highlight=getstructuringelement#Mat getStructuringElement(int shape, Size ksize, Point anchor)
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(sizeOfStructuringElement_x, sizeOfStructuringElement_y), cv::Point(-1, -1));	// -1 means: anchor is at the center
			cv::morphologyEx(mat, mat, cv::MORPH_OPEN, element);
			bm->Unlock();
		}

		static void Morphology_Closing2(ArchImgProc::IBitmapData* bm, int radius)
		{
			// see http://homepages.inf.ed.ac.uk/rbf/HIPR2/open.htm , http://docs.opencv.org/2.4/modules/imgproc/doc/filtering.html?highlight=getstructuringelement#Mat getStructuringElement(int shape, Size ksize, Point anchor)
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(radius, radius), cv::Point(-1, -1));	// -1 means: anchor is at the center
			cv::morphologyEx(mat, mat, cv::MORPH_CLOSE, element);
			bm->Unlock();
		}

		static void Morphology_Opening2(ArchImgProc::IBitmapData* bm, int radius)
		{
			// see http://homepages.inf.ed.ac.uk/rbf/HIPR2/open.htm , http://docs.opencv.org/2.4/modules/imgproc/doc/filtering.html?highlight=getstructuringelement#Mat getStructuringElement(int shape, Size ksize, Point anchor)
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(radius, radius), cv::Point(-1, -1));	// -1 means: anchor is at the center
			cv::morphologyEx(mat, mat, cv::MORPH_OPEN, element);
			bm->Unlock();
		}

		static std::vector<std::vector<cv::Point>> FindContours(ArchImgProc::IBitmapData* bm)
		{
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(mat, contours, CV_RETR_EXTERNAL /*CV_RETR_CCOMP*/, CV_CHAIN_APPROX_SIMPLE/*CV_CHAIN_APPROX_TC89_KCOS*/);
			bm->Unlock();
			std::vector<std::vector<cv::Point>> contoursPolyDp;
			contoursPolyDp.resize(contours.size());
			for (int i = 0; i < contours.size(); ++i)
			{
				cv::approxPolyDP(contours[i], contoursPolyDp[i], 3, true);
			}

			//return contours;
			return contoursPolyDp;
		}

		static std::vector<std::vector<cv::Point>> FindContoursOnlyExtremeOuterContours(IBitmapData* bm)
		{
			//return FindContours(bm);
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(mat, contours, CV_RETR_LIST  /*CV_RETR_CCOMP*/, CV_CHAIN_APPROX_SIMPLE/*CV_CHAIN_APPROX_TC89_KCOS*/);
			bm->Unlock();
			std::vector<std::vector<cv::Point>> contoursPolyDp;
			contoursPolyDp.resize(contours.size());
			for (int i = 0; i < contours.size(); ++i)
			{
				cv::approxPolyDP(contours[i], contoursPolyDp[i], 3, true);
			}

			//return contours;
			return contoursPolyDp;
		}

		static std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>> FindContoursHierarchy(IBitmapData* bm)
		{
			assert(bm->GetPixelType() == ArchImgProc::PixelType::Gray8);
			auto bmLck = bm->Lock();
			cv::Mat mat{ (int)bm->GetHeight(), (int)bm->GetWidth(),CV_8UC1 ,(void*)bmLck.ptrDataRoi, (size_t)bmLck.pitch };
			std::vector<std::vector<cv::Point>> contours;
			std::vector<cv::Vec4i> hierarchy;
			cv::findContours(mat, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			bm->Unlock();
			std::vector<std::vector<cv::Point>> contoursPolyDp;
			contoursPolyDp.resize(contours.size());
			for (int i = 0; i < contours.size(); ++i)
			{
				cv::approxPolyDP(contours[i], contoursPolyDp[i], 3, true);
			}

			//return contours;
			return std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>>{ contoursPolyDp, hierarchy };
		}

		static EllipseUtils::EllipseParametersF FitEllipse(const std::vector<cv::Point>& contour, IntRect* aabb = nullptr)
		{
			cv::RotatedRect rr = cv::fitEllipse(contour);
			auto ep = EllipseUtils::EllipseParametersF{ rr.center.x,rr.center.y,rr.size.width / 2,rr.size.height / 2,float(CV_PI*rr.angle / 180) };
			if (aabb != nullptr)
			{
				cv::Rect boundingBox = rr.boundingRect();
				aabb->x = boundingBox.x;
				aabb->y = boundingBox.y;
				aabb->w = boundingBox.width;
				aabb->h = boundingBox.height;
			}

			return ep;
		}

		/// <summary>	Calculates the circularity. </summary>
		/// <remarks>	
		/// The area of a circle is: A(r) = 4 * Pi * r^2. The perimeter of a circle is: P(r) = 2 * Pi * r.
		/// The area as a function of the perimeter is: A(P) = P^2 / (4 * Pi).
		/// So, we take the area of the contour and the perimeter. The we calculate the ratio of A / A(P) which
		/// gives us: A / A(P) = (4 * Pi * A) / P^2.
		/// For a perfect circle, this would be exactly 1. The more it deviates from 1, the more the contour is
		/// different from a circle.
		/// </remarks>
		/// <param name="contour">	The contour. </param>
		/// <param name="area">   	The area of the contour. If this parameter is negative, the area is calculated.</param>
		/// <returns>	The calculated circularity. </returns>
		static float CalcCircularity(const std::vector<cv::Point> contour, float area)
		{
			if (area < 0)
			{
				area = cv::contourArea(contour);
			}

			float perimeter = cv::arcLength(contour, true);

			float circularity = 4 * 3.1415926535897932384626433832795f * area / (perimeter*perimeter);
			return circularity;
		}

		static int FindCircleCandidate(const std::vector<std::vector<cv::Point>>& contours)
		{
			struct AreaAndIndex
			{
				int index;
				float area;
			};

			std::vector<AreaAndIndex> areas;
			areas.reserve(contours.size());
			for (int i = 0; i < contours.size(); ++i)
			{
				float area = cv::contourArea(contours.at(i));
				areas.push_back(AreaAndIndex{ i,area });
			}

			// TODO: we can take into account some more things... like
			//   - center (of the other rings)
			//   - position in the center is more likely than at the corners
			//   - the circularity should be around the same as for the other rings
			//   - ???

			std::sort(areas.begin(), areas.end(), [](const AreaAndIndex& a, const AreaAndIndex& b)->int {return a.area > b.area; });

			for (int i = 0; i < areas.size(); ++i)
			{
				const AreaAndIndex& ai = areas.at(i);
				// now check if it is (more or less...) a circular contour
				float circularity = CalcCircularity(contours[ai.index], ai.area);
				if (abs(circularity - 1) < 0.15f)
				{
					return ai.index;
				}
			}

			return -1;
		}

		static std::vector<int> Filter(const std::vector<std::vector<cv::Point>>& contours,
			std::function<bool(const std::vector<cv::Point>, float area, float circularity)> predicate)
		{
			std::vector<int> vec;
			int index = 0;
			for (const auto it : contours)
			{
				const std::vector<cv::Point>& p = it;
				float area = cv::contourArea(p);
				float circularity = CalcCircularity(p, area);
				bool b = predicate(it, area, circularity);
				if (b == true)
				{
					vec.push_back(index);
				}

				++index;
			}

			return vec;
		}

		static void CalcAreaAndCircularity(const std::vector<cv::Point>& cont, float* area, float* circularity)
		{
			float a = cv::contourArea(cont);
			if (circularity != nullptr)
			{
				*circularity = CalcCircularity(cont, a);
			}

			if (area != nullptr) { *area = a; }
		}

		static void EnumTopLevelContours(const std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>>& contourNhierarchy,
			std::function<bool(int)> functor)
		{
			const auto& hierarchy = std::get<1>(contourNhierarchy);
			int indx = 0;
			for (const auto& it : hierarchy)
			{
				// 0 -> index of next contour (of same hierarchy level)
				// 1 -> index of previous contour (of same hierarchy level)
				// 2 -> first child contour
				// 3 -> parent contour

				// top level contours are those where the parent-contour-index is -1
				if (it[3] < 0)
				{
					bool b = functor(indx);
					if (b == false)
						break;
				}

				++indx;
			}
		}

		static std::tuple<int, int> FindCircleCandidates(const std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>>& contourNhierarchy)
		{
			int outer = FindCircleCandidate(std::get<0>(contourNhierarchy));
			if (outer < 0)
			{
				return std::tuple<int, int>{-1, -1};
			}

			// check if we have an inner contour
			const auto hier = std::get<1>(contourNhierarchy).at(outer);
			int inner = hier[2];


			//int inner = std::get<1>(contourNhierarchy).at(outer)[2];
			/*int inner = -1;
			// now search the hierarchy for this contour
			const vector<cv::Vec4i>& h = std::get<1>(contourNhierarchy);
			for (int i = 0; i < h.size(); ++i)
			{
			if (h.at(i)[0] == outer)
			{
			inner = h.at(i)[2];
			break;
			}
			}*/

			return std::tuple<int, int>{outer, inner};
		}
	};
}