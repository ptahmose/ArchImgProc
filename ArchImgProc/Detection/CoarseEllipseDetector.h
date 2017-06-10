#pragma once

#include <ellipseUtils.h>
#include "../Bitmap/ColorVisionTransform.h"
#include "morphologyOpenCV.h"

namespace ArchImgProc
{
	struct CoarseResult
	{
		EllipseUtils::EllipseParametersF ellipse;
		
		int radius;

		/// <summary>	
		/// If postive then the "real" ellipse is probably smaller than the ellipse given here,
		/// if negative then the "real" ellise is probably bigger , and if 0 then 
		/// we have no guidance. </summary>
		//int IsLikelyTooLarge;
	};

	class CCoarseEllipseDetector2
	{
	public:
		class ProcessOptions
		{
			
		};
	private:
		static bool IsGold(std::uint8_t h, std::uint8_t s, std::uint8_t v)
		{
			const std::uint8_t range_1 = (std::uint8_t)(/*43*/38 / 360. * 255.);
			const std::uint8_t range_2 = (std::uint8_t)(/*55*/92 / 360. * 255);
			if (h >= range_1 && h <= range_2)
			{
				if (s > 200 && v > 80)
					return true;
			}

			return false;
		}

		static bool IsRed(std::uint8_t h, std::uint8_t s, std::uint8_t v)
		{
			const std::uint8_t range_1 = (std::uint8_t)(10 / 360. * 255.);
			const std::uint8_t range_2 = (std::uint8_t)(320 / 360. * 255);
			if (h <= range_1 || h >= range_2)
			{
				if (s > 128 && v > 128)
					return true;
			}

			return false;
		}

		static bool IsBlue(std::uint8_t h, std::uint8_t s, std::uint8_t v)
		{
			const std::uint8_t range_1 = (std::uint8_t)(/*200*/180 / 360. * 255.);
			const std::uint8_t range_2 = (std::uint8_t)(235 / 360. * 255);
			if (h >= range_1 && h <= range_2)
			{
				if (s > 128 && v > 80)
					return true;
			}

			return false;
		}

		static void ExtractGoldRedBlue(
			std::shared_ptr<ArchImgProc::IBitmapData> bm,
			std::shared_ptr<ArchImgProc::IBitmapData>& outGold,
			std::shared_ptr<ArchImgProc::IBitmapData>& outRed,
			std::shared_ptr<ArchImgProc::IBitmapData>& outBlue)
		{
			//assert(bm->GetPixelType() == NSBitmapLib::PixelType::BGR24);

			auto bmGold = CreateBitmapStd(ArchImgProc::PixelType::Gray8, bm->GetWidth(), bm->GetHeight());
			auto bmRed = CreateBitmapStd(ArchImgProc::PixelType::Gray8, bm->GetWidth(), bm->GetHeight());
			auto bmBlue = CreateBitmapStd(ArchImgProc::PixelType::Gray8, bm->GetWidth(), bm->GetHeight());

			auto lckSrc = bm->Lock();
			auto lckGoldDst = bmGold->Lock();
			auto lckRedDst = bmRed->Lock();
			auto lckBlueDst = bmBlue->Lock();

			for (std::uint32_t y = 0; y < bm->GetHeight(); ++y)
			{
				const std::uint8_t* ptrSrc = (const std::uint8_t*)(((const char*)(lckSrc.ptrDataRoi)) + y*lckSrc.pitch);
				std::uint8_t* ptrGoldDst = (std::uint8_t*)(((char*)(lckGoldDst.ptrDataRoi)) + y*lckGoldDst.pitch);
				std::uint8_t* ptrRedDst = (std::uint8_t*)(((char*)(lckRedDst.ptrDataRoi)) + y*lckRedDst.pitch);
				std::uint8_t* ptrBlueDst = (std::uint8_t*)(((char*)(lckBlueDst.ptrDataRoi)) + y*lckBlueDst.pitch);

				for (std::uint32_t x = 0; x < bm->GetWidth(); ++x)
				{
					std::uint8_t b = *ptrSrc;
					std::uint8_t g = *(ptrSrc + 1);
					std::uint8_t r = *(ptrSrc + 2);

					std::uint8_t h, s, v;
					ColorVisionTransform::RgbToHsv(r, g, b, h, s, v);

					bool isGold = false, isRed = false, isBlue = false;

					if (IsGold(h, s, v))
					{
						isGold = true;
					}
					else if (IsRed(h, s, v))
					{
						isRed = true;
					}
					else if (IsBlue(h, s, v))
					{
						isBlue = true;
					}

					*ptrGoldDst++ = isGold ? 255 : 0;
					*ptrRedDst++ = isRed ? 255 : 0;
					*ptrBlueDst++ = isBlue ? 255 : 0;
					ptrSrc += 3;
				}
			}

			bm->Unlock();
			bmGold->Unlock();
			bmRed->Unlock();
			bmBlue->Unlock();

			outGold = bmGold;
			outRed = bmRed;
			outBlue = bmBlue;
		}
	private:
		std::shared_ptr<ArchImgProc::IBitmapData> bm;
		ProcessOptions options;
		//std::shared_ptr<ILog> log;

