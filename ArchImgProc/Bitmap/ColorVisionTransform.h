#pragma once

#include <cstdint>

namespace ArchImgProc
{
	class ColorVisionTransform
	{
	private:
		static bool IsBlackHsv(std::uint8_t h, std::uint8_t s, std::uint8_t v)
		{
			std::uint8_t minH, maxH;
			minH = (std::uint8_t)((180. / 360.) * 255);
			maxH = (std::uint8_t)((265. / 360.) * 255);

			const std::uint8_t minS = (std::uint8_t)((60. / 100.) * 255);
			const std::uint8_t minV = (std::uint8_t)((40. / 100.) * 255);

			if (minH <= h&&maxH >= h)
			{
				if (s < minS)
				{
					if (v < minV)
					{
						return true;
					}
				}
			}

			return false;
		}

		static bool isWhite(std::uint8_t r, std::uint8_t g, std::uint8_t b)
		{
			const std::uint8_t LowerLimit = 200;
			if (r > LowerLimit && g > LowerLimit&& b > LowerLimit)
			{
				if (abs(r - g) < 10 && abs(r - b) < 10 && abs(g - b) < 10)
				{
					return true;
				}
			}

			return false;
		}

		static bool isBlack(std::uint8_t r, std::uint8_t g, std::uint8_t b)
		{
			const std::uint8_t UpperLimit = 90;
			const int MaxDiff = 20;
			if (r < UpperLimit && g < UpperLimit && b < UpperLimit)
			{
				if (abs(r - g) < MaxDiff && abs(r - b) < MaxDiff && abs(g - b) < MaxDiff)
				{
					return true;
				}
			}

			return false;
		}

		static std::uint8_t ToByte(float f)
		{
			int i = (int)f;
			if (i > 255)
			{
				i = 255;
			}
			else if (i < 0)
			{
				i = 0;
			}

			return (std::uint8_t)i;
		}
	public:
		static std::shared_ptr<ArchImgProc::IBitmapData> Transform(std::shared_ptr<ArchImgProc::IBitmapData> src)
		{
			auto bmFltDst = CreateBitmapStd(PixelType::BGRFloat32, src->GetWidth(), src->GetHeight());
			auto bmDst = CreateBitmapStd(PixelType::BGR24, src->GetWidth(), src->GetHeight());
			auto srcLockInfo = src->Lock();
			auto dstLockInfo = bmDst->Lock();
			auto tmpLockInfo = bmFltDst->Lock();
			ColorVisionTransform::Transform(
				(const std::uint8_t*)srcLockInfo.ptrDataRoi,
				src->GetWidth(),
				src->GetHeight(),
				srcLockInfo.pitch,
				(std::uint8_t*)dstLockInfo.ptrDataRoi,
				dstLockInfo.pitch,
				(float*)tmpLockInfo.ptrDataRoi,
				tmpLockInfo.pitch);
			src->Unlock();
			bmDst->Unlock();
			bmFltDst->Unlock();
			return bmDst;
		}

		static void Transform(const std::uint8_t* pSrc, std::uint32_t width, std::uint32_t height, int strideSrc,
			std::uint8_t* pDst, int strideDst,
			float* pTemp, int strideTemp)
		{
			float minR, maxR, minG, maxG, minB, maxB;
			minR = minG = minB = (std::numeric_limits<float>::max)();
			maxR = maxG = maxB = (std::numeric_limits<float>::min)();
			for (int y = 0; y < height; ++y)
			{
				const std::uint8_t* ptrSrc = pSrc + y*strideSrc;
				float* ptrDst = (float*)(((char*)(pTemp)) + y*strideTemp);
				for (int x = 0; x < width; ++x)
				{
					std::uint8_t r = ptrSrc[2];
					std::uint8_t g = ptrSrc[1];
					std::uint8_t b = ptrSrc[0];

					std::uint8_t h, s, v;
					RgbToHsv(r, g, b, h, s, v);

					//if (x == 1237 && y == 314 /*|| r==39&&g==47&&b==58*/)
					//{
					//	//DebugBreak();
					//}
					if (IsBlackHsv(h, s, v))
					{
						ptrDst[2] =
							ptrDst[1] =
							ptrDst[0] = 0;
						minR =
							minG =
							minB = 0;
					}
					else
					{
						float rF;
						float gF;
						float bF;

						if (isWhite(r, g, b))
						{
							rF = gF = bF = 1;
						}
						else if (isBlack(r, g, b))
						{
							rF = gF = bF = 0;
						}
						else
						{
							float i = r + g + b;

							rF = r / i;
							gF = g / i;
							bF = b / i;
						}

						minR = min(minR, rF); maxR = max(maxR, rF);
						minG = min(minG, gF); maxG = max(maxG, gF);
						minB = min(minB, bF); maxB = max(maxB, bF);

						ptrDst[2] = rF;
						ptrDst[1] = gF;
						ptrDst[0] = bF;
					}

					ptrSrc += 3;
					ptrDst += 3;
				}
			}

			float factorR = 255 / (maxR - minR);
			float factorG = 255 / (maxG - minG);
			float factorB = 255 / (maxB - minB);

			for (int y = 0; y < height; ++y)
			{
				float* ptrDst = (float*)(((char*)(pTemp)) + y*strideTemp);
				for (int x = 0; x < width; ++x)
				{
					ptrDst[2] = (ptrDst[2] - minR)*factorR;
					ptrDst[1] = (ptrDst[1] - minG)*factorG;
					ptrDst[0] = (ptrDst[0] - minB)*factorB;

					ptrDst += 3;
				}
			}

			for (int y = 0; y < height; ++y)
			{
				const float* ptrsrc = (const float*)(((char*)(pTemp)) + y*strideTemp);
				std::uint8_t* ptrBgrSrc = (std::uint8_t*)(((const char*)(pDst)) + y*strideDst);
				for (int x = 0; x < width; ++x)
				{
					ptrBgrSrc[2] = ToByte(ptrsrc[2]);
					ptrBgrSrc[1] = ToByte(ptrsrc[1]);
					ptrBgrSrc[0] = ToByte(ptrsrc[0]);

					ptrsrc += 3;
					ptrBgrSrc += 3;
				}
			}
		}

		static void RgbToHsv(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t& h, std::uint8_t& s, std::uint8_t& v)
		{
			std::uint8_t rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
			std::uint8_t rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
			v = rgbMax;
			if (v == 0)
			{
				h = 0;
				s = 0;
			}
			else
			{
				s = 255 * ((long)(rgbMax - rgbMin)) / v;
				if (s == 0)
				{
					h = 0;
				}
				else
				{
					if (rgbMax == r)
						h = 0 + 43 * (g - b) / (rgbMax - rgbMin);
					else if (rgbMax == g)
						h = 85 + 43 * (b - r) / (rgbMax - rgbMin);
					else
						h = 171 + 43 * (r - g) / (rgbMax - rgbMin);
				}
			}
		}
	};
}