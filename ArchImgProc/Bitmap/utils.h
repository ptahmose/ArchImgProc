#pragma once

#include <stdexcept>

namespace ArchImgProc
{
	class Utils
	{
	public:
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
	};


}