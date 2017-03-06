#pragma once

#include <cstdint>
#include <malloc.h>

namespace ArchImgProc
{

	class CHeapAllocator
	{
	public:
		void*	Allocate(std::uint64_t size)
		{
			void* pv = _aligned_malloc(size, 32);
			return pv;
		}

		void	Free(void* ptr)
		{
			_aligned_free(ptr);
		}
	};

}