		//std::wstring backgroundImageFilename;

		EllipseUtils::EllipseParametersF outerGold;
		EllipseUtils::EllipseParametersF innerRed, outerRed;
		EllipseUtils::EllipseParametersF innerBlue, outerBlue;
	public:
		CCoarseEllipseDetector2(std::shared_ptr<ArchImgProc::IBitmapData> bm, const ProcessOptions& options/*, std::shared_ptr<ILog> log*/)
			: bm(bm), options(options)//, log(log)
		{}

		void Calculate();
		int GetNumberOfResults() const
		{
			return 3;
		}
		CoarseResult GetResult(int index) const
		{
			CoarseResult cr;
			switch (index)
			{
			case 0:
				cr.ellipse = this->outerGold;
				cr.radius = 2;
				//	cr.IsLikelyTooLarge = 0;
				break;
			case 1:
				cr.ellipse = this->outerRed;
				cr.radius = 4;
				//		cr.IsLikelyTooLarge = 0;
				break;
			case 2:
				cr.ellipse = this->outerBlue;
				cr.radius = 6;
				//cr.IsLikelyTooLarge = 0;
				break;
			default:
				throw std::exception("illegal index");
			}

			return cr;
		}
	private:
		void SaveColorFiltered(std::shared_ptr<ArchImgProc::IBitmapData> bmGold, std::shared_ptr<ArchImgProc::IBitmapData> bmRed, std::shared_ptr<ArchImgProc::IBitmapData> bmBlue);
		void DealWithGold(std::shared_ptr<ArchImgProc::IBitmapData> bmGold);
		void DealWithRed(std::shared_ptr<ArchImgProc::IBitmapData> bmRed);
		void DealWithBlue(std::shared_ptr<ArchImgProc::IBitmapData> bmBlue);
		void DealWithRedOrBlue(std::shared_ptr<ArchImgProc::IBitmapData> bm, EllipseUtils::EllipseParametersF& outer, int minArea, int maxArea/*, std::wstring(CCmdLineOptions::*pfnGetFilename)()const, const wchar_t* szExplanationText*/);

		void SaveImage(std::shared_ptr<ArchImgProc::IBitmapData> bm, std::function<std::wstring()> getFilename, std::function<std::wstring()> getText);
		void SaveImage(std::function<std::shared_ptr<ArchImgProc::IBitmapData>()> getImg, std::function<std::wstring()> getFilename, std::function<std::wstring()> getText);

		static int TryToFindContourCandidateRedOrBlue(const EllipseUtils::EllipseParametersF& outerGold, const std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>>& contoursAndHierarchy, const std::vector<std::tuple<int, float>>& candidatesIndexAndArea);
	private:
		static const wchar_t* TextColorVisionResult;
		static const wchar_t* TextGoldOpened;
		static const wchar_t* TextRedOpened;
		static const wchar_t* TextBlueOpened;

		static const wchar_t* TextGoldColorFilterd;
		static const wchar_t* TextRedColorFilterd;
		static const wchar_t* TextBlueColorFilterd;
	};

	inline void CCoarseEllipseDetector2::Calculate()
	{
		std::shared_ptr<ArchImgProc::IBitmapData> cvNormalized = ColorVisionTransform::Transform(bm);

		std::shared_ptr<ArchImgProc::IBitmapData> bmGold, bmRed, bmBlue;
		ExtractGoldRedBlue(cvNormalized, bmGold, bmRed, bmBlue);

		this->DealWithGold(bmGold);
		this->DealWithRed(bmRed);
		this->DealWithBlue(bmBlue);
	}

