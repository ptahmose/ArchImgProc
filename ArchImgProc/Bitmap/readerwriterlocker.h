#pragma once

#include <Windows.h>

namespace ArchImgProc
{
	class CReaderWriterLockerSRW
	{
	private:
		SRWLOCK lock;
	public:
		CReaderWriterLockerSRW()
		{
			InitializeSRWLock(&this->lock);
		}

		void lock_write() { AcquireSRWLockExclusive(&this->lock); }
		void lock_read() { AcquireSRWLockShared(&this->lock); }
		void unlock_write() { ReleaseSRWLockExclusive(&this->lock); }
		void unlock_read() { ReleaseSRWLockShared(&this->lock); }
	};
}