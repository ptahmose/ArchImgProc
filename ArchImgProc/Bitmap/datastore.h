#pragma once

#include <algorithm>

namespace ArchImgProc
{
	/// <summary>	Return value for CDataStore::AddOrSetValue. </summary>
	enum class DataStoreStatusCode
	{
		/// <summary>	The item was new and it was successfully added. </summary>
		OK_Added,

		/// <summary>	The item was already present, and it was successfully updated. </summary>
		OK_Updated,

		/// <summary>	The item was already present, but the stored length is less than what it was tried to update. </summary>
		LengthMismatch,

		/// <summary>	The item could not be added because it did not fit into the datastore. </summary>
		CapacityExceeded
	};

	/// <summary>	A very simplistic (but fast...) data store. It allows to store a couple of (untyped) items,
	/// 			the total size if limited by the template parameter (max. 255 bytes). The overhead per item is 3 bytes. 
	/// </summary>
	///
	/// <remarks>	This class is only meant to store just a few items. Also note that this class is not thread-safe. </remarks>
	///
	/// <typeparam name="tInitialSize">	The total size of the data store (255 bytes max!). </typeparam>
	template <uint8_t tInitialSize>
	class CDataStore
	{
	private:
		uint8_t builtinStore[tInitialSize];
		uint8_t sizeUsed;
	public:
		CDataStore() :sizeUsed(0)
		{
			memset(this->builtinStore, 0, tInitialSize);
		}

		bool	GetValue(uint16_t key, void* ptrData, uint8_t sizeData, uint8_t* ptrSizeRequired) const
		{
			int offset = this->GetOffsetForKey(key);
			if (offset >= 0)
			{
				// the next byte gives us the size
				uint8_t actualSize = this->builtinStore[offset + 2];
				if (ptrSizeRequired != nullptr)
				{
					*ptrSizeRequired = actualSize;
				}

				if (ptrData != nullptr)
				{
					memcpy(ptrData, this->builtinStore + offset + 3, (std::min)(sizeData, actualSize));
				}

				return true;
			}

			return false;
		}

		ITinyDataStore::AddValueRetCode	AddOrSetValue(uint16_t key, void* ptrData, uint8_t sizeData)
		{
			int offset = this->GetOffsetForKey(key);
			if (offset < 0)
			{
				// key is new!

				// TODO: check if it fits in
				if (sizeof(builtinStore) - this->sizeUsed < ((int)sizeData) + 3)
				{
					return ITinyDataStore::AddValueRetCode::Failure_CapacityExceeded;
				}

				*((uint16_t*)((&this->builtinStore[0]) + this->sizeUsed)) = key;

				this->builtinStore[this->sizeUsed + 2] = sizeData;
				memcpy(&(this->builtinStore[this->sizeUsed + 3]), ptrData, sizeData);
				this->sizeUsed += sizeData + 3;
				return ITinyDataStore::AddValueRetCode::Success_Added;// DataStoreStatusCode::OK_Added;
			}

			uint8_t sizeCurrent = this->builtinStore[offset + 2];
			if (sizeCurrent < sizeCurrent)
			{
				return ITinyDataStore::AddValueRetCode::Failure_ItemsLengthTooLarge;// DataStoreStatusCode::LengthMismatch;
			}

			memcpy(&(this->builtinStore[offset + 3]), ptrData, (std::min)(sizeData, sizeCurrent));
			return ITinyDataStore::AddValueRetCode::Success_Updated;// DataStoreStatusCode::OK_Updated;
		}
	private:
		int GetOffsetForKey(uint16_t key) const
		{
			for (int i = 0; i < this->sizeUsed;)
			{
				if (*((uint16_t*)(this->builtinStore + i)) == key)
				{
					return i;
				}

				// advance to next chunk
				i += 3 + this->builtinStore[i + 2];
			}

			return -1;
		}
	};

}