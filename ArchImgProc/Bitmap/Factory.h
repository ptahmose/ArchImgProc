#pragma once

namespace ArchImgProc
{
	std::shared_ptr<IBitmapData2> CreateBitmapStd(PixelType pixeltype, std::uint32_t width, std::uint32_t height, std::uint32_t pitch = 0, std::uint32_t extraRows = 0, std::uint32_t extraColumns = 0)
	{
		return CBitmapData<CHeapAllocator>::Create(pixeltype, width, height, pitch, extraRows, extraColumns);
	}
}