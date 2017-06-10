#pragma once

#include <memory>
#include <cstdint>

namespace ArchImgProc
{
	enum class PixelType
	{
		BGR24,
		BGRFloat32,
		GrayFloat32,
		GrayDouble64,
		ComplexFloat32,
		ComplexDouble64,
		Gray16,
		Gray8
	};

	struct BitmapLockInfo
	{
		void*				ptrData;
		void*				ptrDataRoi;
		std::uint32_t		pitch;
		std::uint64_t		size;
	};

	class IBitmapData
	{

	public:
		virtual PixelType			GetPixelType() const = 0;
		virtual std::uint32_t		GetWidth() const = 0;
		virtual std::uint32_t		GetHeight() const = 0;

		virtual BitmapLockInfo	Lock() = 0;
		virtual void Unlock() = 0;
	};

	class ITinyDataStore
	{
	public:
		enum class AddValueRetCode
		{
			Success_Added,
			Success_Updated,
			Failure_CapacityExceeded,
			Failure_ItemsLengthTooLarge
		};

		virtual bool	GetValue(uint16_t key, void* ptrData, uint8_t sizeData, uint8_t* ptrSizeRequired) const = 0;
		virtual AddValueRetCode AddOrSetValue(uint16_t key, void* ptrData, uint8_t sizeData) = 0;

		bool GetValueInt32(uint16_t key, int& value) { return this->GetValue(key, &value, sizeof(int32_t), nullptr); }
		bool GetValueUInt8(uint16_t key, std::uint8_t& value) { return this->GetValue(key, &value, sizeof(std::uint8_t), nullptr); }
		bool GetValueUInt16(uint16_t key, std::uint16_t& value) { return this->GetValue(key, &value, sizeof(std::uint16_t), nullptr); }
		AddValueRetCode AddOrSetValueUint8(uint16_t key, uint8_t value) { return this->AddOrSetValue(key, &value, sizeof(value)); }
		AddValueRetCode AddOrSetValueUint16(uint16_t key, uint16_t value) { return this->AddOrSetValue(key, &value, sizeof(value)); }
	};

	class IRWLock
	{
	public:
		virtual void lock_write() = 0;
		virtual void lock_read() = 0;
		virtual void unlock_write() = 0;
		virtual void unlock_read() = 0;
	};

	class IBitmapData2 : public IRWLock, public ITinyDataStore, public IBitmapData
	{
	};

	/*std::shared_ptr<IBitmapData2> CreateBitmapStd(PixelType pixeltype, std::uint32_t width, std::uint32_t height, std::uint32_t pitch = 0, std::uint32_t extraRows = 0, std::uint32_t extraColumns = 0)
	{
		return CBitmapData<CHeapAllocator>::Create(pixeltype, width, height, pitch, extraRows, extraColumns);
	}*/

	/*std::shared_ptr<IBitmapData> LoadBitmapFromFile(const wchar_t* szwFilename) { return CLoadBitmap::LoadBitmapFromFile(szwFilename); }*/

	//void SaveBitmap(std::shared_ptr<IBitmapData> bitmap, const wchar_t* szwFilename);

	//void SaveAsFloat32(std::shared_ptr<IBitmapData> bitmap, const wchar_t* szwFilename);

	//void SaveAsCsv(std::shared_ptr<IBitmapData> bitmap, const wchar_t* szwFilename);
}

#include "Bitmap.h"
#include "allocator.h"

#include "Factory.h"
#include "BitmapLoader.h"
#include "BitmapSave.h"
#include "ColorVisionTransform.h"

namespace ArchImgProc
{
	inline std::shared_ptr<IBitmapData> LoadBitmapFromFile(const wchar_t* szwFilename) { return CLoadBitmap::LoadBitmapFromFile(szwFilename); }
	inline void SaveBitmapToFileAsPng(std::shared_ptr<IBitmapData> bitmap, const wchar_t* szwFilename) { return CSaveBitmap::SaveBitmapAsPng(bitmap, szwFilename); }
}
