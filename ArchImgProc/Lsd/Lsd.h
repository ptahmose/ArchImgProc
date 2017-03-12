#pragma once

#include "../Bitmap/IBitmap.h"

namespace ArchImgProc
{
	template <typename tFloat>
	struct LineSegment
	{
		tFloat x1, y1;
		tFloat x2, y2;
		tFloat width;
		tFloat angle_precision;
		tFloat nfa;

		tFloat LengthSquared() const { return (this->x1 - this->x2)*(this->x1 - this->x2) + (this->y1 - this->y2)*(this->y1 - this->y2); }
		tFloat Length() const { return sqrt(this->LengthSquared()); }
	};
}

#include "LsdImp.h"

namespace ArchImgProc
{
	typedef LineSegment<float> LSDLineSegment;

	class LineSegmentDetection
	{
	public:
		static std::vector<LSDLineSegment> DoLSD(ArchImgProc::IBitmapData* bm, float scaleX = 1, float scaleY = 1)
		{
			float scales[2] = { scaleX,scaleY };
			const float* ptrScales = (scaleX != 1 || scaleY != 1) ? scales : nullptr;
			PixelType pt = bm->GetPixelType();
			switch (pt)
			{
			case ArchImgProc::PixelType::BGR24:
			{
				BitmapLockInfo bmLockInfo = bm->Lock();
				Internal::LSDNew<float, float> lsd;
				auto r = lsd.LSD_BGR((const unsigned char*)bmLockInfo.ptrDataRoi, bm->GetWidth(), bm->GetHeight(), bmLockInfo.pitch, ptrScales);
				bm->Unlock();
				return r;
			}
			case ArchImgProc::PixelType::Gray8:
			{
				BitmapLockInfo bmLockInfo = bm->Lock();
				Internal::LSDNew<float, float> lsd;
				auto r = lsd.LSD_Gray8((const unsigned char*)bmLockInfo.ptrDataRoi, bm->GetWidth(), bm->GetHeight(), bmLockInfo.pitch, ptrScales);
				bm->Unlock();
				return r;
			}
			}

			throw std::runtime_error("not implemented");
		}

		static std::vector<LSDLineSegment> DoLSD(ArchImgProc::PixelType pixeltype, const void* ptr, int stride, int width, int height, float scaleX = 1, float scaleY = 1)
		{
			float scales[2] = { scaleX,scaleY };
			const float* ptrScales = (scaleX != 1 || scaleY != 1) ? scales : nullptr;
			switch (pixeltype)
			{
			case ArchImgProc::PixelType::BGR24:
			{
				Internal::LSDNew<float, float> lsd;
				auto r = lsd.LSD_BGR((const unsigned char*)ptr, width, height, stride, ptrScales);
				return r;
			}
			case ArchImgProc::PixelType::Gray8:
			{
				Internal::LSDNew<float, float> lsd;
				auto r = lsd.LSD_Gray8((const unsigned char*)ptr, width, height, stride, ptrScales);
				return r;
			}
			}

			throw std::runtime_error("not implemented");
		}
	};


}