	inline void CCoarseEllipseDetector2::DealWithGold(std::shared_ptr<ArchImgProc::IBitmapData> bmGold)
	{
		for (int timesOpened = 0; timesOpened < 5; ++timesOpened)
		{
			COpenCVUtils::Morphology_Closing2(bmGold.get(), 10 * (timesOpened + 1));

			auto regions = COpenCVUtils::FindContoursOnlyExtremeOuterContours(bmGold.get());


			int minArea = 4 * pow((60. / 2592.)*this->bm->GetWidth(), 2);
			int maxArea = 4 * pow((550. / 2592.)*this->bm->GetWidth(), 2);

			// we have a rough idea of the size and the circularity... so let's check whether we can find the Gold... 
			std::vector<int> vec = COpenCVUtils::Filter(regions, [&](const std::vector<cv::Point> region, float area, float circularity)->bool
			{
				if (area < minArea || area >= maxArea)
				{
					return false;
				}

				if (abs(circularity - 1) < 0.15f)
				{
					return true;
				}

				return false;
			});

			// in the best case we have only once candidate... otherwise things get difficult...
			if (vec.size() != 1)
			{
				continue;
			}

			this->outerGold = COpenCVUtils::FitEllipse(regions[vec[0]]);
			return;
		}

		throw std::exception("no unique region found");
	}

	inline void CCoarseEllipseDetector2::DealWithRed(std::shared_ptr<ArchImgProc::IBitmapData> bmRed)
	{
		int minArea = 4 * pow((100. / 2592.)*this->bm->GetWidth(), 2);
		int maxArea = 4 * pow((850. / 2592.)*this->bm->GetWidth(), 2);

		this->DealWithRedOrBlue(bmRed, this->outerRed, minArea, maxArea/*, &CCmdLineOptions::TargetScan3_RedColorFilteredOpenedImage, CCoarseEllipseDetector2::TextRedOpened*/);
	}

	inline void CCoarseEllipseDetector2::DealWithBlue(std::shared_ptr<ArchImgProc::IBitmapData> bmBlue)
	{
		int minArea = 4 * pow((130. / 2592.)*this->bm->GetWidth(), 2);
		int maxArea = 4 * pow((1550. / 2592.)*this->bm->GetWidth(), 2);

		this->DealWithRedOrBlue(bmBlue, this->outerBlue, minArea, maxArea/*, &CCmdLineOptions::TargetScan3_BlueColorFilteredOpenedImage, CCoarseEllipseDetector2::TextBlueOpened*/);
	}

	inline void CCoarseEllipseDetector2::DealWithRedOrBlue(std::shared_ptr<ArchImgProc::IBitmapData> bm, EllipseUtils::EllipseParametersF& outer, int minArea, int maxArea/*, std::wstring(CCmdLineOptions::*pfnGetFilename)()const, const wchar_t* szExplanationText*/)
	{
		for (int timesOpened = 0; timesOpened < 3 + 2; ++timesOpened)
		{
			if (timesOpened < 3)
			{
				COpenCVUtils::Morphology_Closing2(bm.get(), 10 * (timesOpened + 1));
			}
			else
			{
				if (timesOpened == 3)
					COpenCVUtils::Morphology_Opening2(bm.get(), 10 /* * (timesOpened + 1)*/);
				else
					COpenCVUtils::Morphology_Opening2(bm.get(), 20 /* * (timesOpened + 1)*/);
			}

			/*this->SaveImage(
				[&bm]() {CImgStuff::BinarizeToBlackWhite(bm.get()); return bm; },
				[this, timesOpened, pfnGetFilename]()
			{
				auto fname = (this->options.*pfnGetFilename)();
				std::wstringstream ss;
				ss << L"#" << timesOpened + 1;
				AppendBeforeLastDot(fname, ss.str());
				return fname;
			},
				[timesOpened, szExplanationText]() {
				std::wstringstream ss;
				ss << szExplanationText;
				ss << GetStringForNTimes(timesOpened + 1);
				return ss.str();
			}
			);*/

			auto regions = COpenCVUtils::FindContoursHierarchy(bm.get());

			std::vector<std::tuple<int, float>> candidates;

			// we are now interested in a contour which is roughly circular, and has reasonable size and 
			COpenCVUtils::EnumTopLevelContours(regions,
				[&](int idx)->bool
			{
				const auto& contour = std::get<0>(regions).at(idx);
				const auto& hierarchy = std::get<1>(regions).at(idx);
				// check if it has a child contour
				if (hierarchy[2] < 0) { return true; /* continue enumeration */ }
				float area, circularity;
				COpenCVUtils::CalcAreaAndCircularity(contour, &area, &circularity);
				if (area<minArea || area>maxArea) { return true; /* continue enumeration */ }

				// check if the red is reasonable close to the gold

				candidates.push_back(std::tuple<int, float>(idx, area));
				return true;
			});

			int candidateIndex = -1;
			// in the best case we have only once candidate... otherwise things get difficult...
			if (candidates.size() > 1)
			{
				candidateIndex = CCoarseEllipseDetector2::TryToFindContourCandidateRedOrBlue(this->outerGold, regions, candidates);
				// TODO: at least as last resort, take the one for which the center is closest to the center to the gold
			}
			else if (candidates.size() == 1)
			{
				candidateIndex = 0;
			}

			if (candidateIndex < 0)
			{
				continue;
			}

			outer = COpenCVUtils::FitEllipse(std::get<0>(regions)[std::get<0>(candidates[candidateIndex])]);

			// TODO: is there a better and/or faster quality measure? Maybe moments?
			// calc mean distance from points on the contour to the ellipse
			float meanDistance, maxDistance;
			const std::vector<cv::Point>& pts = std::get<0>(regions)[std::get<0>(candidates[candidateIndex])];
			EllipseUtils::CEllipseUtilities::EstimateErrorOfFit<float>(
				outer,
				[&](size_t index, float* x, float* y)->bool
			{
				if (index < pts.size())
				{
					if (x != nullptr) { *x = pts.at(index).x; }
					if (y != nullptr) { *y = pts.at(index).y; }
					return true;
				}

				return false;
			},
				&meanDistance, nullptr, &maxDistance);

			if (meanDistance / ((std::min)(outer.a, outer.b)) > 0.07)
			{
				continue;
				//outer.Clear();
			}

			return;
		}

		// TODO: we should get more fancy here
		//   - detect that nothing is there (case 21) -> then maybe fiddle with the colors (derive it from the other colors that were detected?)
		outer.Clear();
		//throw std::exception("no unique region found");
	}

