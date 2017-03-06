#pragma once

#include "readerwriterlocker.h"
#include "datastore.h"
#include "utils.h"
#include <atomic>
#include <stdexcept> 

namespace ArchImgProc
{
	template  < typename tAllocator, typename tRwLocker = CReaderWriterLockerSRW, typename tDataStore = CDataStore<128>>
	class CBitmapData : public ArchImgProc::IBitmapData2
	{
	private:
		tRwLocker	rwLocker;
		tDataStore dataStore;
		tAllocator	allocator;
		ArchImgProc::PixelType	pixelType;
		std::uint32_t		width;
		std::uint32_t		height;
		std::uint32_t		pitch;

		std::uint32_t		extraRows;
		std::uint32_t		extraColumns;

		void*	pData;
		std::uint64_t		dataSize;

		std::atomic<int> lockCnt = ATOMIC_VAR_INIT(0);
	public:
		static std::shared_ptr<ArchImgProc::IBitmapData2> Create(ArchImgProc::PixelType pixeltype, std::uint32_t width, std::uint32_t height, std::uint32_t pitch = 0, std::uint32_t extraRows = 0, std::uint32_t extraColumns = 0)
		{
			if (pitch <= 0)
			{
				pitch = CalcDefaultPitch(pixeltype, width + 2 * extraColumns);
			}

			auto s = std::make_shared<CBitmapData<tAllocator>>(pixeltype, width, height, pitch, extraRows, extraColumns);

			return s;
		}

		virtual ArchImgProc::PixelType			GetPixelType() const { return this->pixelType; }
		virtual std::uint32_t		GetWidth() const { return this->width; }
		virtual std::uint32_t		GetHeight() const { return this->height; }
		virtual ArchImgProc::BitmapLockInfo	Lock()
		{
			std::atomic_fetch_add(&this->lockCnt, 1);
			ArchImgProc::BitmapLockInfo bli;
			bli.ptrData = this->pData;
			bli.ptrDataRoi = ((char*)this->pData) + this->extraRows*this->pitch;
			bli.pitch = this->pitch;
			bli.size = this->dataSize;
			return bli;
		}

		virtual void Unlock()
		{
			int lckCnt = std::atomic_fetch_sub(&this->lockCnt, 1);
			if (lckCnt < 1)
			{
				throw std::logic_error("Lock/Unlock-semantic was violated.");
			}
		}


		virtual void lock_write() { return this->rwLocker.lock_write(); }
		virtual void lock_read() { return  this->rwLocker.lock_read(); }
		virtual void unlock_write() { return this->unlock_write(); }
		virtual void unlock_read() { return this->unlock_read(); }

		virtual bool GetValue(uint16_t key, void* ptrData, uint8_t sizeData, uint8_t* ptrSizeRequired) const
		{
			return this->dataStore.GetValue(key, ptrData, sizeData, ptrSizeRequired);
		}

		virtual ArchImgProc::ITinyDataStore::AddValueRetCode AddOrSetValue(uint16_t key, void* ptrData, uint8_t sizeData)
		{
			return this->dataStore.AddOrSetValue(key, ptrData, sizeData);
		}


		virtual ~CBitmapData()
		{
			this->allocator.Free(this->pData);
		}


		CBitmapData(ArchImgProc::PixelType pixeltype, std::uint32_t width, std::uint32_t height, std::uint32_t pitch, std::uint32_t extraRows, std::uint32_t extraColumns)
		{
			uint64_t size = (height + extraRows * 2ULL) * pitch;
			this->pData = this->allocator.Allocate(size);
			this->dataSize = size;
			this->width = width;
			this->height = height;
			this->pitch = pitch;
			this->extraColumns = extraColumns;
			this->extraRows = extraRows;
			this->pixelType = pixeltype;
		}

	private:
		static std::uint32_t CalcDefaultPitch(ArchImgProc::PixelType pixeltype, int width)
		{
			return Utils::CalcDefaultPitch(pixeltype, width);
		}
	};
}