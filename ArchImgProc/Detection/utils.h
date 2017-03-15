#pragma once

namespace ArchImgProc
{
	class CUtils
	{
	public:
		template<typename tFlt>
		tFlt CalcDistance(tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			return std::sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
		}

		template<typename tFlt>
		tFlt CalcDistanceSquared(tFlt x1, tFlt y1, tFlt x2, tFlt y2)
		{
			return ((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
		}



	};
}