	/*static*/inline int CCoarseEllipseDetector2::TryToFindContourCandidateRedOrBlue(const EllipseUtils::EllipseParametersF& outerGold, const std::tuple<std::vector<std::vector<cv::Point>>, std::vector<cv::Vec4i>>& contoursAndHierarchy, const std::vector<std::tuple<int, float>>& candidatesIndexAndArea)
	{
		if (outerGold.IsValid() == false)
		{
			return -1;
		}

		int numberOfCandidatesStillRemaining = candidatesIndexAndArea.size();
		std::vector<bool> candidatesStillValid(candidatesIndexAndArea.size(), true);

		// the area should be greater than the gold-size
		float areaGold = CV_PI * outerGold.a * outerGold.b;
		for (int i = 0; i < candidatesIndexAndArea.size(); ++i)
		{
			if (std::get<1>(candidatesIndexAndArea.at(i)) < areaGold)
			{
				candidatesStillValid[i] = false;
				--numberOfCandidatesStillRemaining;
			}
		}

		if (numberOfCandidatesStillRemaining == 0)
		{
			return -1;
		}
		else if (numberOfCandidatesStillRemaining == 1)
		{
			const auto itResult = std::find(candidatesStillValid.cbegin(), candidatesStillValid.cend(), true);
			return std::distance(candidatesStillValid.cbegin(), itResult);
		}

		float maxDistanceSqr = (std::min)(outerGold.a, outerGold.b);
		maxDistanceSqr *= maxDistanceSqr;
		for (int i = 0; i < candidatesIndexAndArea.size(); ++i)
		{
			if (candidatesStillValid.at(i) == true)
			{
				// calculate the center of mass
				cv::Moments moments;
				moments = cv::moments(std::get<0>(contoursAndHierarchy).at(std::get<0>(candidatesIndexAndArea.at(i))));
				auto centerX = moments.m10 / moments.m00;
				auto centerY = moments.m01 / moments.m00;
				float distanceSqr = (outerGold.x0 - centerX)*(outerGold.x0 - centerX) + (outerGold.y0 - centerY)*(outerGold.y0 - centerY);
				if (distanceSqr > maxDistanceSqr)
				{
					candidatesStillValid[i] = false;
					--numberOfCandidatesStillRemaining;
				}
			}
		}

		if (numberOfCandidatesStillRemaining == 0)
		{
			return -1;
		}
		else if (numberOfCandidatesStillRemaining == 1)
		{
			const auto itResult = std::find(candidatesStillValid.cbegin(), candidatesStillValid.cend(), true);
			return std::distance(candidatesStillValid.cbegin(), itResult);
		}

		return -1;
	}

}