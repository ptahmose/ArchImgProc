#pragma once

#include <stdexcept>
#include <functional>

namespace ArchImgProc
{
	class Utils
	{
	private:
		static inline std::uint8_t RgbToHSV_V(std::uint8_t r, std::uint8_t g, std::uint8_t b)
		{
			std::uint8_t rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
			std::uint8_t rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);
			std::uint8_t v = rgbMax;
			return v;
		}
	public:
		static void ConvertBgr24ToGray8_HSV(ArchImgProc::PixelType pixeltypeSrc, const void* pSrc, int width, int height, int strideSrc, void* pDst, int strideDst)
		{
			switch (pixeltypeSrc)
			{
			case ArchImgProc::PixelType::BGR24:
				{
					for (std::uint32_t y = 0; y < height; ++y)
					{
						const std::uint8_t* ptrSrc = (const std::uint8_t*)(((const char*)(pSrc)) + y*strideSrc);
						std::uint8_t* ptrDst = (std::uint8_t*)(((char*)(pDst)) + y*strideDst);
						for (std::uint32_t x = 0; x < width; ++x)
						{
							std::uint8_t b = *ptrSrc;
							std::uint8_t g = *(ptrSrc + 1);
							std::uint8_t r = *(ptrSrc + 2);

							*ptrDst = RgbToHSV_V(r, g, b);

							++ptrDst;
							ptrSrc += 3;
						}
					}

					break;
				}
			default:
				throw std::runtime_error("not implemented");
			}
		}

		static std::uint32_t Utils::GetBytesPerPel(ArchImgProc::PixelType pixeltype)
		{
			switch (pixeltype)
			{
			case ArchImgProc::PixelType::BGR24:
				return 3;
			case ArchImgProc::PixelType::BGRFloat32:
				return 3 * 4;
			case ArchImgProc::PixelType::GrayFloat32:
				return 4;
			case ArchImgProc::PixelType::ComplexFloat32:
				return 2 * 4;
			case ArchImgProc::PixelType::Gray16:
				return 2;
			case ArchImgProc::PixelType::Gray8:
				return 1;
			case ArchImgProc::PixelType::ComplexDouble64:
				return 2 * 8;
			case ArchImgProc::PixelType::GrayDouble64:
				return 8;
			default:
				throw std::invalid_argument("Unknown pixeltype.");
			}
		}

		static std::uint32_t CalcDefaultPitch(ArchImgProc::PixelType pixeltype, int width, std::uint8_t multipleOf = 4)
		{
			return ((Utils::GetBytesPerPel(pixeltype)*width + (multipleOf - 1)) / multipleOf) * multipleOf;
		}

		static void ThrowIfFailed(const char* function, HRESULT hr)
		{
			if (FAILED(hr))
			{
				char errorMsg[255];
				_snprintf_s(errorMsg, _TRUNCATE, "COM-ERROR hr=0x%08X (%s)", hr, function);
				throw std::runtime_error(errorMsg);
			}
		}

		static void ThrowIfFailed(const char* function, HRESULT hr, std::function<bool(HRESULT)> checkFunc)
		{
			if (checkFunc(hr) != true)
			{
				char errorMsg[255];
				_snprintf_s(errorMsg, _TRUNCATE, "COM-ERROR hr=0x%08X (%s)", hr, function);
				throw std::runtime_error(errorMsg);
			}
		}
	};


}